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
class UIARangeValueProvider  : public UIAProviderBase,
                               public ComBaseClassHelper<IRangeValueProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT SetValue (double val) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();

        if (auto* valueInterface = handler.getValueInterface())
        {
            auto range = valueInterface->getRange();

            if (range.isValid())
            {
                if (val < range.getMinimumValue() || val > range.getMaximumValue())
                    return E_INVALIDARG;

                if (! valueInterface->isReadOnly())
                {
                    valueInterface->setValue (val);

                    VARIANT newValue;
                    VariantHelpers::setDouble (valueInterface->getCurrentValue(), &newValue);
                    sendAccessibilityPropertyChangedEvent (handler, UIA_RangeValueValuePropertyId, newValue);

                    return S_OK;
                }
            }
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT get_Value (double* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.getCurrentValue();
        });
    }

    JUCE_COMRESULT get_IsReadOnly (BOOL* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.isReadOnly();
        });
    }

    JUCE_COMRESULT get_Maximum (double* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.getRange().getMaximumValue();
        });
    }

    JUCE_COMRESULT get_Minimum (double* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.getRange().getMinimumValue();
        });
    }

    JUCE_COMRESULT get_LargeChange (double* pRetVal) override
    {
        return get_SmallChange (pRetVal);
    }

    JUCE_COMRESULT get_SmallChange (double* pRetVal) override
    {
        return withValueInterface (pRetVal, [] (const AccessibilityValueInterface& valueInterface)
        {
            return valueInterface.getRange().getInterval();
        });
    }

private:
    template <typename Value, typename Callback>
    JUCE_COMRESULT withValueInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* valueInterface = getHandler().getValueInterface())
            {
                if (valueInterface->getRange().isValid())
                {
                    *pRetVal = callback (*valueInterface);
                    return S_OK;
                }
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIARangeValueProvider)
};

} // namespace juce
