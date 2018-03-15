/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
    Represents a parallelogram that is defined by 3 points.
    @see Rectangle, Point, Line

    @tags{Graphics}
*/
template <typename ValueType>
class Parallelogram
{
public:
    //==============================================================================
    /** Creates a parallelogram with zero size at the origin.
    */
    Parallelogram() noexcept
    {
    }

    /** Creates a copy of another parallelogram. */
    Parallelogram (const Parallelogram& other) noexcept
       : topLeft (other.topLeft), topRight (other.topRight), bottomLeft (other.bottomLeft)
    {
    }

    /** Creates a parallelogram based on 3 points. */
    Parallelogram (Point<ValueType> topLeftPosition,
                   Point<ValueType> topRightPosition,
                   Point<ValueType> bottomLeftPosition) noexcept
       : topLeft (topLeftPosition), topRight (topRightPosition), bottomLeft (bottomLeftPosition)
    {
    }

    /** Creates a parallelogram from a rectangle. */
    Parallelogram (Rectangle<ValueType> rectangle) noexcept
       : topLeft (rectangle.getTopLeft()),
         topRight (rectangle.getTopRight()),
         bottomLeft (rectangle.getBottomLeft())
    {
    }

    Parallelogram& operator= (const Parallelogram& other) noexcept
    {
        topLeft = other.topLeft;
        topRight = other.topRight;
        bottomLeft = other.bottomLeft;
        return *this;
    }

    /** Destructor. */
    ~Parallelogram() noexcept {}

    //==============================================================================
    /** Returns true if the parallelogram has a width or height of more than zero. */
    bool isEmpty() const noexcept                                   { return topLeft != topRight || topLeft != bottomLeft; }

    /** Returns true if the parallelogram's coordinates are all finite numbers, i.e. not NaN or infinity. */
    inline bool isFinite() const noexcept                           { return topLeft.isFinite() && topRight.isFinite() && bottomLeft.isFinite(); }

    /** Returns the width of the parallelogram (i.e. the straight-line distance between the top-left and top-right. */
    inline ValueType getWidth() const noexcept                      { return Line<ValueType> (topLeft, topRight).getLength(); }

    /** Returns the height of the parallelogram (i.e. the straight-line distance between the top-left and bottom-left. */
    inline ValueType getHeight() const noexcept                     { return Line<ValueType> (topLeft, bottomLeft).getLength(); }

    //==============================================================================
    /** Returns the parallelogram's top-left position as a Point. */
    Point<ValueType> getTopLeft() const noexcept                    { return topLeft; }

    /** Returns the parallelogram's top-right position as a Point. */
    Point<ValueType> getTopRight() const noexcept                   { return topRight; }

    /** Returns the parallelogram's bottom-left position as a Point. */
    Point<ValueType> getBottomLeft() const noexcept                 { return bottomLeft; }

    /** Returns the parallelogram's bottom-right position as a Point. */
    Point<ValueType> getBottomRight() const noexcept                { return topRight + (bottomLeft - topLeft); }

    //==============================================================================
    /** Returns true if the two parallelograms are identical. */
    bool operator== (const Parallelogram& other) const noexcept     { return topLeft == other.topLeft && topRight == other.topRight && bottomLeft == other.bottomLeft; }

    /** Returns true if the two parallelograms are not identical. */
    bool operator!= (const Parallelogram& other) const noexcept     { return ! operator== (other); }

    //==============================================================================
    /** Returns a parallelogram which is the same as this one moved by a given amount. */
    Parallelogram operator+ (Point<ValueType> deltaPosition) const noexcept
    {
        auto p = *this;
        p += deltaPosition;
        return p;
    }

    /** Moves this parallelogram by a given amount. */
    Parallelogram& operator+= (Point<ValueType> deltaPosition) noexcept
    {
        topLeft += deltaPosition;
        topRight += deltaPosition;
        bottomLeft += deltaPosition;
        return *this;
    }

    /** Returns a parallelogram which is the same as this one moved by a given amount. */
    Parallelogram operator- (Point<ValueType> deltaPosition) const noexcept
    {
        return operator+ (-deltaPosition);
    }

    /** Moves this parallelogram by a given amount. */
    Parallelogram& operator-= (Point<ValueType> deltaPosition) noexcept
    {
        return operator-= (-deltaPosition);
    }

    /** Returns a parallelogram that has been scaled by the given amount, centred around the origin. */
    template <typename PointOrScalarType>
    Parallelogram operator* (PointOrScalarType scaleFactor) const noexcept
    {
        auto p = *this;
        p *= scaleFactor;
        return p;
    }

    /** Scales this parallelogram by the given amount, centred around the origin. */
    template <typename PointOrScalarType>
    Parallelogram operator*= (PointOrScalarType scaleFactor) noexcept
    {
        topLeft *= scaleFactor;
        topRight *= scaleFactor;
        bottomLeft *= scaleFactor;
        return *this;
    }

    //==============================================================================
    /** Returns a point within this parallelogram, specified as proportional coordinates.
        The relative X and Y values should be between 0 and 1, where 0 is the left or
        top of this parallelogram, and 1 is the right or bottom. (Out-of-bounds values
        will return a point outside the parallelogram).
    */
    Point<ValueType> getRelativePoint (Point<ValueType> relativePosition) const noexcept
    {
        return topLeft
                + (topRight - topLeft) * relativePosition.x
                + (bottomLeft - topLeft) * relativePosition.y;
    }

    /** Returns a transformed verstion of the parallelogram. */
    Parallelogram transformedBy (const AffineTransform& transform) const noexcept
    {
        auto p = *this;
        transform.transformPoints (p.topLeft, p.topRight, p.bottomLeft);
        return p;
    }

    /** Returns the smallest rectangle that encloses this parallelogram. */
    Rectangle<ValueType> getBoundingBox() const noexcept
    {
        const Point<ValueType> points[] = { topLeft, topRight, bottomLeft, getBottomRight() };
        return Rectangle<ValueType>::findAreaContainingPoints (points, 4);
    }

    Point<ValueType> topLeft, topRight, bottomLeft;
};

} // namespace juce
