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

DrawableRectangle::DrawableRectangle() {}
DrawableRectangle::~DrawableRectangle() {}

DrawableRectangle::DrawableRectangle (const DrawableRectangle& other)
    : DrawableShape (other),
      bounds (other.bounds),
      cornerSize (other.cornerSize)
{
    rebuildPath();
}

std::unique_ptr<Drawable> DrawableRectangle::createCopy() const
{
    return std::make_unique<DrawableRectangle> (*this);
}

//==============================================================================
void DrawableRectangle::setRectangle (Parallelogram<float> newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        rebuildPath();
    }
}

void DrawableRectangle::setCornerSize (Point<float> newSize)
{
    if (cornerSize != newSize)
    {
        cornerSize = newSize;
        rebuildPath();
    }
}

void DrawableRectangle::rebuildPath()
{
    auto w = bounds.getWidth();
    auto h = bounds.getHeight();

    Path newPath;

    if (cornerSize.x > 0 && cornerSize.y > 0)
        newPath.addRoundedRectangle (0, 0, w, h, cornerSize.x, cornerSize.y);
    else
        newPath.addRectangle (0, 0, w, h);

    newPath.applyTransform (AffineTransform::fromTargetPoints (Point<float>(),       bounds.topLeft,
                                                               Point<float> (w, 0),  bounds.topRight,
                                                               Point<float> (0, h),  bounds.bottomLeft));

    if (path != newPath)
    {
        path.swapWithPath (newPath);
        pathChanged();
    }
}

} // namespace juce
