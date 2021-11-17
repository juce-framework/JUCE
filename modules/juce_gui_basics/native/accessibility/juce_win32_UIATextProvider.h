/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class UIATextProvider  : public UIAProviderBase,
                         public ComBaseClassHelper<ITextProvider2>
{
public:
    explicit UIATextProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle)
    {
    }

    //==============================================================================
    JUCE_COMRESULT QueryInterface (REFIID iid, void** result) override
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

        if (iid == _uuidof (IUnknown) || iid == _uuidof (ITextProvider))
            return castToType<ITextProvider> (result);

        if (iid == _uuidof (ITextProvider2))
            return castToType<ITextProvider2> (result);

        *result = nullptr;
        return E_NOINTERFACE;

        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    //=============================================================================
    JUCE_COMRESULT get_DocumentRange (ITextRangeProvider** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            *pRetVal = new UIATextRangeProvider (*this, { 0, textInterface.getTotalNumCharacters() });
            return S_OK;
        });
    }

    JUCE_COMRESULT get_SupportedTextSelection (SupportedTextSelection* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = SupportedTextSelection_Single;
            return S_OK;
        });
    }

    JUCE_COMRESULT GetSelection (SAFEARRAY** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, 1);

            if (pRetVal != nullptr)
            {
                auto selection = textInterface.getSelection();
                auto hasSelection = ! selection.isEmpty();
                auto cursorPos = textInterface.getTextInsertionOffset();

                auto* rangeProvider = new UIATextRangeProvider (*this,
                                                                { hasSelection ? selection.getStart() : cursorPos,
                                                                  hasSelection ? selection.getEnd()   : cursorPos });

                LONG pos = 0;
                auto hr = SafeArrayPutElement (*pRetVal, &pos, static_cast<IUnknown*> (rangeProvider));

                if (FAILED (hr))
                    return E_FAIL;

                rangeProvider->Release();
            }

            return S_OK;
        });
    }

    JUCE_COMRESULT GetVisibleRanges (SAFEARRAY** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, 1);

            if (pRetVal != nullptr)
            {
                auto* rangeProvider = new UIATextRangeProvider (*this, { 0, textInterface.getTotalNumCharacters() });

                LONG pos = 0;
                auto hr = SafeArrayPutElement (*pRetVal, &pos, static_cast<IUnknown*> (rangeProvider));

                if (FAILED (hr))
                    return E_FAIL;

                rangeProvider->Release();
            }

            return S_OK;
        });
    }

    JUCE_COMRESULT RangeFromChild (IRawElementProviderSimple*, ITextRangeProvider** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, []
        {
            return S_OK;
        });
    }

    JUCE_COMRESULT RangeFromPoint (UiaPoint point, ITextRangeProvider** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            auto offset = textInterface.getOffsetAtPoint ({ roundToInt (point.x), roundToInt (point.y) });

            if (offset > 0)
                *pRetVal = new UIATextRangeProvider (*this, { offset, offset });

            return S_OK;
        });
    }

    //==============================================================================
    JUCE_COMRESULT GetCaretRange (BOOL* isActive, ITextRangeProvider** pRetVal) override
    {
        return withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
        {
            *isActive = getHandler().hasFocus (false);

            auto cursorPos = textInterface.getTextInsertionOffset();
            *pRetVal = new UIATextRangeProvider (*this, { cursorPos, cursorPos });

            return S_OK;
        });
    }

    JUCE_COMRESULT RangeFromAnnotation (IRawElementProviderSimple*, ITextRangeProvider** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, []
        {
            return S_OK;
        });
    }

