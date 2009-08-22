/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_Line.h"
#include "juce_PathIterator.h"


//==============================================================================
static bool juce_lineIntersection (const float x1, const float y1,
                                   const float x2, const float y2,
                                   const float x3, const float y3,
                                   const float x4, const float y4,
                                   float& intersectionX,
                                   float& intersectionY) throw()
{
    if (x2 != x3 || y2 != y3)
    {
        const float dx1 = x2 - x1;
        const float dy1 = y2 - y1;
        const float dx2 = x4 - x3;
        const float dy2 = y4 - y3;
        const float divisor = dx1 * dy2 - dx2 * dy1;

        if (divisor == 0)
        {
            if (! ((dx1 == 0 && dy1 == 0) || (dx2 == 0 && dy2 == 0)))
            {
                if (dy1 == 0 && dy2 != 0)
                {
                    const float along = (y1 - y3) / dy2;
                    intersectionX = x3 + along * dx2;
                    intersectionY = y1;

                    return along >= 0 && along <= 1.0f;
                }
                else if (dy2 == 0 && dy1 != 0)
                {
                    const float along = (y3 - y1) / dy1;
                    intersectionX = x1 + along * dx1;
                    intersectionY = y3;

                    return along >= 0 && along <= 1.0f;
                }
                else if (dx1 == 0 && dx2 != 0)
                {
                    const float along = (x1 - x3) / dx2;
                    intersectionX = x1;
                    intersectionY = y3 + along * dy2;

                    return along >= 0 && along <= 1.0f;
                }
                else if (dx2 == 0 && dx1 != 0)
                {
                    const float along = (x3 - x1) / dx1;
                    intersectionX = x3;
                    intersectionY = y1 + along * dy1;

                    return along >= 0 && along <= 1.0f;
                }
            }

            intersectionX = 0.5f * (x2 + x3);
            intersectionY = 0.5f * (y2 + y3);

            return false;
        }

        const float along1 = ((y1 - y3) * dx2 - (x1 - x3) * dy2) / divisor;

        intersectionX = x1 + along1 * dx1;
        intersectionY = y1 + along1 * dy1;

        if (along1 < 0 || along1 > 1.0f)
            return false;

        const float along2 = ((y1 - y3) * dx1 - (x1 - x3) * dy1) / divisor;

        return along2 >= 0 && along2 <= 1.0f;
    }

    intersectionX = x2;
    intersectionY = y2;
    return true;
}

//==============================================================================
Line::Line() throw()
    : startX (0.0f),
      startY (0.0f),
      endX (0.0f),
      endY (0.0f)
{
}

Line::Line (const Line& other) throw()
    : startX (other.startX),
      startY (other.startY),
      endX (other.endX),
      endY (other.endY)
{
}

Line::Line (const float startX_, const float startY_,
            const float endX_, const float endY_) throw()
    : startX (startX_),
      startY (startY_),
      endX (endX_),
      endY (endY_)
{
}

Line::Line (const Point& start,
            const Point& end) throw()
    : startX (start.getX()),
      startY (start.getY()),
      endX (end.getX()),
      endY (end.getY())
{
}

const Line& Line::operator= (const Line& other) throw()
{
    startX = other.startX;
    startY = other.startY;
    endX = other.endX;
    endY = other.endY;

    return *this;
}

Line::~Line() throw()
{
}

//==============================================================================
const Point Line::getStart() const throw()
{
    return Point (startX, startY);
}

const Point Line::getEnd() const throw()
{
    return Point (endX, endY);
}

void Line::setStart (const float newStartX,
                     const float newStartY) throw()
{
    startX = newStartX;
    startY = newStartY;
}

void Line::setStart (const Point& newStart) throw()
{
    startX = newStart.getX();
    startY = newStart.getY();
}

void Line::setEnd (const float newEndX,
                   const float newEndY) throw()
{
    endX = newEndX;
    endY = newEndY;
}

void Line::setEnd (const Point& newEnd) throw()
{
    endX = newEnd.getX();
    endY = newEnd.getY();
}

bool Line::operator== (const Line& other) const throw()
{
    return startX == other.startX
            && startY == other.startY
            && endX == other.endX
            && endY == other.endY;
}

bool Line::operator!= (const Line& other) const throw()
{
    return startX != other.startX
            || startY != other.startY
            || endX != other.endX
            || endY != other.endY;
}

