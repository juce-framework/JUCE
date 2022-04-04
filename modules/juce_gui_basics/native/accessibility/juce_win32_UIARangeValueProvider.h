/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class UIARangeValueProvider  : public UIAProviderBase,
                               public ComBaseClassHelper<ComTypes::IRangeValueProvider>
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
