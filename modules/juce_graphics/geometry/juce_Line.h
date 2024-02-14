/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

//==============================================================================
/**
    Represents a line.

    This class contains a bunch of useful methods for various geometric
    tasks.

    The ValueType template parameter should be a primitive type - float or double
    are what it's designed for. Integer types will work in a basic way, but some methods
    that perform mathematical operations may not compile, or they may not produce
    sensible results.

    @see Point, Rectangle, Path, Graphics::drawLine

    @tags{Graphics}
*/
template <typename ValueType>
class Line
{
public:
    //==============================================================================
    /** Creates a line, using (0, 0) as its start and end points. */
    Line() = default;

    /** Creates a copy of another line. */
    Line (const Line&) = default;

    /** Creates a line based on the coordinates of its start and end points. */
    Line (ValueType startX, ValueType startY, ValueType endX, ValueType endY) noexcept
        : start (startX, startY), end (endX, endY)
    {
    }

    /** Creates a line from its start and end points. */
    Line (Point<ValueType> startPoint, Point<ValueType> endPoint) noexcept
        : start (startPoint), end (endPoint)
    {
    }

    /** Copies a line from another one. */
    Line& operator= (const Line&) = default;

    /** Destructor. */
    ~Line() = default;

    //==============================================================================
    /** Returns the x coordinate of the line's start point. */
    inline ValueType getStartX() const noexcept                             { return start.x; }

    /** Returns the y coordinate of the line's start point. */
    inline ValueType getStartY() const noexcept                             { return start.y; }

    /** Returns the x coordinate of the line's end point. */
    inline ValueType getEndX() const noexcept                               { return end.x; }

    /** Returns the y coordinate of the line's end point. */
    inline ValueType getEndY() const noexcept                               { return end.y; }

    /** Returns the line's start point. */
    inline Point<ValueType> getStart() const noexcept                       { return start; }

    /** Returns the line's end point. */
    inline Point<ValueType> getEnd() const noexcept                         { return end; }

    /** Changes this line's start point */
    void setStart (ValueType newStartX, ValueType newStartY) noexcept       { start.setXY (newStartX, newStartY); }

    /** Changes this line's end point */
    void setEnd (ValueType newEndX, ValueType newEndY) noexcept             { end.setXY (newEndX, newEndY); }

    /** Changes this line's start point */
    void setStart (const Point<ValueType> newStart) noexcept                { start = newStart; }

    /** Changes this line's end point */
    void setEnd (const Point<ValueType> newEnd) noexcept                    { end = newEnd; }

    /** Returns a line that is the same as this one, but with the start and end reversed, */
    Line reversed() const noexcept                                          { return { end, start }; }

    /** Applies an affine transform to the line's start and end points. */
    void applyTransform (const AffineTransform& transform) noexcept
    {
        start.applyTransform (transform);
        end.applyTransform (transform);
    }

    //==============================================================================
    /** Returns the length of the line. */
    ValueType getLength() const noexcept                                    { return start.getDistanceFrom (end); }

    /** Returns the length of the line. */
    ValueType getLengthSquared() const noexcept                             { return start.getDistanceSquaredFrom (end); }

    /** Returns true if the line's start and end x coordinates are the same. */
    bool isVertical() const noexcept                                        { return start.x == end.x; }

    /** Returns true if the line's start and end y coordinates are the same. */
    bool isHorizontal() const noexcept                                      { return start.y == end.y; }

    /** Returns the line's angle.

        This value is the number of radians clockwise from the 12 o'clock direction,
        where the line's start point is considered to be at the centre.
    */
    typename Point<ValueType>::FloatType getAngle() const noexcept          { return start.getAngleToPoint (end); }

    /** Creates a line from a start point, length and angle.

        This angle is the number of radians clockwise from the 12 o'clock direction,
        where the line's start point is considered to be at the centre.
    */
    static Line fromStartAndAngle (Point<ValueType> startPoint, ValueType length, ValueType angle) noexcept
    {
        return { startPoint, startPoint.getPointOnCircumference (length, angle) };
    }

    //==============================================================================
    /** Casts this line to float coordinates. */
    Line<float> toFloat() const noexcept                                    { return { start.toFloat(), end.toFloat() }; }

    /** Casts this line to double coordinates. */
    Line<double> toDouble() const noexcept                                  { return { start.toDouble(), end.toDouble() }; }

    //==============================================================================
    /** Compares two lines. */
    bool operator== (Line other) const noexcept                             { return start == other.start && end == other.end; }

    /** Compares two lines. */
    bool operator!= (Line other) const noexcept                             { return start != other.start || end != other.end; }

