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
class UIAValueProvider  : public UIAProviderBase,
                          public ComBaseClassHelper<ComTypes::IValueProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT SetValue (LPCWSTR val) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();
        auto& valueInterface = *handler.getValueInterface();

        if (valueInterface.isReadOnly())
            return (HRESULT) UIA_E_NOTSUPPORTED;

        valueInterface.setValueAsString (String (val));

        VARIANT newValue;
        VariantHelpers::setString (valueInterface.getCurrentValueAsString(), &newValue);

        sendAccessibilityPropertyChangedEvent (handler, UIA_ValueValuePropertyId, newValue);

        return S_OK;
    }

    JUCE_COMRESULT get_Value (BSTR* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            auto currentValueString = getHandler().getValueInterface()->getCurrentValueAsString();

            *pRetVal = SysAllocString ((const OLECHAR*) currentValueString.toWideCharPointer());
            return S_OK;
        });
    }

    JUCE_COMRESULT get_IsReadOnly (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = getHandler().getValueInterface()->isReadOnly();
            return S_OK;
        });
    }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAValueProvider)
};

} // namespace juce
