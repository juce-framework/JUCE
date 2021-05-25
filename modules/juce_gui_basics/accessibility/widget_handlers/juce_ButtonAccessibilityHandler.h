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

/** Basic accessible interface for a Button that can be clicked or toggled.

    @tags{Accessibility}
*/
class JUCE_API  ButtonAccessibilityHandler  : public AccessibilityHandler
{
public:
    explicit ButtonAccessibilityHandler (Button& buttonToWrap)
        : AccessibilityHandler (buttonToWrap,
                                getButtonRole (buttonToWrap),
                                getAccessibilityActions (buttonToWrap)),
          button (buttonToWrap)
    {
    }

    AccessibleState getCurrentState() const override
    {
        auto state = AccessibilityHandler::getCurrentState();

        if (button.getClickingTogglesState())
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

private:
    static AccessibilityRole getButtonRole (const Button& b)
    {
        if (b.getRadioGroupId() != 0)     return AccessibilityRole::radioButton;
        if (b.getClickingTogglesState())  return AccessibilityRole::toggleButton;

        return AccessibilityRole::button;
    }

    static AccessibilityActions getAccessibilityActions (Button& button)
    {
        auto actions = AccessibilityActions().addAction (AccessibilityActionType::press,
                                                         [&button] { button.triggerClick(); });

        if (button.getClickingTogglesState())
            actions = actions.addAction (AccessibilityActionType::toggle,
                                         [&button] { button.setToggleState (! button.getToggleState(), sendNotification); });

        return actions;
    }

    Button& button;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonAccessibilityHandler)
};

} // namespace juce
