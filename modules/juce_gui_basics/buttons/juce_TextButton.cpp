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