private:
    //==============================================================================
    template <typename Value, typename Callback>
    JUCE_COMRESULT withTextInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* textInterface = getHandler().getTextInterface())
                return callback (*textInterface);

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    class UIATextRangeProvider  : public UIAProviderBase,
                                  public ComBaseClassHelper<ITextRangeProvider>
    {
    public:
        UIATextRangeProvider (UIATextProvider& textProvider, Range<int> range)
            : UIAProviderBase (textProvider.getHandler().getNativeImplementation()),
              owner (&textProvider),
              selectionRange (range)
        {
        }

        //==============================================================================
        Range<int> getSelectionRange() const noexcept  { return selectionRange; }

        //==============================================================================
        JUCE_COMRESULT AddToSelection() override
        {
            return Select();
        }

        JUCE_COMRESULT Clone (ITextRangeProvider** pRetVal) override
        {
            return withCheckedComArgs (pRetVal, *this, [&]
            {
                *pRetVal = new UIATextRangeProvider (*owner, selectionRange);
                return S_OK;
            });
        }

        JUCE_COMRESULT Compare (ITextRangeProvider* range, BOOL* pRetVal) override
        {
            return withCheckedComArgs (pRetVal, *this, [&]
            {
                *pRetVal = (selectionRange == static_cast<UIATextRangeProvider*> (range)->getSelectionRange());
                return S_OK;
            });
        }

        JUCE_COMRESULT CompareEndpoints (TextPatternRangeEndpoint endpoint,
                                         ITextRangeProvider* targetRange,
                                         TextPatternRangeEndpoint targetEndpoint,
                                         int* pRetVal) override
        {
            if (targetRange == nullptr)
                return E_INVALIDARG;

            return withCheckedComArgs (pRetVal, *this, [&]
            {
                auto offset = (endpoint == TextPatternRangeEndpoint_Start ? selectionRange.getStart()
                                                                          : selectionRange.getEnd());

                auto otherRange = static_cast<UIATextRangeProvider*> (targetRange)->getSelectionRange();
                auto otherOffset = (targetEndpoint == TextPatternRangeEndpoint_Start ? otherRange.getStart()
                                                                                     : otherRange.getEnd());

                *pRetVal = offset - otherOffset;
                return S_OK;
            });
        }

        JUCE_COMRESULT ExpandToEnclosingUnit (TextUnit unit) override
        {
            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            if (auto* textInterface = owner->getHandler().getTextInterface())
            {
                const auto boundaryType = getBoundaryType (unit);

                const auto start = AccessibilityTextHelpers::findTextBoundary (*textInterface,
                                                                               selectionRange.getStart(),
                                                                               boundaryType,
                                                                               AccessibilityTextHelpers::Direction::backwards);

                const auto end = AccessibilityTextHelpers::findTextBoundary (*textInterface,
                                                                             start,
                                                                             boundaryType,
                                                                             AccessibilityTextHelpers::Direction::forwards);

                selectionRange = Range<int> (start, end);

                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

        JUCE_COMRESULT FindAttribute (TEXTATTRIBUTEID, VARIANT, BOOL, ITextRangeProvider** pRetVal) override
        {
            return withCheckedComArgs (pRetVal, *this, []
            {
                return S_OK;
            });
        }

        JUCE_COMRESULT FindText (BSTR text, BOOL backward, BOOL ignoreCase,
                                 ITextRangeProvider** pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                auto selectionText = textInterface.getText (selectionRange);
                String textToSearchFor (text);

                auto offset = (backward ? (ignoreCase ? selectionText.lastIndexOfIgnoreCase (textToSearchFor) : selectionText.lastIndexOf (textToSearchFor))
                                        : (ignoreCase ? selectionText.indexOfIgnoreCase (textToSearchFor)     : selectionText.indexOf (textToSearchFor)));

                if (offset != -1)
                    *pRetVal = new UIATextRangeProvider (*owner, { offset, offset + textToSearchFor.length() });

                return S_OK;
            });
        }

        JUCE_COMRESULT GetAttributeValue (TEXTATTRIBUTEID attributeId, VARIANT* pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                VariantHelpers::clear (pRetVal);

                switch (attributeId)
                {
                    case UIA_IsReadOnlyAttributeId:
                    {
                        VariantHelpers::setBool (textInterface.isReadOnly(), pRetVal);
                        break;
                    }

                    case UIA_CaretPositionAttributeId:
                    {
                        auto cursorPos = textInterface.getTextInsertionOffset();

                        auto caretPos = [&]
                        {
                            if (cursorPos == 0)
                                return CaretPosition_BeginningOfLine;

                            if (cursorPos == textInterface.getTotalNumCharacters())
                                return CaretPosition_EndOfLine;

                            return CaretPosition_Unknown;
                        }();

                        VariantHelpers::setInt (caretPos, pRetVal);
                        break;
                    }
                }

                return S_OK;
            });
        }

        JUCE_COMRESULT GetBoundingRectangles (SAFEARRAY** pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                auto rectangleList = textInterface.getTextBounds (selectionRange);
                auto numRectangles = rectangleList.getNumRectangles();

                *pRetVal = SafeArrayCreateVector (VT_R8, 0, 4 * (ULONG) numRectangles);

                if (*pRetVal == nullptr)
                    return E_FAIL;

                if (numRectangles > 0)
                {
                    double* doubleArr = nullptr;

                    if (FAILED (SafeArrayAccessData (*pRetVal, reinterpret_cast<void**> (&doubleArr))))
                    {
                        SafeArrayDestroy (*pRetVal);
                        return E_FAIL;
                    }

                    for (int i = 0; i < numRectangles; ++i)
                    {
                        auto r = Desktop::getInstance().getDisplays().logicalToPhysical (rectangleList.getRectangle (i));

                        doubleArr[i * 4]     = r.getX();
                        doubleArr[i * 4 + 1] = r.getY();
                        doubleArr[i * 4 + 2] = r.getWidth();
                        doubleArr[i * 4 + 3] = r.getHeight();
                    }

                    if (FAILED (SafeArrayUnaccessData (*pRetVal)))
                    {
                        SafeArrayDestroy (*pRetVal);
                        return E_FAIL;
                    }
                }

                return S_OK;
            });
        }

        JUCE_COMRESULT GetChildren (SAFEARRAY** pRetVal) override
        {
            return withCheckedComArgs (pRetVal, *this, [&]
            {
                *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, 0);
                return S_OK;
            });
        }

        JUCE_COMRESULT GetEnclosingElement (IRawElementProviderSimple** pRetVal) override
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

            return withCheckedComArgs (pRetVal, *this, [&]
            {
                getHandler().getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
                return S_OK;
            });

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }

        JUCE_COMRESULT GetText (int maxLength, BSTR* pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                auto text = textInterface.getText (selectionRange);

                if (maxLength >= 0 && text.length() > maxLength)
                    text = text.substring (0, maxLength);

                *pRetVal = SysAllocString ((const OLECHAR*) text.toWideCharPointer());
                return S_OK;
            });
        }

        JUCE_COMRESULT Move (TextUnit unit, int count, int* pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface&)
            {
                if (count > 0)
                {
                    MoveEndpointByUnit (TextPatternRangeEndpoint_End, unit, count, pRetVal);
                    MoveEndpointByUnit (TextPatternRangeEndpoint_Start, unit, count, pRetVal);
                }
                else if (count < 0)
                {
                    MoveEndpointByUnit (TextPatternRangeEndpoint_Start, unit, count, pRetVal);
                    MoveEndpointByUnit (TextPatternRangeEndpoint_End, unit, count, pRetVal);
                }

                return S_OK;
            });
        }

        JUCE_COMRESULT MoveEndpointByRange (TextPatternRangeEndpoint endpoint,
                                            ITextRangeProvider* targetRange,
                                            TextPatternRangeEndpoint targetEndpoint) override
        {
            if (targetRange == nullptr)
                return E_INVALIDARG;

            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            if (auto* textInterface = owner->getHandler().getTextInterface())
            {
                auto otherRange = static_cast<UIATextRangeProvider*> (targetRange)->getSelectionRange();
                auto targetPoint = (targetEndpoint == TextPatternRangeEndpoint_Start ? otherRange.getStart()
                                                                                     : otherRange.getEnd());

                setEndpointChecked (endpoint, targetPoint);
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

        JUCE_COMRESULT MoveEndpointByUnit (TextPatternRangeEndpoint endpoint,
                                           TextUnit unit,
                                           int count,
                                           int* pRetVal) override
        {
            return owner->withTextInterface (pRetVal, [&] (const AccessibilityTextInterface& textInterface)
            {
                if (count == 0 || textInterface.getTotalNumCharacters() == 0)
                    return S_OK;

                auto endpointToMove = (endpoint == TextPatternRangeEndpoint_Start ? selectionRange.getStart()
                                                                                  : selectionRange.getEnd());

                const auto direction = (count > 0 ? AccessibilityTextHelpers::Direction::forwards
                                                  : AccessibilityTextHelpers::Direction::backwards);

                const auto boundaryType = getBoundaryType (unit);

                // handle case where endpoint is on a boundary
                if (AccessibilityTextHelpers::findTextBoundary (textInterface, endpointToMove, boundaryType, direction) == endpointToMove)
                    endpointToMove += (direction == AccessibilityTextHelpers::Direction::forwards ? 1 : -1);

                int numMoved;
                for (numMoved = 0; numMoved < std::abs (count); ++numMoved)
                {
                    auto nextEndpoint = AccessibilityTextHelpers::findTextBoundary (textInterface,
                                                                                    endpointToMove,
                                                                                    boundaryType,
                                                                                    direction);

                    if (nextEndpoint == endpointToMove)
                        break;

                    endpointToMove = nextEndpoint;
                }

                *pRetVal = numMoved;
                setEndpointChecked (endpoint, endpointToMove);

                return S_OK;
            });
        }

        JUCE_COMRESULT RemoveFromSelection() override
        {
            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            if (auto* textInterface = owner->getHandler().getTextInterface())
            {
                textInterface->setSelection ({});
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

        JUCE_COMRESULT ScrollIntoView (BOOL) override
        {
            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

        JUCE_COMRESULT Select() override
        {
            if (! isElementValid())
                return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

            if (auto* textInterface = owner->getHandler().getTextInterface())
            {
                textInterface->setSelection ({});
                textInterface->setSelection (selectionRange);

                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        }

    private:
        static AccessibilityTextHelpers::BoundaryType getBoundaryType (TextUnit unit)
        {
            switch (unit)
            {
                case TextUnit_Character:
                    return AccessibilityTextHelpers::BoundaryType::character;

                case TextUnit_Format:
                case TextUnit_Word:
                    return AccessibilityTextHelpers::BoundaryType::word;

                case TextUnit_Line:
                    return AccessibilityTextHelpers::BoundaryType::line;

                case TextUnit_Paragraph:
                case TextUnit_Page:
                case TextUnit_Document:
                    return AccessibilityTextHelpers::BoundaryType::document;
            };

            jassertfalse;
            return AccessibilityTextHelpers::BoundaryType::character;
        }

        void setEndpointChecked (TextPatternRangeEndpoint endpoint, int newEndpoint)
        {
            if (endpoint == TextPatternRangeEndpoint_Start)
            {
                if (selectionRange.getEnd() < newEndpoint)
                    selectionRange.setEnd (newEndpoint);

                selectionRange.setStart (newEndpoint);
            }
            else
            {
                if (selectionRange.getStart() > newEndpoint)
                    selectionRange.setStart (newEndpoint);

                selectionRange.setEnd (newEndpoint);
            }
        }

        ComSmartPtr<UIATextProvider> owner;
        Range<int> selectionRange;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIATextRangeProvider)
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIATextProvider)
};

} // namespace juce
