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
class UIARangeValueProvider  : public UIAProviderBase,
                               public ComBaseClassHelper<IRangeValueProvider>
{
public:
    explicit UIARangeValueProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle)
    {
    }

    //==============================================================================
    JUCE_COMRESULT SetValue (double val) override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

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

        return UIA_E_NOTSUPPORTED;
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

            return UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIARangeValueProvider)
};

} // namespace juce
