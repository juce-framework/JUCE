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

HyperlinkButton::HyperlinkButton ()
   : Button (String::empty),
     font (14.0f, Font::underlined),
     resizeFont (true),
     justification (Justification::centred)
{
    setMouseCursor (MouseCursor::PointingHandCursor);
}

HyperlinkButton::~HyperlinkButton()
{
}

//==============================================================================
void HyperlinkButton::setFont (const Font& newFont,
                               const bool resizeToMatchComponentHeight,
                               Justification justificationType)
{
    font = newFont;
    resizeFont = resizeToMatchComponentHeight;
    justification = justificationType;
    repaint();
}

void HyperlinkButton::setURL (const URL& newURL) noexcept
{
    url = newURL;
    setTooltip (newURL.toString (false));
}

Font HyperlinkButton::getFontToUse() const
{
    if (resizeFont)
        return font.withHeight (getHeight() * 0.7f);

    return font;
}

void HyperlinkButton::changeWidthToFitText()
{
    setSize (getFontToUse().getStringWidth (getButtonText()) + 6, getHeight());
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

    g.drawText (getButtonText(), getLocalBounds().reduced (1, 0),
                justification.getOnlyHorizontalFlags() | Justification::verticallyCentred,
                true);
}
