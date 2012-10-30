/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

TextButton::TextButton (const String& name, const String& toolTip)
    : Button (name)
{
    setTooltip (toolTip);
}

TextButton::~TextButton()
{
}

void TextButton::paintButton (Graphics& g,
                              bool isMouseOverButton,
                              bool isButtonDown)
{
    LookAndFeel& lf = getLookAndFeel();

    lf.drawButtonBackground (g, *this,
                             findColour (getToggleState() ? buttonOnColourId
                                                          : buttonColourId),
                             isMouseOverButton,
                             isButtonDown);

    lf.drawButtonText (g, *this,
                       isMouseOverButton,
                       isButtonDown);
}

void TextButton::colourChanged()
{
    repaint();
}

Font TextButton::getFont()
{
    return Font (jmin (15.0f, getHeight() * 0.6f));
}

void TextButton::changeWidthToFitText (const int newHeight)
{
    if (newHeight >= 0)
        setSize (jmax (1, getWidth()), newHeight);

    setSize (getFont().getStringWidth (getButtonText()) + getHeight(),
             getHeight());
}
