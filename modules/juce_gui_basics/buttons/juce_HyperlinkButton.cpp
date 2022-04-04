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

HyperlinkButton::HyperlinkButton()
   : Button (String()),
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
        return font.withHeight ((float) getHeight() * 0.7f);

    return font;
}

void HyperlinkButton::changeWidthToFitText()
{
    setSize (getFontToUse().getStringWidth (getButtonText()) + 6, getHeight());
}

void HyperlinkButton::setJustificationType (Justification newJustification)
{
    if (justification != newJustification)
    {
        justification = newJustification;
        repaint();
    }
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
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    const Colour textColour (findColour (textColourId));

    if (isEnabled())
        g.setColour ((shouldDrawButtonAsHighlighted) ? textColour.darker ((shouldDrawButtonAsDown) ? 1.3f : 0.4f)
                                         : textColour);
    else
        g.setColour (textColour.withMultipliedAlpha (0.4f));

    g.setFont (getFontToUse());

    g.drawText (getButtonText(), getLocalBounds().reduced (1, 0),
                justification.getOnlyHorizontalFlags() | Justification::verticallyCentred,
                true);
}

} // namespace juce
