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

ToggleButton::ToggleButton()
    : Button (String())
{
    setClickingTogglesState (true);
}

ToggleButton::ToggleButton (const String& buttonText)
    : Button (buttonText)
{
    setClickingTogglesState (true);
}

ToggleButton::~ToggleButton()
{
}

void ToggleButton::paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    getLookAndFeel().drawToggleButton (g, *this, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void ToggleButton::changeWidthToFitText()
{
    getLookAndFeel().changeToggleButtonWidthToFitText (*this);
}

void ToggleButton::colourChanged()
{
    repaint();
}

std::unique_ptr<AccessibilityHandler> ToggleButton::createAccessibilityHandler()
{
    return std::make_unique<ButtonAccessibilityHandler> (*this, AccessibilityRole::toggleButton);
}

} // namespace juce
