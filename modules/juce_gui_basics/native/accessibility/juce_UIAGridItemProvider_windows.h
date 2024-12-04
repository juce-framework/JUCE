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

//==============================================================================
class UIAGridItemProvider  : public UIAProviderBase,
                             public ComBaseClassHelper<IGridItemProvider, ITableItemProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT get_Row (int* pRetVal) override
    {
        return withTableSpan (pRetVal,
                              &AccessibilityTableInterface::getRowSpan,
                              &AccessibilityTableInterface::Span::begin);
    }

    JUCE_COMRESULT get_Column (int* pRetVal) override
    {
        return withTableSpan (pRetVal,
                              &AccessibilityTableInterface::getColumnSpan,
                              &AccessibilityTableInterface::Span::begin);
    }

    JUCE_COMRESULT get_RowSpan (int* pRetVal) override
    {
        return withTableSpan (pRetVal,
                              &AccessibilityTableInterface::getRowSpan,
                              &AccessibilityTableInterface::Span::num);
    }

    JUCE_COMRESULT get_ColumnSpan (int* pRetVal) override
    {
        return withTableSpan (pRetVal,
                              &AccessibilityTableInterface::getColumnSpan,
                              &AccessibilityTableInterface::Span::num);
    }

    JUCE_COMRESULT get_ContainingGrid (IRawElementProviderSimple** pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityHandler& tableHandler)
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            tableHandler.getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            return true;
        });
    }

    JUCE_COMRESULT GetRowHeaderItems (SAFEARRAY**) override
    {
        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT GetColumnHeaderItems (SAFEARRAY** pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityHandler& tableHandler)
        {
            if (auto* tableInterface = tableHandler.getTableInterface())
            {
                if (const auto column = tableInterface->getColumnSpan (getHandler()))
                {
                    if (auto* header = tableInterface->getHeaderHandler())
                    {
                        const auto children = header->getChildren();

                        if (isPositiveAndBelow (column->begin, children.size()))
                        {
                            ComSmartPtr<IRawElementProviderSimple> provider;

                            if (auto* child = children[(size_t) column->begin])
                            {
                                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
                                if (child->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress())) == S_OK && provider != nullptr)
                                {
                                    *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, 1);
                                    LONG index = 0;
                                    const auto hr = SafeArrayPutElement (*pRetVal, &index, provider);

                                    return ! FAILED (hr);
                                }
                                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
                            }
                        }
                    }
                }
            }

            return false;
        });
    }
private:
    template <typename Value, typename Callback>
    JUCE_COMRESULT withTableInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* handler = detail::AccessibilityHelpers::getEnclosingHandlerWithInterface (&getHandler(), &AccessibilityHandler::getTableInterface))
                if (handler->getTableInterface() != nullptr && callback (*handler))
                    return S_OK;

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT withTableSpan (int* pRetVal,
                                  Optional<AccessibilityTableInterface::Span> (AccessibilityTableInterface::* getSpan) (const AccessibilityHandler&) const,
                                  int AccessibilityTableInterface::Span::* spanMember) const
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityHandler& handler)
        {
            if (const auto span = ((handler.getTableInterface())->*getSpan) (getHandler()))
            {
                *pRetVal = (*span).*spanMember;
                return true;
            }

            return false;
        });
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAGridItemProvider)
};

} // namespace juce
