/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

int AccessibilityNativeHandle::idCounter = 0;

//==============================================================================
class UIAScrollProvider final : public UIAProviderBase,
                                public ComBaseClassHelper<IScrollProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    JUCE_COMCALL Scroll (ScrollAmount, ScrollAmount) override { return E_FAIL; }
    JUCE_COMCALL SetScrollPercent (double, double) override { return E_FAIL; }
    JUCE_COMCALL get_HorizontalScrollPercent (double*) override { return E_FAIL; }
    JUCE_COMCALL get_VerticalScrollPercent (double*) override { return E_FAIL; }
    JUCE_COMCALL get_HorizontalViewSize (double*) override { return E_FAIL; }
    JUCE_COMCALL get_VerticalViewSize (double*) override { return E_FAIL; }
    JUCE_COMCALL get_HorizontallyScrollable (BOOL*) override { return E_FAIL; }
    JUCE_COMCALL get_VerticallyScrollable (BOOL*) override { return E_FAIL; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAScrollProvider)
};

class UIAScrollItemProvider final : public UIAProviderBase,
                                    public ComBaseClassHelper<IScrollItemProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    JUCE_COMCALL ScrollIntoView() override
    {
        if (auto* handler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (&getHandler(), &AccessibilityHandler::getTableInterface))
        {
            if (auto* tableInterface = handler->getTableInterface())
            {
                tableInterface->showCell (getHandler());
                return S_OK;
            }
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAScrollItemProvider)
};

//==============================================================================
static String getAutomationId (const AccessibilityHandler& handler)
{
    auto result = handler.getTitle();
    auto* parentComponent = handler.getComponent().getParentComponent();

    while (parentComponent != nullptr)
    {
        if (auto* parentHandler = parentComponent->getAccessibilityHandler())
        {
            auto parentTitle = parentHandler->getTitle();
            result << "." << (parentTitle.isNotEmpty() ? parentTitle : "<empty>");
        }

        parentComponent = parentComponent->getParentComponent();
    }

    return result;
}

static auto roleToControlTypeId (AccessibilityRole roleType)
{
    switch (roleType)
    {
        case AccessibilityRole::popupMenu:
        case AccessibilityRole::dialogWindow:
        case AccessibilityRole::splashScreen:
        case AccessibilityRole::window:        return UIA_WindowControlTypeId;

        case AccessibilityRole::label:
        case AccessibilityRole::staticText:    return UIA_TextControlTypeId;

        case AccessibilityRole::column:
        case AccessibilityRole::row:           return UIA_ListItemControlTypeId;

        case AccessibilityRole::button:        return UIA_ButtonControlTypeId;
        case AccessibilityRole::toggleButton:  return UIA_CheckBoxControlTypeId;
        case AccessibilityRole::radioButton:   return UIA_RadioButtonControlTypeId;
        case AccessibilityRole::comboBox:      return UIA_ComboBoxControlTypeId;
        case AccessibilityRole::image:         return UIA_ImageControlTypeId;
        case AccessibilityRole::slider:        return UIA_SliderControlTypeId;
        case AccessibilityRole::editableText:  return UIA_EditControlTypeId;
        case AccessibilityRole::menuItem:      return UIA_MenuItemControlTypeId;
        case AccessibilityRole::menuBar:       return UIA_MenuBarControlTypeId;
        case AccessibilityRole::table:         return UIA_TableControlTypeId;
        case AccessibilityRole::tableHeader:   return UIA_HeaderControlTypeId;
        case AccessibilityRole::cell:          return UIA_DataItemControlTypeId;
        case AccessibilityRole::hyperlink:     return UIA_HyperlinkControlTypeId;
        case AccessibilityRole::list:          return UIA_ListControlTypeId;
        case AccessibilityRole::listItem:      return UIA_ListItemControlTypeId;
        case AccessibilityRole::tree:          return UIA_TreeControlTypeId;
        case AccessibilityRole::treeItem:      return UIA_TreeItemControlTypeId;
        case AccessibilityRole::progressBar:   return UIA_ProgressBarControlTypeId;
        case AccessibilityRole::group:         return UIA_GroupControlTypeId;
        case AccessibilityRole::scrollBar:     return UIA_ScrollBarControlTypeId;
        case AccessibilityRole::tooltip:       return UIA_ToolTipControlTypeId;

        case AccessibilityRole::ignored:
        case AccessibilityRole::unspecified:   break;
    };

    return UIA_CustomControlTypeId;
}

