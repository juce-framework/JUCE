/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

    return AffineTransform::fromTargetPoints (corners[0].x, corners[0].y, corners[0].x, corners[0].y,
                                              corners[1].x, corners[1].y, newTopRight.x, newTopRight.y,
                                              corners[2].x, corners[2].y, newBottomLeft.x, newBottomLeft.y);
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
