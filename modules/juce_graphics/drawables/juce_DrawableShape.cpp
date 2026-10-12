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

void DrawableShape::setFill (const FillType& newFill)
{
    if (std::exchange (mainFill, newFill) != newFill)
        callListeners();
}

void DrawableShape::setStrokeFill (const FillType& newStrokeFill)
{
    if (std::exchange (strokeFill, newStrokeFill) != newStrokeFill)
        strokeChanged();
}

void DrawableShape::setStrokeType (const PathStrokeType& newStrokeType)
{
    const auto newStrokeAttributes = strokeOptions.withWidth (newStrokeType.getStrokeThickness())
                                                  .withLineJoin (newStrokeType.getJointStyle())
                                                  .withLineCap (newStrokeType.getEndStyle())
                                                  .withMiterLimit (newStrokeType.getMiterLimit());

    if (std::exchange (strokeOptions, newStrokeAttributes) != newStrokeAttributes)
        strokeChanged();
}

void DrawableShape::setDashLengths (Span<const float> newDashLengths)
{
    const auto newStrokeAttributes = strokeOptions.withDashArray (newDashLengths);

    if (std::exchange (strokeOptions, newStrokeAttributes) != newStrokeAttributes)
        strokeChanged();
}

void DrawableShape::setDashOffset (float dashOffset)
{
    const auto newStrokeAttributes = strokeOptions.withDashOffset (dashOffset);

    if (std::exchange (strokeOptions, newStrokeAttributes) != newStrokeAttributes)
        strokeChanged();
}

void DrawableShape::setStrokeThickness (const float newThickness)
{
    const auto newStrokeAttributes = strokeOptions.withWidth (newThickness);

    if (std::exchange (strokeOptions, newStrokeAttributes) != newStrokeAttributes)
        strokeChanged();
}

bool DrawableShape::isStrokeVisible() const noexcept
{
    return strokeOptions.createStrokeType().getStrokeThickness() > 0.0f && ! strokeFill.isInvisible();
}

//==============================================================================
void DrawableShape::paint (Graphics& g) const
{
    g.setFillType (mainFill);
    g.fillPath (path);

    if (isStrokeVisible())
    {
        g.setFillType (strokeFill);
        g.fillPath (strokePath);
    }
}

void DrawableShape::pathChanged()
{
    strokeChanged();
}

void DrawableShape::strokeChanged()
{
    strokePath.clear();
    static constexpr float extraAccuracy = 4.0f;

    const auto strokeType = strokeOptions.createStrokeType();

    if (strokeOptions.getDashArray().empty())
        strokeType.createStrokedPath (strokePath, path, AffineTransform(), extraAccuracy);
    else
        strokeType.createDashedStroke (strokePath,
                                       path,
                                       strokeOptions.getDashArray().data(),
                                       (int) strokeOptions.getDashArray().size(),
                                       AffineTransform(),
                                       extraAccuracy,
                                       strokeOptions.getDashOffset());

    callListeners();
}

Rectangle<float> DrawableShape::getDrawableBoundsUntransformed() const
{
    if (isStrokeVisible())
        return strokePath.getBounds();

    return path.getBounds();
}

bool DrawableShape::hitTest (Point<float> p) const
{
    const auto contains = [] (const Path& pathToCheck, Point<float> point, const AffineTransform& t)
    {
        auto transformed = pathToCheck;
        transformed.applyTransform (t);
        return transformed.contains (point);
    };

    return contains (path, p, getDrawableTransform())
           || (isStrokeVisible() && contains (strokePath, p, getDrawableTransform()));
}

//==============================================================================
static bool replaceColourInFill (FillType& fill, Colour original, Colour replacement)
{
    if (fill.colour == original && fill.isColour())
    {
        fill = FillType (replacement);
        return true;
    }

    return false;
}

bool DrawableShape::replaceColour (Colour original, Colour replacement)
{
    bool changed1 = replaceColourInFill (mainFill,   original, replacement);
    bool changed2 = replaceColourInFill (strokeFill, original, replacement);
    return changed1 || changed2;
}

Path DrawableShape::getOutlineAsPath() const
{
    auto outline = isStrokeVisible() ? strokePath : path;
    outline.applyTransform (getTransform());
    return outline;
}

} // namespace juce