//==============================================================================
AccessibilityNativeHandle::AccessibilityNativeHandle (AccessibilityHandler& handler)
    : accessibilityHandler (handler)
{
}

//==============================================================================
JUCE_COMRESULT AccessibilityNativeHandle::QueryInterface (REFIID refId, void** result)
{
    *result = nullptr;

    if (! isElementValid())
        return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

    if ((refId == __uuidof (IRawElementProviderFragmentRoot) && ! isFragmentRoot()))
        return E_NOINTERFACE;

    return ComBaseClassHelper::QueryInterface (refId, result);
}

//==============================================================================
JUCE_COMRESULT AccessibilityNativeHandle::get_HostRawElementProvider (IRawElementProviderSimple** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        if (auto* wrapper = WindowsUIAWrapper::getInstanceWithoutCreating())
        {
            if (isFragmentRoot())
                return wrapper->hostProviderFromHwnd ((HWND) accessibilityHandler.getComponent().getWindowHandle(), pRetVal);

            if (auto* embeddedWindow = static_cast<HWND> (AccessibilityHandler::getNativeChildForComponent (accessibilityHandler.getComponent())))
                return wrapper->hostProviderFromHwnd (embeddedWindow, pRetVal);
        }

        return S_OK;
    });
}

JUCE_COMRESULT AccessibilityNativeHandle::get_ProviderOptions (ProviderOptions* options)
{
    if (options == nullptr)
        return E_INVALIDARG;

    *options = (ProviderOptions) (ProviderOptions_ServerSideProvider | ProviderOptions_UseComThreading);

    if (AccessibilityHandler::getNativeChildForComponent (accessibilityHandler.getComponent()) != nullptr)
        *options = (ProviderOptions) (*options | ProviderOptions_OverrideProvider);

    return S_OK;
}

