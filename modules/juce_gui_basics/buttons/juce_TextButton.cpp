/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

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

void TextButton::paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    LookAndFeel& lf = getLookAndFeel();

    lf.drawButtonBackground (g, *this,
                             findColour (getToggleState() ? buttonOnColourId : buttonColourId),
                             isMouseOverButton, isButtonDown);

    lf.drawButtonText (g, *this, isMouseOverButton, isButtonDown);
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
