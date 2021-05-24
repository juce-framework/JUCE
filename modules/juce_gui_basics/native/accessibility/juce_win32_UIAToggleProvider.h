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
class UIAToggleProvider  : public UIAProviderBase,
                           public ComBaseClassHelper<IToggleProvider>
{
public:
    explicit UIAToggleProvider (AccessibilityNativeHandle* nativeHandle)
        : UIAProviderBase (nativeHandle)
    {
    }

    //==============================================================================
    JUCE_COMRESULT Toggle() override
    {
        if (! isElementValid())
            return UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();

        if (handler.getActions().invoke (AccessibilityActionType::toggle))
        {
            VARIANT newValue;
            VariantHelpers::setInt (getCurrentToggleState(), &newValue);

            sendAccessibilityPropertyChangedEvent (handler, UIA_ToggleToggleStatePropertyId, newValue);

            return S_OK;
        }

        return UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT get_ToggleState (ToggleState* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = getCurrentToggleState();
            return S_OK;
        });
    }

private:
    ToggleState getCurrentToggleState() const
    {
        return getHandler().getCurrentState().isChecked() ? ToggleState_On
                                                          : ToggleState_Off;
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAToggleProvider)
};

} // namespace juce
