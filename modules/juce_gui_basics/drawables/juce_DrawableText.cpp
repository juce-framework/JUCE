/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

DrawableText::DrawableText()
    : colour (Colours::black),
      justification (Justification::centredLeft)
{
    setBoundingBox (Parallelogram<float> ({ 0.0f, 0.0f, 50.0f, 20.0f }));
    setFont (Font (15.0f), true);
}

DrawableText::DrawableText (const DrawableText& other)
    : Drawable (other),
      bounds (other.bounds),
      fontHeight (other.fontHeight),
      fontHScale (other.fontHScale),
      font (other.font),
      text (other.text),
      colour (other.colour),
      justification (other.justification)
{
    refreshBounds();
}

DrawableText::~DrawableText()
{
}

Drawable* DrawableText::createCopy() const
{
    return new DrawableText (*this);
}

//==============================================================================
void DrawableText::setText (const String& newText)
{
    if (text != newText)
    {
        text = newText;
        refreshBounds();
    }
}

void DrawableText::setColour (Colour newColour)
{
    if (colour != newColour)
    {
        colour = newColour;
        repaint();
    }
}

void DrawableText::setFont (const Font& newFont, bool applySizeAndScale)
{
    if (font != newFont)
    {
        font = newFont;

        if (applySizeAndScale)
        {
            fontHeight = font.getHeight();
            fontHScale = font.getHorizontalScale();
        }

        refreshBounds();
    }
}

void DrawableText::setJustification (Justification newJustification)
{
    justification = newJustification;
    repaint();
}

void DrawableText::setBoundingBox (Parallelogram<float> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        refreshBounds();
    }
}

void DrawableText::setFontHeight (float newHeight)
{
    if (fontHeight != newHeight)
    {
        fontHeight = newHeight;
        refreshBounds();
    }
}

void DrawableText::setFontHorizontalScale (float newScale)
{
    if (fontHScale != newScale)
    {
        fontHScale = newScale;
        refreshBounds();
    }
}

void DrawableText::refreshBounds()
{
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    auto height = jlimit (0.01f, jmax (0.01f, h), fontHeight);
    auto hscale = jlimit (0.01f, jmax (0.01f, w), fontHScale);

    scaledFont = font;
    scaledFont.setHeight (height);
    scaledFont.setHorizontalScale (hscale);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

//==============================================================================
Rectangle<int> DrawableText::getTextArea (float w, float h) const
{
    return Rectangle<float> (w, h).getSmallestIntegerContainer();
}

AffineTransform DrawableText::getTextTransform (float w, float h) const
{
    return AffineTransform::fromTargetPoints (Point<float>(),       bounds.topLeft,
                                              Point<float> (w, 0),  bounds.topRight,
                                              Point<float> (0, h),  bounds.bottomLeft);
}

void DrawableText::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);

    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    g.addTransform (getTextTransform (w, h));
    g.setFont (scaledFont);
    g.setColour (colour);

    g.drawFittedText (text, getTextArea (w, h), justification, 0x100000);
}

Rectangle<float> DrawableText::getDrawableBounds() const
{
    return bounds.getBoundingBox();
}

Path DrawableText::getOutlineAsPath() const
{
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();
    auto area = getTextArea (w, h).toFloat();

    GlyphArrangement arr;
    arr.addFittedText (scaledFont, text,
                       area.getX(), area.getY(),
                       area.getWidth(), area.getHeight(),
                       justification,
                       0x100000);

    Path pathOfAllGlyphs;

    for (auto& glyph : arr)
    {
        Path gylphPath;
        glyph.createPath (gylphPath);
        pathOfAllGlyphs.addPath (gylphPath);
    }

    pathOfAllGlyphs.applyTransform (getTextTransform (w, h).followedBy (getTransform()));

    return pathOfAllGlyphs;
}

} // namespace juce