JUCE_COMRESULT AccessibilityNativeHandle::GetPatternProvider (PATTERNID pId, IUnknown** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        *pRetVal = [&]() -> IUnknown*
        {
            const auto role = accessibilityHandler.getRole();
            const auto fragmentRoot = isFragmentRoot();

            const auto isListOrTableCell = [] (auto& handler)
            {
                if (auto* tableHandler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (&handler, &AccessibilityHandler::getTableInterface))
                {
                    if (auto* tableInterface = tableHandler->getTableInterface())
                    {
                        const auto row    = tableInterface->getRowSpan    (handler);
                        const auto column = tableInterface->getColumnSpan (handler);

                        return row.hasValue() && column.hasValue();
                    }
                }

                return false;
            };

            switch (pId)
            {
                case UIA_WindowPatternId:
                {
                    if (fragmentRoot)
                        return new UIAWindowProvider (this);

                    break;
                }
                case UIA_TransformPatternId:
                {
                    if (fragmentRoot)
                        return new UIATransformProvider (this);

                    break;
                }
                case UIA_TextPatternId:
                case UIA_TextPattern2Id:
                {
                    if (accessibilityHandler.getTextInterface() != nullptr)
                        return new UIATextProvider (this);

                    break;
                }
                case UIA_ValuePatternId:
                {
                    if (accessibilityHandler.getValueInterface() != nullptr)
                        return new UIAValueProvider (this);

                    break;
                }
                case UIA_RangeValuePatternId:
                {
                    if (accessibilityHandler.getValueInterface() != nullptr
                        && accessibilityHandler.getValueInterface()->getRange().isValid())
                    {
                        return new UIARangeValueProvider (this);
                    }

                    break;
                }
                case UIA_TogglePatternId:
                {
                    if (accessibilityHandler.getCurrentState().isCheckable()
                        && (accessibilityHandler.getActions().contains (AccessibilityActionType::toggle)
                            || accessibilityHandler.getActions().contains (AccessibilityActionType::press)))
                    {
                        return new UIAToggleProvider (this);
                    }

                    break;
                }
                case UIA_SelectionPatternId:
                {
                    if (role == AccessibilityRole::list
                        || role == AccessibilityRole::popupMenu
                        || role == AccessibilityRole::tree)
                    {
                        return new UIASelectionProvider (this);
                    }

                    break;
                }
                case UIA_SelectionItemPatternId:
                {
                    auto state = accessibilityHandler.getCurrentState();

                    if (state.isSelectable() || state.isMultiSelectable() || role == AccessibilityRole::radioButton)
                    {
                        return new UIASelectionItemProvider (this);
                    }

                    break;
                }
                case UIA_TablePatternId:
                case UIA_GridPatternId:
                {
                    if (accessibilityHandler.getTableInterface() != nullptr
                        && (pId == UIA_GridPatternId || accessibilityHandler.getRole() == AccessibilityRole::table))
                        return static_cast<IGridProvider*> (new UIAGridProvider (this));

                    break;
                }
                case UIA_TableItemPatternId:
                case UIA_GridItemPatternId:
                {
                    if (isListOrTableCell (accessibilityHandler))
                        return static_cast<IGridItemProvider*> (new UIAGridItemProvider (this));

                    break;
                }
                case UIA_InvokePatternId:
                {
                    if (accessibilityHandler.getActions().contains (AccessibilityActionType::press))
                        return new UIAInvokeProvider (this);

                    break;
                }
                case UIA_ExpandCollapsePatternId:
                {
                    if (accessibilityHandler.getActions().contains (AccessibilityActionType::showMenu)
                        && accessibilityHandler.getCurrentState().isExpandable())
                        return new UIAExpandCollapseProvider (this);

                    break;
                }
                case UIA_ScrollPatternId:
                {
                    if (accessibilityHandler.getTableInterface() != nullptr)
                        return new UIAScrollProvider (this);

                    break;
                }
                case UIA_ScrollItemPatternId:
                {
                    if (isListOrTableCell (accessibilityHandler))
                        return new UIAScrollItemProvider (this);

                    break;
                }
            }

            return nullptr;
        }();

        return S_OK;
    });
}

