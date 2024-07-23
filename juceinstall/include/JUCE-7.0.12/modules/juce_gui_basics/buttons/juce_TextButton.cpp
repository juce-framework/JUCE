/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

TextButton::TextButton()  : Button (String())
{
}

TextButton::TextButton (const String& name) : Button (name)
{
}

TextButton::TextButton (const String& name, const String& toolTip)  : Button (name)
{
    setTooltip (toolTip);
}

TextButton::~TextButton()
{
}

void TextButton::paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto& lf = getLookAndFeel();

    lf.drawButtonBackground (g, *this,
                             findColour (getToggleState() ? buttonOnColourId : buttonColourId),
                             shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    lf.drawButtonText (g, *this, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
}

void TextButton::colourChanged()
{
    repaint();
}

void TextButton::changeWidthToFitText()
{
    changeWidthToFitText (getHeight());
}

void TextButton::changeWidthToFitText (const int newHeight)
{
    setSize (getBestWidthForHeight (newHeight), newHeight);
}

int TextButton::getBestWidthForHeight (int buttonHeight)
{
    return getLookAndFeel().getTextButtonWidthToFitText (*this, buttonHeight);
}

} // namespace juce
