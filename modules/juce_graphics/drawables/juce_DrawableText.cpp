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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

std::unique_ptr<Drawable> DrawableText::createCopy() const
{
    return std::make_unique<DrawableText> (*this);
}

//==============================================================================
void DrawableText::setText (const String& newText)
{
    setMember (&DrawableText::text, newText);
}

void DrawableText::setPreserveWhitespace (bool shouldPreserveWhitespace)
{
    setMember (&DrawableText::preserveWhitespace, shouldPreserveWhitespace);
}

void DrawableText::setColour (Colour newColour)
{
    setFill (FillType { newColour });
}

void DrawableText::setFill (const FillType& newFill)
{
    setMember (&DrawableText::fill, newFill);
}

void DrawableText::setStrokeOptions (const StrokeOptions& newStrokeOptions)
{
    if (std::exchange (strokeOptions, newStrokeOptions) != newStrokeOptions)
        update();
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

        update();
    }
}

void DrawableText::setJustification (Justification newJustification)
{
    setMember (&DrawableText::justification, newJustification);
}

void DrawableText::setBoundingBox (Parallelogram<float> newBounds)
{
    setMember (&DrawableText::bounds, newBounds);
}

bool DrawableText::hitTest (Point<float> p) const
{
    const auto t = getDrawableTransform();
    Path path;
    path.addRectangle (bounds.getBoundingBox());
    path.applyTransform (t);
    return path.contains (p);
}

void DrawableText::setFontHeight (float newHeight)
{
    if (! approximatelyEqual (fontHeight, newHeight))
    {
        fontHeight = newHeight;
        update();
    }
}

void DrawableText::setFontHorizontalScale (float newScale)
{
    if (! approximatelyEqual (fontHScale, newScale))
    {
        fontHScale = newScale;
        update();
    }
}

void DrawableText::update()
{
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    auto height = jlimit (0.01f, jmax (0.01f, h), fontHeight);
    auto hscale = jlimit (0.01f, jmax (0.01f, w), fontHScale);

    scaledFont = font;
    scaledFont.setHeight (height);
    scaledFont.setHorizontalScale (hscale);

    if (strokeOptions.getWidth() > 0.0f)
    {
        const auto strokeType = strokeOptions.createStrokeType();
        const auto path = getOutlineAsPath();
        static constexpr float extraAccuracy = 4.0f;

        if (strokeOptions.getDashArray().empty())
        {
            strokeType.createStrokedPath (strokePath, path, getTransform().inverted(), extraAccuracy);
        }
        else
        {
            strokeType.createDashedStroke (strokePath,
                                           path,
                                           strokeOptions.getDashArray().data(),
                                           (int) strokeOptions.getDashArray().size(),
                                           getTransform().inverted(),
                                           extraAccuracy,
                                           strokeOptions.getDashOffset());
        }
    }

    callListeners();
}

//==============================================================================
Rectangle<float> DrawableText::getTextArea() const
{
    return { bounds.getWidth(), bounds.getHeight() };
}

AffineTransform DrawableText::getTextTransform() const
{
    const auto transformBounds = bounds - bounds.getTopLeft();

    return AffineTransform::fromTargetPoints (Point<float>(),       transformBounds.topLeft,
                                              Point<float> (bounds.getWidth(), 0),  transformBounds.topRight,
                                              Point<float> (0, bounds.getHeight()),  transformBounds.bottomLeft);
}

void DrawableText::paint (Graphics& g) const
{
    const Graphics::ScopedSaveState ss (g);

    g.setFont (scaledFont);
    g.setFillType (fill);

    if (preserveWhitespace)
    {
        g.drawText (text, getBoundingRectangle(), justification, false);
    }
    else
    {
        g.drawFittedText (text,
                          getBoundingRectangle().toNearestInt(),
                          justification,
                          0x100000);
    }

    if (strokeOptions.getWidth() > 0.0f)
    {
        g.setColour (strokeOptions.getColour());
        g.fillPath (strokePath);
    }
}

AffineTransform DrawableText::getTransform() const
{
    return getTextTransform().followedBy (getDrawableTransform());
}

Rectangle<float> DrawableText::getDrawableBoundsUntransformed() const
{
    return bounds.getBoundingBox();
}

Path DrawableText::getOutlineAsPath() const
{
    auto area = getBoundingRectangle();

    GlyphArrangement arr;

    if (preserveWhitespace)
    {
        arr.addJustifiedText (scaledFont,
                              text,
                              area.getX(),
                              area.getY() + scaledFont.getAscent(),
                              area.getWidth(),
                              justification);
    }
    else
    {
        arr.addFittedText (scaledFont,
                           text,
                           area.getX(),
                           area.getY(),
                           area.getWidth(),
                           area.getHeight(),
                           justification,
                           0x100000);
    }

    Path pathOfAllGlyphs;

    for (auto& glyph : arr)
    {
        Path gylphPath;
        glyph.createPath (gylphPath);
        pathOfAllGlyphs.addPath (gylphPath);
    }

    pathOfAllGlyphs.applyTransform (getTransform());

    return pathOfAllGlyphs;
}

bool DrawableText::replaceColour (Colour originalColour, Colour replacementColour)
{
    if (colour != originalColour)
        return false;

    setColour (replacementColour);
    return true;
}

std::optional<String> DrawableText::getContainedText() const
{
    return getText();
}

template <typename Object, typename Member, typename Other>
void DrawableText::setMember (Member Object::* member, const Other& value)
{
    if (std::exchange (this->*member, value) != value)
        update();
}

Rectangle<float> DrawableText::getBoundingRectangle() const
{
    const auto boundingRectangle = bounds.getBoundingBox();

    if (preserveWhitespace)
        return boundingRectangle;

    // Supporting legacy behaviour
    return boundingRectangle.getSmallestIntegerContainer().toFloat();
}

} // namespace juce
