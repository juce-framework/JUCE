/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_TextButton.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
TextButton::TextButton (const String& name,
                        const String& toolTip)
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
    getLookAndFeel().drawButtonBackground (g, *this,
                                           findColour (getToggleState() ? buttonOnColourId
                                                                        : buttonColourId),
                                           isMouseOverButton,
                                           isButtonDown);

    const int yIndent = jmin (4, proportionOfHeight (0.3f));
    const int cornerSize = jmin (getHeight(), getWidth()) / 2;

    g.setFont (getFont());
    g.setColour (findColour (textColourId).withMultipliedAlpha (isEnabled() ? 1.0f : 0.5f));

    const int fontHeight = roundFloatToInt (g.getCurrentFont().getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (isConnectedOnRight() ? 4 : 2));

    g.drawFittedText (getButtonText(),
                      leftIndent,
                      yIndent,
                      getWidth() - leftIndent - rightIndent,
                      getHeight() - yIndent * 2,
                      Justification::centred, 2);
}

void TextButton::colourChanged()
{
    repaint();
}

const Font TextButton::getFont()
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


END_JUCE_NAMESPACE