JUCE_COMRESULT AccessibilityNativeHandle::GetPropertyValue (PROPERTYID propertyId, VARIANT* pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        VariantHelpers::clear (pRetVal);

        const auto role    = accessibilityHandler.getRole();
        const auto state   = accessibilityHandler.getCurrentState();
        const auto ignored = accessibilityHandler.isIgnored();

        switch (propertyId)
        {
            case UIA_AutomationIdPropertyId:
                VariantHelpers::setString (getAutomationId (accessibilityHandler), pRetVal);
                break;
            case UIA_ControlTypePropertyId:
                VariantHelpers::setInt (roleToControlTypeId (role), pRetVal);
                break;
            case UIA_FrameworkIdPropertyId:
                VariantHelpers::setString ("JUCE", pRetVal);
                break;
            case UIA_FullDescriptionPropertyId:
                VariantHelpers::setString (accessibilityHandler.getDescription(), pRetVal);
                break;
            case UIA_HelpTextPropertyId:
                VariantHelpers::setString (accessibilityHandler.getHelp(), pRetVal);
                break;
            case UIA_IsContentElementPropertyId:
                VariantHelpers::setBool (! ignored && accessibilityHandler.isVisibleWithinParent(),
                                         pRetVal);
                break;
            case UIA_IsControlElementPropertyId:
                VariantHelpers::setBool (true, pRetVal);
                break;
            case UIA_IsDialogPropertyId:
                VariantHelpers::setBool (role == AccessibilityRole::dialogWindow, pRetVal);
                break;
            case UIA_IsEnabledPropertyId:
                VariantHelpers::setBool (accessibilityHandler.getComponent().isEnabled(), pRetVal);
                break;
            case UIA_IsKeyboardFocusablePropertyId:
                VariantHelpers::setBool (state.isFocusable(), pRetVal);
                break;
            case UIA_HasKeyboardFocusPropertyId:
                VariantHelpers::setBool (accessibilityHandler.hasFocus (true), pRetVal);
                break;
            case UIA_IsOffscreenPropertyId:
                VariantHelpers::setBool (! accessibilityHandler.isVisibleWithinParent(), pRetVal);
                break;
            case UIA_IsPasswordPropertyId:
                if (auto* textInterface = accessibilityHandler.getTextInterface())
                    VariantHelpers::setBool (textInterface->isDisplayingProtectedText(), pRetVal);

                break;
            case UIA_IsPeripheralPropertyId:
                VariantHelpers::setBool (role == AccessibilityRole::tooltip
                                         || role == AccessibilityRole::popupMenu
                                         || role == AccessibilityRole::splashScreen,
                                         pRetVal);
                break;
            case UIA_NamePropertyId:
                if (! ignored)
                     VariantHelpers::setString (getElementName(), pRetVal);

                break;
            case UIA_ProcessIdPropertyId:
                VariantHelpers::setInt ((int) GetCurrentProcessId(), pRetVal);
                break;
            case UIA_NativeWindowHandlePropertyId:
                if (isFragmentRoot())
                    VariantHelpers::setInt ((int) (pointer_sized_int) accessibilityHandler.getComponent().getWindowHandle(), pRetVal);

                break;
        }

        return S_OK;
    });
}

//==============================================================================
JUCE_COMRESULT AccessibilityNativeHandle::Navigate (NavigateDirection direction, IRawElementProviderFragment** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        auto* handler = [&]() -> AccessibilityHandler*
        {
            if (direction == NavigateDirection_Parent)
                return accessibilityHandler.getParent();

            if (direction == NavigateDirection_FirstChild
                || direction == NavigateDirection_LastChild)
            {
                auto children = accessibilityHandler.getChildren();

                return children.empty() ? nullptr
                                        : (direction == NavigateDirection_FirstChild ? children.front()
                                                                                     : children.back());
            }

            if (direction == NavigateDirection_NextSibling
                || direction == NavigateDirection_PreviousSibling)
            {
                if (auto* parent = accessibilityHandler.getParent())
                {
                    const auto siblings = parent->getChildren();
                    const auto iter = std::find (siblings.cbegin(), siblings.cend(), &accessibilityHandler);

                    if (iter == siblings.end())
                        return nullptr;

                    if (direction == NavigateDirection_NextSibling && iter != std::prev (siblings.cend()))
                        return *std::next (iter);

                    if (direction == NavigateDirection_PreviousSibling && iter != siblings.cbegin())
                        return *std::prev (iter);
                }
            }

            return nullptr;
        }();

        if (handler != nullptr)
            if (auto* provider = handler->getNativeImplementation())
                if (provider->isElementValid())
                    provider->QueryInterface (IID_PPV_ARGS (pRetVal));

        return S_OK;
    });
}

JUCE_COMRESULT AccessibilityNativeHandle::GetRuntimeId (SAFEARRAY** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        if (! isFragmentRoot())
        {
            *pRetVal = SafeArrayCreateVector (VT_I4, 0, 2);

            if (*pRetVal == nullptr)
                return E_OUTOFMEMORY;

            for (LONG i = 0; i < 2; ++i)
            {
                auto hr = SafeArrayPutElement (*pRetVal, &i, &rtid[(size_t) i]);

                if (FAILED (hr))
                    return E_FAIL;
            }
        }

        return S_OK;
    });
}

