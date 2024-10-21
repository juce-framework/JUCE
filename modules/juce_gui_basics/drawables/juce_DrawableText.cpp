/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

DrawableText::DrawableText()
    : colour (Colours::black),
      justification (Justification::centredLeft)
{
    setBoundingBox (Parallelogram<float> ({ 0.0f, 0.0f, 50.0f, 20.0f }));
    setFont (withDefaultMetrics (FontOptions (15.0f)), true);
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

std::unique_ptr<Drawable> DrawableText::createCopy() const
{
    return std::make_unique<DrawableText> (*this);
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
    if (! approximatelyEqual (fontHeight, newHeight))
    {
        fontHeight = newHeight;
        refreshBounds();
    }
}

void DrawableText::setFontHorizontalScale (float newScale)
{
    if (! approximatelyEqual (fontHScale, newScale))
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

    pathOfAllGlyphs.applyTransform (getTextTransform (w, h).followedBy (drawableTransform));

    return pathOfAllGlyphs;
}

bool DrawableText::replaceColour (Colour originalColour, Colour replacementColour)
{
    if (colour != originalColour)
        return false;

    setColour (replacementColour);
    return true;
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> DrawableText::createAccessibilityHandler()
{
    class DrawableTextAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        DrawableTextAccessibilityHandler (DrawableText& drawableTextToWrap)
            : AccessibilityHandler (drawableTextToWrap, AccessibilityRole::staticText),
              drawableText (drawableTextToWrap)
        {
        }

        String getTitle() const override  { return drawableText.getText(); }

    private:
        DrawableText& drawableText;
    };

    return std::make_unique<DrawableTextAccessibilityHandler> (*this);
}

} // namespace juce
