/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

DrawableShape::DrawableShape()
    : strokeType (0.0f),
      mainFill (Colours::black),
      strokeFill (Colours::black)
{
}

DrawableShape::DrawableShape (const DrawableShape& other)
    : Drawable (other),
      strokeType (other.strokeType),
      dashLengths (other.dashLengths),
      mainFill (other.mainFill),
      strokeFill (other.strokeFill)
{
}

DrawableShape::~DrawableShape()
{
}

//==============================================================================
void DrawableShape::setFill (const FillType& newFill)
{
    if (mainFill != newFill)
    {
        mainFill = newFill;
        repaint();
    }
}

void DrawableShape::setStrokeFill (const FillType& newFill)
{
    if (strokeFill != newFill)
    {
        strokeFill = newFill;
        repaint();
    }
}

void DrawableShape::setStrokeType (const PathStrokeType& newStrokeType)
{
    if (strokeType != newStrokeType)
    {
        strokeType = newStrokeType;
        strokeChanged();
    }
}

void DrawableShape::setDashLengths (const Array<float>& newDashLengths)
{
    if (dashLengths != newDashLengths)
    {
        dashLengths = newDashLengths;
        strokeChanged();
    }
}

void DrawableShape::setStrokeThickness (const float newThickness)
{
    setStrokeType (PathStrokeType (newThickness, strokeType.getJointStyle(), strokeType.getEndStyle()));
}

bool DrawableShape::isStrokeVisible() const noexcept
{
    return strokeType.getStrokeThickness() > 0.0f && ! strokeFill.isInvisible();
}

//==============================================================================
void DrawableShape::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);
    applyDrawableClipPath (g);

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
    const float extraAccuracy = 4.0f;

    if (dashLengths.isEmpty())
        strokeType.createStrokedPath (strokePath, path, AffineTransform(), extraAccuracy);
    else
        strokeType.createDashedStroke (strokePath, path, dashLengths.getRawDataPointer(),
                                       dashLengths.size(), AffineTransform(), extraAccuracy);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

Rectangle<float> DrawableShape::getDrawableBounds() const
{
    if (isStrokeVisible())
        return strokePath.getBounds();

    return path.getBounds();
}

bool DrawableShape::hitTest (int x, int y)
{
    bool allowsClicksOnThisComponent, allowsClicksOnChildComponents;
    getInterceptsMouseClicks (allowsClicksOnThisComponent, allowsClicksOnChildComponents);

    if (! allowsClicksOnThisComponent)
        return false;

    auto globalX = (float) (x - originRelativeToComponent.x);
    auto globalY = (float) (y - originRelativeToComponent.y);

    return path.contains (globalX, globalY)
            || (isStrokeVisible() && strokePath.contains (globalX, globalY));
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
