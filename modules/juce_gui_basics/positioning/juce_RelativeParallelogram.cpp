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

RelativeParallelogram::RelativeParallelogram()
{
}

RelativeParallelogram::RelativeParallelogram (const Rectangle<float>& r)
    : topLeft (r.getTopLeft()), topRight (r.getTopRight()), bottomLeft (r.getBottomLeft())
{
}

RelativeParallelogram::RelativeParallelogram (const RelativePoint& topLeft_, const RelativePoint& topRight_, const RelativePoint& bottomLeft_)
    : topLeft (topLeft_), topRight (topRight_), bottomLeft (bottomLeft_)
{
}

RelativeParallelogram::RelativeParallelogram (const String& topLeft_, const String& topRight_, const String& bottomLeft_)
    : topLeft (topLeft_), topRight (topRight_), bottomLeft (bottomLeft_)
{
}

RelativeParallelogram::~RelativeParallelogram()
{
}

void RelativeParallelogram::resolveThreePoints (Point<float>* points, Expression::Scope* const scope) const
{
    points[0] = topLeft.resolve (scope);
    points[1] = topRight.resolve (scope);
    points[2] = bottomLeft.resolve (scope);
}

void RelativeParallelogram::resolveFourCorners (Point<float>* points, Expression::Scope* const scope) const
{
    resolveThreePoints (points, scope);
    points[3] = points[1] + (points[2] - points[0]);
}

const Rectangle<float> RelativeParallelogram::getBounds (Expression::Scope* const scope) const
{
    Point<float> points[4];
    resolveFourCorners (points, scope);
    return Rectangle<float>::findAreaContainingPoints (points, 4);
}

void RelativeParallelogram::getPath (Path& path, Expression::Scope* const scope) const
{
    Point<float> points[4];
    resolveFourCorners (points, scope);

    path.startNewSubPath (points[0]);
    path.lineTo (points[1]);
    path.lineTo (points[3]);
    path.lineTo (points[2]);
    path.closeSubPath();
}

AffineTransform RelativeParallelogram::resetToPerpendicular (Expression::Scope* const scope)
{
    Point<float> corners[3];
    resolveThreePoints (corners, scope);

    const Line<float> top (corners[0], corners[1]);
    const Line<float> left (corners[0], corners[2]);
    const Point<float> newTopRight (corners[0] + Point<float> (top.getLength(), 0.0f));
    const Point<float> newBottomLeft (corners[0] + Point<float> (0.0f, left.getLength()));

    topRight.moveToAbsolute (newTopRight, scope);
    bottomLeft.moveToAbsolute (newBottomLeft, scope);

    return AffineTransform::fromTargetPoints (corners[0], corners[0],
                                              corners[1], newTopRight,
                                              corners[2], newBottomLeft);
}

bool RelativeParallelogram::isDynamic() const
{
    return topLeft.isDynamic() || topRight.isDynamic() || bottomLeft.isDynamic();
}

bool RelativeParallelogram::operator== (const RelativeParallelogram& other) const noexcept
{
    return topLeft == other.topLeft && topRight == other.topRight && bottomLeft == other.bottomLeft;
}

bool RelativeParallelogram::operator!= (const RelativeParallelogram& other) const noexcept
{
    return ! operator== (other);
}

Point<float> RelativeParallelogram::getInternalCoordForPoint (const Point<float>* const corners, Point<float> target) noexcept
{
    const Point<float> tr (corners[1] - corners[0]);
    const Point<float> bl (corners[2] - corners[0]);
    target -= corners[0];

    return Point<float> (Line<float> (Point<float>(), tr).getIntersection (Line<float> (target, target - bl)).getDistanceFromOrigin(),
                         Line<float> (Point<float>(), bl).getIntersection (Line<float> (target, target - tr)).getDistanceFromOrigin());
}

Point<float> RelativeParallelogram::getPointForInternalCoord (const Point<float>* const corners, const Point<float> point) noexcept
{
    return corners[0]
            + Line<float> (Point<float>(), corners[1] - corners[0]).getPointAlongLine (point.x)
            + Line<float> (Point<float>(), corners[2] - corners[0]).getPointAlongLine (point.y);
}

Rectangle<float> RelativeParallelogram::getBoundingBox (const Point<float>* const p) noexcept
{
    const Point<float> points[] = { p[0], p[1], p[2], p[1] + (p[2] - p[0]) };
    return Rectangle<float>::findAreaContainingPoints (points, 4);
}

} // namespace juce
