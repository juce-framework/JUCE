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

#include "juce_HyperlinkButton.h"


//==============================================================================
HyperlinkButton::HyperlinkButton (const String& linkText,
                                  const URL& linkURL)
   : Button (linkText),
     url (linkURL),
     font (14.0f, Font::underlined),
     resizeFont (true),
     justification (Justification::centred)
{
    setMouseCursor (MouseCursor::PointingHandCursor);
    setTooltip (linkURL.toString (false));
}

HyperlinkButton::~HyperlinkButton()
{
}

//==============================================================================
void HyperlinkButton::setFont (const Font& newFont,
                               const bool resizeToMatchComponentHeight,
                               const Justification& justificationType)
{
    font = newFont;
    resizeFont = resizeToMatchComponentHeight;
    justification = justificationType;
    repaint();
}

void HyperlinkButton::setURL (const URL& newURL) throw()
{
    url = newURL;
    setTooltip (newURL.toString (false));
}

const Font HyperlinkButton::getFontToUse() const
{
    Font f (font);

    if (resizeFont)
        f.setHeight (getHeight() * 0.7f);

    return f;
}

void HyperlinkButton::changeWidthToFitText()
{
    setSize (getFontToUse().getStringWidth (getName()) + 6, getHeight());
}

void HyperlinkButton::colourChanged()
{
    repaint();
}

//==============================================================================
void HyperlinkButton::clicked()
{
    if (url.isWellFormed())
        url.launchInDefaultBrowser();
}

void HyperlinkButton::paintButton (Graphics& g,
                                   bool isMouseOverButton,
                                   bool isButtonDown)
{
    const Colour textColour (findColour (textColourId));

    if (isEnabled())
        g.setColour ((isMouseOverButton) ? textColour.darker ((isButtonDown) ? 1.3f : 0.4f)
                                         : textColour);
    else
        g.setColour (textColour.withMultipliedAlpha (0.4f));

    g.setFont (getFontToUse());

    g.drawText (getButtonText(),
                2, 0, getWidth() - 2, getHeight(),
                justification.getOnlyHorizontalFlags() | Justification::verticallyCentred,
                true);
}


END_JUCE_NAMESPACE