    //==============================================================================
    /** Finds the intersection between two lines.

        @param line     the line to intersect with
        @returns        the point at which the lines intersect, even if this lies beyond the end of the lines
    */
    Point<ValueType> getIntersection (Line line) const noexcept
    {
        Point<ValueType> p;
        findIntersection (start, end, line.start, line.end, p);
        return p;
    }

    /** Finds the intersection between two lines.

        @param line             the other line
        @param intersection     the position of the point where the lines meet (or
                                where they would meet if they were infinitely long)
                                the intersection (if the lines intersect). If the lines
                                are parallel, this will just be set to the position
                                of one of the line's endpoints.
        @returns    true if the line segments intersect; false if they don't. Even if they
                    don't intersect, the intersection coordinates returned will still
                    be valid
    */
    bool intersects (Line line, Point<ValueType>& intersection) const noexcept
    {
        return findIntersection (start, end, line.start, line.end, intersection);
    }

    /** Returns true if this line intersects another. */
    bool intersects (Line other) const noexcept
    {
        Point<ValueType> ignored;
        return findIntersection (start, end, other.start, other.end, ignored);
    }

    //==============================================================================
    /** Returns the location of the point which is a given distance along this line.

        @param distanceFromStart    the distance to move along the line from its
                                    start point. This value can be negative or longer
                                    than the line itself
        @see getPointAlongLineProportionally
    */
    Point<ValueType> getPointAlongLine (ValueType distanceFromStart) const noexcept
    {
        const auto length = getLength();
        return approximatelyEqual (length, (ValueType) 0) ? start : start + (end - start) * (distanceFromStart / length);
    }

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
    Point<ValueType> getPointAlongLine (ValueType distanceFromStart,
                                        ValueType perpendicularDistance) const noexcept
    {
        auto delta = end - start;
        auto length = juce_hypot ((double) delta.x,
                                  (double) delta.y);
        if (length <= 0)
            return start;

        return { start.x + static_cast<ValueType> ((delta.x * distanceFromStart - delta.y * perpendicularDistance) / length),
                 start.y + static_cast<ValueType> ((delta.y * distanceFromStart + delta.x * perpendicularDistance) / length) };
    }

    /** Returns the location of the point which is a given distance along this line
        proportional to the line's length.

        @param proportionOfLength   the distance to move along the line from its
                                    start point, in multiples of the line's length.
                                    So a value of 0.0 will return the line's start point
                                    and a value of 1.0 will return its end point. (This value
                                    can be negative or greater than 1.0).
        @see getPointAlongLine
    */
    Point<ValueType> getPointAlongLineProportionally (typename Point<ValueType>::FloatType proportionOfLength) const noexcept
    {
        return start + (end - start) * proportionOfLength;
    }

    /** Returns the smallest distance between this line segment and a given point.

        So if the point is close to the line, this will return the perpendicular
        distance from the line; if the point is a long way beyond one of the line's
        end-point's, it'll return the straight-line distance to the nearest end-point.

        pointOnLine receives the position of the point that is found.

        @returns the point's distance from the line
        @see getPositionAlongLineOfNearestPoint
    */
    ValueType getDistanceFromPoint (Point<ValueType> targetPoint,
                                    Point<ValueType>& pointOnLine) const noexcept
    {
        auto delta = end - start;
        auto length = delta.x * delta.x + delta.y * delta.y;

        if (length > 0)
        {
            auto prop = ((targetPoint.x - start.x) * delta.x
                       + (targetPoint.y - start.y) * delta.y) / (double) length;

            if (prop >= 0 && prop <= 1.0)
            {
                pointOnLine = start + delta * prop;
                return targetPoint.getDistanceFrom (pointOnLine);
            }
        }

        auto fromStart = targetPoint.getDistanceFrom (start);
        auto fromEnd   = targetPoint.getDistanceFrom (end);

        if (fromStart < fromEnd)
        {
            pointOnLine = start;
            return fromStart;
        }

        pointOnLine = end;
        return fromEnd;
    }

    /** Finds the point on this line which is nearest to a given point, and
        returns its position as a proportional position along the line.

        @returns    a value 0 to 1.0 which is the distance along this line from the
                    line's start to the point which is nearest to the point passed-in. To
                    turn this number into a position, use getPointAlongLineProportionally().
        @see getDistanceFromPoint, getPointAlongLineProportionally
    */
    ValueType findNearestProportionalPositionTo (Point<ValueType> point) const noexcept
    {
        auto delta = end - start;
        auto length = delta.x * delta.x + delta.y * delta.y;

        return length <= 0 ? 0
                           : jlimit (ValueType(), static_cast<ValueType> (1),
                                     static_cast<ValueType> ((((point.x - start.x) * delta.x
                                                             + (point.y - start.y) * delta.y) / length)));
    }