//==============================================================================
void Line::applyTransform (const AffineTransform& transform) throw()
{
    transform.transformPoint (startX, startY);
    transform.transformPoint (endX, endY);
}

//==============================================================================
float Line::getLength() const throw()
{
    return (float) juce_hypot (startX - endX,
                               startY - endY);
}

float Line::getAngle() const throw()
{
    return atan2f (endX - startX,
                   endY - startY);
}

const Point Line::getPointAlongLine (const float distanceFromStart) const throw()
{
    const float alpha = distanceFromStart / getLength();

    return Point (startX + (endX - startX) * alpha,
                  startY + (endY - startY) * alpha);
}

const Point Line::getPointAlongLine (const float offsetX,
                                     const float offsetY) const throw()
{
    const float dx = endX - startX;
    const float dy = endY - startY;
    const double length = juce_hypot (dx, dy);

    if (length == 0)
        return Point (startX, startY);
    else
        return Point (startX + (float) (((dx * offsetX) - (dy * offsetY)) / length),
                      startY + (float) (((dy * offsetX) + (dx * offsetY)) / length));
}

const Point Line::getPointAlongLineProportionally (const float alpha) const throw()
{
    return Point (startX + (endX - startX) * alpha,
                  startY + (endY - startY) * alpha);
}

float Line::getDistanceFromLine (const float x,
                                 const float y) const throw()
{
    const double dx = endX - startX;
    const double dy = endY - startY;
    const double length = dx * dx + dy * dy;

    if (length > 0)
    {
        const double prop = ((x - startX) * dx + (y - startY) * dy) / length;

        if (prop >= 0.0f && prop < 1.0f)
        {
            return (float) juce_hypot (x - (startX + prop * dx),
                                       y - (startY + prop * dy));
        }
    }

    return (float) jmin (juce_hypot (x - startX, y - startY),
                         juce_hypot (x - endX, y - endY));
}

float Line::findNearestPointTo (const float x,
                                const float y) const throw()
{
    const double dx = endX - startX;
    const double dy = endY - startY;
    const double length = dx * dx + dy * dy;

    if (length <= 0.0)
        return 0.0f;

    return jlimit (0.0f, 1.0f,
                   (float) (((x - startX) * dx + (y - startY) * dy) / length));
}

const Line Line::withShortenedStart (const float distanceToShortenBy) const throw()
{
    const float length = getLength();

    return Line (getPointAlongLine (jmin (distanceToShortenBy, length)),
                 getEnd());
}

const Line Line::withShortenedEnd (const float distanceToShortenBy) const throw()
{
    const float length = getLength();

    return Line (getStart(),
                 getPointAlongLine (length - jmin (distanceToShortenBy, length)));
}

//==============================================================================
bool Line::clipToPath (const Path& path,
                       const bool keepSectionOutsidePath) throw()
{
    const bool startInside = path.contains (startX, startY);
    const bool endInside = path.contains (endX, endY);

    if (startInside == endInside)
    {
        if (keepSectionOutsidePath != startInside)
        {
            // entirely outside the path
            return false;
        }
        else
        {
            // entirely inside the path
            startX = 0.0f;
            startY = 0.0f;
            endX = 0.0f;
            endY = 0.0f;

            return true;
        }
    }
    else
    {
        bool changed = false;
        PathFlatteningIterator iter (path, AffineTransform::identity);

        while (iter.next())
        {
            float ix, iy;

            if (intersects (Line (iter.x1, iter.y1,
                                  iter.x2, iter.y2),
                            ix, iy))
            {
                if ((startInside && keepSectionOutsidePath)
                     || (endInside && ! keepSectionOutsidePath))
                {
                    setStart (ix, iy);
                }
                else
                {
                    setEnd (ix, iy);
                }

                changed = true;
            }
        }

        return changed;
    }
}

//==============================================================================
bool Line::intersects (const Line& line,
                       float& intersectionX,
                       float& intersectionY) const throw()
{
    return juce_lineIntersection (startX, startY,
                                  endX, endY,
                                  line.startX, line.startY,
                                  line.endX, line.endY,
                                  intersectionX,
                                  intersectionY);
}

bool Line::isVertical() const throw()
{
    return startX == endX;
}

bool Line::isHorizontal() const throw()
{
    return startY == endY;
}

bool Line::isPointAbove (const float x, const float y) const throw()
{
    return startX != endX
            && y < ((endY - startY) * (x - startX)) / (endX - startX) + startY;
}


END_JUCE_NAMESPACE
