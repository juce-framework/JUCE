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

namespace juce::detail
{

//==============================================================================
class ButtonAccessibilityHandler  : public AccessibilityHandler
{
public:
    ButtonAccessibilityHandler (Button& buttonToWrap, AccessibilityRole roleIn)
        : AccessibilityHandler (buttonToWrap,
                                isRadioButton (buttonToWrap) ? AccessibilityRole::radioButton : roleIn,
                                getAccessibilityActions (buttonToWrap),
                                getAccessibilityInterfaces (buttonToWrap)),
          button (buttonToWrap)
    {}


    AccessibleState getCurrentState() const override
    {
        auto state = AccessibilityHandler::getCurrentState();

        if (button.isToggleable())
        {
            state = state.withCheckable();

            if (button.getToggleState())
                state = state.withChecked();
        }

        return state;
    }


    String getTitle() const override
    {
        auto title = AccessibilityHandler::getTitle();

        if (title.isEmpty())
            return button.getButtonText();

        return title;
    }

    String getHelp() const override
    {
        return button.getTooltip();
    }


private:
    class ButtonValueInterface  : public AccessibilityTextValueInterface
    {
    public:
        explicit ButtonValueInterface (Button& buttonToWrap)
            : button (buttonToWrap)
        {
        }

        bool isReadOnly() const override                 { return true; }
        String getCurrentValueAsString() const override  { return button.getToggleState() ? "On" : "Off"; }
        void setValueAsString (const String&) override   {}

    private:
        Button& button;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonValueInterface)
    };

    static bool isRadioButton (const Button& button) noexcept
    {
        return button.getRadioGroupId() != 0;
    }

    static AccessibilityActions getAccessibilityActions (Button& button)
    {
        auto actions = AccessibilityActions().addAction (AccessibilityActionType::press,
                                                         [&button] { button.triggerClick(); });

        if (button.isToggleable())
            actions = actions.addAction (AccessibilityActionType::toggle,
                                         [&button] { button.setToggleState (! button.getToggleState(), sendNotification); });

        return actions;
    }

    static Interfaces getAccessibilityInterfaces (Button& button)
    {
        if (button.isToggleable())
            return { std::make_unique<ButtonValueInterface> (button) };

        return {};
    }

    Button& button;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonAccessibilityHandler)
};

} // namespace juce::detail