JUCE_COMRESULT AccessibilityNativeHandle::get_BoundingRectangle (UiaRect* pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        auto bounds = Desktop::getInstance().getDisplays()
                        .logicalToPhysical (accessibilityHandler.getComponent().getScreenBounds());

        pRetVal->left   = bounds.getX();
        pRetVal->top    = bounds.getY();
        pRetVal->width  = bounds.getWidth();
        pRetVal->height = bounds.getHeight();

        return S_OK;
    });
}

JUCE_COMRESULT AccessibilityNativeHandle::GetEmbeddedFragmentRoots (SAFEARRAY** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, []
    {
        return S_OK;
    });
}

JUCE_COMRESULT AccessibilityNativeHandle::SetFocus()
{
    if (! isElementValid())
        return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

    const WeakReference<Component> safeComponent (&accessibilityHandler.getComponent());

    accessibilityHandler.getActions().invoke (AccessibilityActionType::focus);

    if (safeComponent != nullptr)
        accessibilityHandler.grabFocus();

    return S_OK;
}

JUCE_COMRESULT AccessibilityNativeHandle::get_FragmentRoot (IRawElementProviderFragmentRoot** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
    {
        auto* handler = [&]() -> AccessibilityHandler*
        {
            if (isFragmentRoot())
                return &accessibilityHandler;

            if (auto* peer = accessibilityHandler.getComponent().getPeer())
                return peer->getComponent().getAccessibilityHandler();

            return nullptr;
        }();

        if (handler != nullptr)
        {
            handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
            return S_OK;
        }

        return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;
    });
}

//==============================================================================
JUCE_COMRESULT AccessibilityNativeHandle::ElementProviderFromPoint (double x, double y, IRawElementProviderFragment** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        auto* handler = [&]
        {
            auto logicalScreenPoint = Desktop::getInstance().getDisplays()
                                        .physicalToLogical (Point<int> (roundToInt (x),
                                                                        roundToInt (y)));

            if (auto* child = accessibilityHandler.getChildAt (logicalScreenPoint))
                return child;

            return &accessibilityHandler;
        }();

        handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

        return S_OK;
    });
}

JUCE_COMRESULT AccessibilityNativeHandle::GetFocus (IRawElementProviderFragment** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        const auto getFocusHandler = [this]() -> AccessibilityHandler*
        {
            if (auto* modal = Component::getCurrentlyModalComponent())
            {
                const auto& component = accessibilityHandler.getComponent();

                if (! component.isParentOf (modal)
                     && component.isCurrentlyBlockedByAnotherModalComponent())
                {
                    if (auto* modalHandler = modal->getAccessibilityHandler())
                    {
                        if (auto* focusChild = modalHandler->getChildFocus())
                            return focusChild;

                        return modalHandler;
                    }
                }
            }

            if (auto* focusChild = accessibilityHandler.getChildFocus())
                return focusChild;

            return nullptr;
        };

        if (auto* focusHandler = getFocusHandler())
            focusHandler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

        return S_OK;
    });
}

JUCE_COMRESULT AccessibilityNativeHandle::GetOverrideProviderForHwnd (HWND hwnd, IRawElementProviderSimple** pRetVal)
{
    return withCheckedComArgs (pRetVal, *this, [&]
    {
        if (auto* component = AccessibilityHandler::getComponentForNativeChild (hwnd))
            if (auto* handler = component->getAccessibilityHandler())
                handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

        return S_OK;
    });
}

//==============================================================================
String AccessibilityNativeHandle::getElementName() const
{
    if (accessibilityHandler.getRole() == AccessibilityRole::tooltip)
        return accessibilityHandler.getDescription();

    auto name = accessibilityHandler.getTitle();

    if (name.isEmpty() && isFragmentRoot())
        return detail::AccessibilityHelpers::getApplicationOrPluginName();

    return name;
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce
