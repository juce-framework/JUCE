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

//==============================================================================
/**
    A pair of (x, y) coordinates.

    The ValueType template should be a primitive type such as int, float, double,
    rather than a class.

    @see Line, Path, AffineTransform

    @tags{Graphics}
*/
template <typename ValueType>
class Point
{
public:
    /** Creates a point at the origin */
    constexpr Point() = default;

    /** Creates a copy of another point. */
    constexpr Point (const Point&) = default;

    /** Creates a point from an (x, y) position. */
    constexpr Point (ValueType initialX, ValueType initialY) noexcept : x (initialX), y (initialY) {}

    //==============================================================================
    /** Copies this point from another one. */
    Point& operator= (const Point&) = default;

    constexpr inline bool operator== (Point other) const noexcept      { return x == other.x && y == other.y; }
    constexpr inline bool operator!= (Point other) const noexcept      { return x != other.x || y != other.y; }

    /** Returns true if the point is (0, 0). */
    constexpr bool isOrigin() const noexcept                           { return x == ValueType() && y == ValueType(); }

    /** Returns true if the coordinates are finite values. */
    constexpr inline bool isFinite() const noexcept                    { return juce_isfinite(x) && juce_isfinite(y); }

    /** Returns the point's x coordinate. */
    constexpr inline ValueType getX() const noexcept                   { return x; }

    /** Returns the point's y coordinate. */
    constexpr inline ValueType getY() const noexcept                   { return y; }

    /** Sets the point's x coordinate. */
    inline void setX (ValueType newX) noexcept                         { x = newX; }

    /** Sets the point's y coordinate. */
    inline void setY (ValueType newY) noexcept                         { y = newY; }

    /** Returns a point which has the same Y position as this one, but a new X. */
    constexpr Point withX (ValueType newX) const noexcept              { return Point (newX, y); }

    /** Returns a point which has the same X position as this one, but a new Y. */
    constexpr Point withY (ValueType newY) const noexcept              { return Point (x, newY); }

    /** Changes the point's x and y coordinates. */
    void setXY (ValueType newX, ValueType newY) noexcept               { x = newX; y = newY; }

    /** Adds a pair of coordinates to this value. */
    void addXY (ValueType xToAdd, ValueType yToAdd) noexcept           { x += xToAdd; y += yToAdd; }

    //==============================================================================
    /** Returns a point with a given offset from this one. */
    constexpr Point translated (ValueType deltaX, ValueType deltaY) const noexcept    { return Point (x + deltaX, y + deltaY); }

    /** Adds two points together */
    constexpr Point operator+ (Point other) const noexcept             { return Point (x + other.x, y + other.y); }

    /** Adds another point's coordinates to this one */
    Point& operator+= (Point other) noexcept                           { x += other.x; y += other.y; return *this; }

    /** Subtracts one points from another */
    constexpr Point operator- (Point other) const noexcept             { return Point (x - other.x, y - other.y); }

    /** Subtracts another point's coordinates to this one */
    Point& operator-= (Point other) noexcept                           { x -= other.x; y -= other.y; return *this; }

    /** Multiplies two points together */
    template <typename OtherType>
    constexpr Point operator* (Point<OtherType> other) const noexcept  { return Point ((ValueType) (x * other.x), (ValueType) (y * other.y)); }

    /** Multiplies another point's coordinates to this one */
    template <typename OtherType>
    Point& operator*= (Point<OtherType> other) noexcept                { *this = *this * other; return *this; }

    /** Divides one point by another */
    template <typename OtherType>
    constexpr Point operator/ (Point<OtherType> other) const noexcept  { return Point ((ValueType) (x / other.x), (ValueType) (y / other.y)); }

    /** Divides this point's coordinates by another */
    template <typename OtherType>
    Point& operator/= (Point<OtherType> other) noexcept                { *this = *this / other; return *this; }

    /** Returns a point whose coordinates are multiplied by a given scalar value. */
    template <typename FloatType>
    constexpr Point operator* (FloatType multiplier) const noexcept    { return Point ((ValueType) ((FloatType) x * multiplier), (ValueType) ((FloatType) y * multiplier)); }

    /** Returns a point whose coordinates are divided by a given scalar value. */
    template <typename FloatType>
    constexpr Point operator/ (FloatType divisor) const noexcept       { return Point ((ValueType) ((FloatType) x / divisor), (ValueType) ((FloatType) y / divisor)); }

    /** Multiplies the point's coordinates by a scalar value. */
    template <typename FloatType>
    Point& operator*= (FloatType multiplier) noexcept                  { x = (ValueType) (x * multiplier); y = (ValueType) (y * multiplier); return *this; }

    /** Divides the point's coordinates by a scalar value. */
    template <typename FloatType>
    Point& operator/= (FloatType divisor) noexcept                     { x = (ValueType) (x / divisor); y = (ValueType) (y / divisor); return *this; }

    /** Returns the inverse of this point. */
    constexpr Point operator-() const noexcept                         { return Point (-x, -y); }

    //==============================================================================
    /** This type will be double if the Point's type is double, otherwise it will be float. */
    using FloatType = typename TypeHelpers::SmallestFloatType<ValueType>::type;

    //==============================================================================
    /** Returns the straight-line distance between this point and the origin. */
    ValueType getDistanceFromOrigin() const noexcept                          { return juce_hypot (x, y); }

    /** Returns the straight-line distance between this point and another one. */
    ValueType getDistanceFrom (Point other) const noexcept                    { return juce_hypot (x - other.x, y - other.y); }

    /** Returns the square of the straight-line distance between this point and the origin. */
    constexpr ValueType getDistanceSquaredFromOrigin() const noexcept         { return x * x + y * y; }

    /** Returns the square of the straight-line distance between this point and another one. */
    constexpr ValueType getDistanceSquaredFrom (Point other) const noexcept   { return (*this - other).getDistanceSquaredFromOrigin(); }

    /** Returns the angle from this point to another one.

        Taking this point to be the centre of a circle, and the other point being a position on
        the circumference, the return value is the number of radians clockwise from the 12 o'clock
        direction.
        So 12 o'clock = 0, 3 o'clock = Pi/2, 6 o'clock = Pi, 9 o'clock = -Pi/2
    */
    FloatType getAngleToPoint (Point other) const noexcept
    {
        return static_cast<FloatType> (std::atan2 (static_cast<FloatType> (other.x - x),
                                                   static_cast<FloatType> (y - other.y)));
    }

    /** Returns the point that would be reached by rotating this point clockwise
        about the origin by the specified angle.
    */
    Point rotatedAboutOrigin (ValueType angleRadians) const noexcept
    {
        return Point (x * std::cos (angleRadians) - y * std::sin (angleRadians),
                      x * std::sin (angleRadians) + y * std::cos (angleRadians));
    }

    /** Taking this point to be the centre of a circle, this returns a point on its circumference.
        @param radius   the radius of the circle.
        @param angle    the angle of the point, in radians clockwise from the 12 o'clock position.
    */
    Point<FloatType> getPointOnCircumference (float radius, float angle) const noexcept
    {
        return Point<FloatType> (static_cast<FloatType> (x + radius * std::sin (angle)),
                                 static_cast<FloatType> (y - radius * std::cos (angle)));
    }

    /** Taking this point to be the centre of an ellipse, this returns a point on its circumference.
        @param radiusX  the horizontal radius of the circle.
        @param radiusY  the vertical radius of the circle.
        @param angle    the angle of the point, in radians clockwise from the 12 o'clock position.
    */
    Point<FloatType> getPointOnCircumference (float radiusX, float radiusY, float angle) const noexcept
    {
        return Point<FloatType> (static_cast<FloatType> (x + radiusX * std::sin (angle)),
                                 static_cast<FloatType> (y - radiusY * std::cos (angle)));
    }

    /** Returns the dot-product of two points (x1 * x2 + y1 * y2). */
    constexpr FloatType getDotProduct (Point other) const noexcept     { return x * other.x + y * other.y; }

    //==============================================================================
    /** Uses a transform to change the point's coordinates.
        This will only compile if ValueType = float!

        @see AffineTransform::transformPoint
    */
    void applyTransform (const AffineTransform& transform) noexcept     { transform.transformPoint (x, y); }

    /** Returns the position of this point, if it is transformed by a given AffineTransform. */
    Point transformedBy (const AffineTransform& transform) const noexcept
    {
        return Point (static_cast<ValueType> (transform.mat00 * (float) x + transform.mat01 * (float) y + transform.mat02),
                      static_cast<ValueType> (transform.mat10 * (float) x + transform.mat11 * (float) y + transform.mat12));
    }

    //==============================================================================
    /** Casts this point to a Point<int> object. */
    constexpr Point<int> toInt() const noexcept              { return Point<int> (static_cast<int> (x), static_cast<int> (y)); }

    /** Casts this point to a Point<float> object. */
    constexpr Point<float> toFloat() const noexcept          { return Point<float> (static_cast<float> (x), static_cast<float> (y)); }

    /** Casts this point to a Point<double> object. */
    constexpr Point<double> toDouble() const noexcept        { return Point<double> (static_cast<double> (x), static_cast<double> (y)); }

    /** Casts this point to a Point<int> object using roundToInt() to convert the values. */
    constexpr Point<int> roundToInt() const noexcept         { return Point<int> (juce::roundToInt (x), juce::roundToInt (y)); }

    /** Returns the point as a string in the form "x, y". */
    String toString() const                                       { return String (x) + ", " + String (y); }

    //==============================================================================
    ValueType x{}; /**< The point's X coordinate. */
    ValueType y{}; /**< The point's Y coordinate. */
};

/** Multiplies the point's coordinates by a scalar value. */
template <typename ValueType>
Point<ValueType> operator* (ValueType value, Point<ValueType> p) noexcept       { return p * value; }

} // namespace juce
