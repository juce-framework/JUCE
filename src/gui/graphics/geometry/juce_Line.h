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

#ifndef __JUCE_LINE_JUCEHEADER__
#define __JUCE_LINE_JUCEHEADER__

#include "juce_Path.h"
#include "juce_Point.h"


//==============================================================================
/**
    Represents a line, using 32-bit float co-ordinates.

    This class contains a bunch of useful methods for various geometric
    tasks.

    @see Point, Rectangle, Path, Graphics::drawLine
*/
class JUCE_API  Line
{
public:
    //==============================================================================
    /** Creates a line, using (0, 0) as its start and end points. */
    Line() throw();

    /** Creates a copy of another line. */
    Line (const Line& other) throw();

    /** Creates a line based on the co-ordinates of its start and end points. */
    Line (const float startX,
          const float startY,
          const float endX,
          const float endY) throw();

    /** Creates a line from its start and end points. */
    Line (const Point& start,
          const Point& end) throw();

    /** Copies a line from another one. */
    const Line& operator= (const Line& other) throw();

    /** Destructor. */
    ~Line() throw();

    //==============================================================================
    /** Returns the x co-ordinate of the line's start point. */
    inline float getStartX() const throw()                              { return startX; }

    /** Returns the y co-ordinate of the line's start point. */
    inline float getStartY() const throw()                              { return startY; }

    /** Returns the x co-ordinate of the line's end point. */
    inline float getEndX() const throw()                                { return endX; }

    /** Returns the y co-ordinate of the line's end point. */
    inline float getEndY() const throw()                                { return endY; }

    /** Returns the line's start point. */
    const Point getStart() const throw();

    /** Returns the line's end point. */
    const Point getEnd() const throw();

    /** Changes this line's start point */
    void setStart (const float newStartX,
                   const float newStartY) throw();

    /** Changes this line's end point */
    void setEnd (const float newEndX,
                 const float newEndY) throw();

    /** Changes this line's start point */
    void setStart (const Point& newStart) throw();

    /** Changes this line's end point */
    void setEnd (const Point& newEnd) throw();

    /** Applies an affine transform to the line's start and end points. */
    void applyTransform (const AffineTransform& transform) throw();

    //==============================================================================
    /** Returns the length of the line. */
    float getLength() const throw();

    /** Returns true if the line's start and end x co-ordinates are the same. */
    bool isVertical() const throw();

    /** Returns true if the line's start and end y co-ordinates are the same. */
    bool isHorizontal() const throw();

    /** Returns the line's angle.

        This value is the number of radians clockwise from the 3 o'clock direction,
        where the line's start point is considered to be at the centre.
    */
    float getAngle() const throw();

    //==============================================================================
    /** Compares two lines. */
    bool operator== (const Line& other) const throw();

    /** Compares two lines. */
    bool operator!= (const Line& other) const throw();

    //==============================================================================
    /** Finds the intersection between two lines.

        @param line             the other line
        @param intersectionX    the x co-ordinate of the point where the lines meet (or
                                where they would meet if they were infinitely long)
                                the intersection (if the lines intersect). If the lines
                                are parallel, this will just be set to the position
                                of one of the line's endpoints.
        @param intersectionY    the y co-ordinate of the point where the lines meet
        @returns    true if the line segments intersect; false if they dont. Even if they
                    don't intersect, the intersection co-ordinates returned will still
                    be valid
    */
    bool intersects (const Line& line,
                     float& intersectionX,
                     float& intersectionY) const throw();

    //==============================================================================
    /** Returns the location of the point which is a given distance along this line.

        @param distanceFromStart    the distance to move along the line from its
                                    start point. This value can be negative or longer
                                    than the line itself
        @see getPointAlongLineProportionally
    */
    const Point getPointAlongLine (const float distanceFromStart) const throw();

    /** Returns a point which is a certain distance along and to the side of this line.

        This effectively moves a given distance along the line, then another distance
        perpendicularly to this, and returns the resulting position.

        @param distanceFromStart    the distance to move along the line from its
                                    start point. This value can be negative or longer
                                    than the line itself
        @param perpendicularDistance    how far to move sideways from the line. If you're
                                    looking along the line from its start towards its
                                    end, then a positive value here will move to the
                                    right, negative value move to the left.
    */
    const Point getPointAlongLine (const float distanceFromStart,
                                   const float perpendicularDistance) const throw();

    /** Returns the location of the point which is a given distance along this line
        proportional to the line's length.

        @param proportionOfLength   the distance to move along the line from its
                                    start point, in multiples of the line's length.
                                    So a value of 0.0 will return the line's start point
                                    and a value of 1.0 will return its end point. (This value
                                    can be negative or greater than 1.0).
        @see getPointAlongLine
    */
    const Point getPointAlongLineProportionally (const float proportionOfLength) const throw();

    /** Returns the smallest distance between this line segment and a given point.

        So if the point is close to the line, this will return the perpendicular
        distance from the line; if the point is a long way beyond one of the line's
        end-point's, it'll return the straight-line distance to the nearest end-point.

        @param x    x position of the point to test
        @param y    y position of the point to test
        @returns the point's distance from the line
        @see getPositionAlongLineOfNearestPoint
    */
    float getDistanceFromLine (const float x,
                               const float y) const throw();

    /** Finds the point on this line which is nearest to a given point, and
        returns its position as a proportional position along the line.

        @param x    x position of the point to test
        @param y    y position of the point to test
        @returns    a value 0 to 1.0 which is the distance along this line from the
                    line's start to the point which is nearest to the point passed-in. To
                    turn this number into a position, use getPointAlongLineProportionally().
        @see getDistanceFromLine, getPointAlongLineProportionally
    */
    float findNearestPointTo (const float x,
                              const float y) const throw();

    /** Returns true if the given point lies above this line.

        The return value is true if the point's y coordinate is less than the y
        coordinate of this line at the given x (assuming the line extends infinitely
        in both directions).
    */
    bool isPointAbove (const float x, const float y) const throw();

    //==============================================================================
    /** Returns a shortened copy of this line.

        This will chop off part of the start of this line by a certain amount, (leaving the
        end-point the same), and return the new line.
    */
    const Line withShortenedStart (const float distanceToShortenBy) const throw();

    /** Returns a shortened copy of this line.

        This will chop off part of the end of this line by a certain amount, (leaving the
        start-point the same), and return the new line.
    */
    const Line withShortenedEnd (const float distanceToShortenBy) const throw();

    /** Cuts off parts of this line to keep the parts that are either inside or
        outside a path.

        Note that this isn't smart enough to cope with situations where the
        line would need to be cut into multiple pieces to correctly clip against
        a re-entrant shape.

        @param path                     the path to clip against
        @param keepSectionOutsidePath   if true, it's the section outside the path
                                        that will be kept; if false its the section inside
                                        the path
        @returns true if the line was changed.
    */
    bool clipToPath (const Path& path,
                     const bool keepSectionOutsidePath) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    float startX, startY, endX, endY;
};


#endif   // __JUCE_LINE_JUCEHEADER__