    /** Finds the point on this line which is nearest to a given point.
        @see getDistanceFromPoint, findNearestProportionalPositionTo
    */
    Point<ValueType> findNearestPointTo (Point<ValueType> point) const noexcept
    {
        return getPointAlongLineProportionally (findNearestProportionalPositionTo (point));
    }

    /** Returns true if the given point lies above this line.

        The return value is true if the point's y coordinate is less than the y
        coordinate of this line at the given x (assuming the line extends infinitely
        in both directions).
    */
    bool isPointAbove (Point<ValueType> point) const noexcept
    {
        return start.x != end.x
                && point.y < ((end.y - start.y) * (point.x - start.x)) / (end.x - start.x) + start.y;
    }

    /** Returns a lengthened copy of this line.

        This will extend the line by a certain amount by moving the start away from the end
        (leaving the end-point the same), and return the new line.
    */
    Line withLengthenedStart (ValueType distanceToLengthenBy) const noexcept
    {
        return withShortenedStart (-distanceToLengthenBy);
    }

    //==============================================================================
    /** Returns a shortened copy of this line.

        This will chop off part of the start of this line by a certain amount, (leaving the
        end-point the same), and return the new line.
    */
    Line withShortenedStart (ValueType distanceToShortenBy) const noexcept
    {
        return { getPointAlongLine (jmin (distanceToShortenBy, getLength())), end };
    }

    /** Returns a lengthened copy of this line.

        This will extend the line by a certain amount by moving the end away from the start
        (leaving the start-point the same), and return the new line.
    */
    Line withLengthenedEnd (ValueType distanceToLengthenBy) const noexcept
    {
        return withShortenedEnd (-distanceToLengthenBy);
    }

    /** Returns a shortened copy of this line.

        This will chop off part of the end of this line by a certain amount, (leaving the
        start-point the same), and return the new line.
    */
    Line withShortenedEnd (ValueType distanceToShortenBy) const noexcept
    {
        auto length = getLength();
        return { start, getPointAlongLine (length - jmin (distanceToShortenBy, length)) };
    }

private:
    //==============================================================================
    Point<ValueType> start, end;

    static bool isZeroToOne (ValueType v) noexcept      { return v >= 0 && v <= static_cast<ValueType> (1); }

    static bool findIntersection (const Point<ValueType> p1, const Point<ValueType> p2,
                                  const Point<ValueType> p3, const Point<ValueType> p4,
                                  Point<ValueType>& intersection) noexcept
    {
        if (p2 == p3)
        {
            intersection = p2;
            return true;
        }

        auto d1 = p2 - p1;
        auto d2 = p4 - p3;
        auto divisor = d1.x * d2.y - d2.x * d1.y;

        const auto zero = ValueType{};

        if (approximatelyEqual (divisor, zero))
        {
            if (! (d1.isOrigin() || d2.isOrigin()))
            {
                if (approximatelyEqual (d1.y, zero) && ! approximatelyEqual (d2.y, zero))
                {
                    auto along = (p1.y - p3.y) / d2.y;
                    intersection = p1.withX (p3.x + along * d2.x);
                    return isZeroToOne (along);
                }

                if (approximatelyEqual (d2.y, zero) && ! approximatelyEqual (d1.y, zero))
                {
                    auto along = (p3.y - p1.y) / d1.y;
                    intersection = p3.withX (p1.x + along * d1.x);
                    return isZeroToOne (along);
                }

                if (approximatelyEqual (d1.x, zero) && ! approximatelyEqual (d2.x, zero))
                {
                    auto along = (p1.x - p3.x) / d2.x;
                    intersection = p1.withY (p3.y + along * d2.y);
                    return isZeroToOne (along);
                }

                if (approximatelyEqual (d2.x, zero) && ! approximatelyEqual (d1.x, zero))
                {
                    auto along = (p3.x - p1.x) / d1.x;
                    intersection = p3.withY (p1.y + along * d1.y);
                    return isZeroToOne (along);
                }
            }

            intersection = (p2 + p3) / static_cast<ValueType> (2);
            return false;
        }

        auto along1 = ((p1.y - p3.y) * d2.x - (p1.x - p3.x) * d2.y) / divisor;
        intersection = p1 + d1 * along1;

        if (! isZeroToOne (along1))
            return false;

        auto along2 = ((p1.y - p3.y) * d1.x - (p1.x - p3.x) * d1.y) / divisor;
        return isZeroToOne (along2);
    }
};

} // namespace juce
