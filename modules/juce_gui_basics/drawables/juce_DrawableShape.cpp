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
