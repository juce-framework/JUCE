/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_POINT_JUCEHEADER__
#define __JUCE_POINT_JUCEHEADER__

#include "juce_AffineTransform.h"


//==============================================================================
/**
    A pair of (x, y) co-ordinates.

    The ValueType template should be a primitive type such as int, float, double,
    rather than a class.

    @see Line, Path, AffineTransform
*/
template <typename ValueType>
class Point
{
public:
    //==============================================================================
    /** Creates a point with co-ordinates (0, 0). */
    Point() throw()  : x (0), y (0) {}

    /** Creates a copy of another point. */
    Point (const Point& other) throw()  : x (other.x), y (other.y)  {}

    /** Creates a point from an (x, y) position. */
    Point (const ValueType initialX, const ValueType initialY) throw()  : x (initialX), y (initialY) {}

    /** Destructor. */
    ~Point() throw() {}

    //==============================================================================
    /** Copies this point from another one. */
    Point& operator= (const Point& other) throw()                       { x = other.x; y = other.y; return *this; }

    /** Returns true if the point is (0, 0). */
    bool isOrigin() const throw()                                       { return x == ValueType() && y == ValueType(); }

    /** Returns the point's x co-ordinate. */
    inline ValueType getX() const throw()                               { return x; }

    /** Returns the point's y co-ordinate. */
    inline ValueType getY() const throw()                               { return y; }

    /** Sets the point's x co-ordinate. */
    inline void setX (const ValueType newX) throw()                     { x = newX; }

    /** Sets the point's y co-ordinate. */
    inline void setY (const ValueType newY) throw()                     { y = newY; }

    /** Returns a point which has the same Y position as this one, but a new X. */
    const Point withX (const ValueType newX) const throw()              { return Point (newX, y); }

    /** Returns a point which has the same X position as this one, but a new Y. */
    const Point withY (const ValueType newY) const throw()              { return Point (x, newY); }

    /** Changes the point's x and y co-ordinates. */
    void setXY (const ValueType newX, const ValueType newY) throw()     { x = newX; y = newY; }

    /** Adds a pair of co-ordinates to this value. */
    void addXY (const ValueType xToAdd, const ValueType yToAdd) throw() { x += xToAdd; y += yToAdd; }

    inline bool operator== (const Point& other) const throw()           { return x == other.x && y == other.y; }
    inline bool operator!= (const Point& other) const throw()           { return x != other.x || y != other.y; }

    /** Adds two points together. */
    const Point operator+ (const Point& other) const throw()            { return Point (x + other.x, y + other.y); }

    /** Adds another point's co-ordinates to this one. */
    Point& operator+= (const Point& other) throw()                      { x += other.x; y += other.y; return *this; }

    /** Subtracts one points from another. */
    const Point operator- (const Point& other) const throw()            { return Point (x - other.x, y - other.y); }

    /** Subtracts another point's co-ordinates to this one. */
    Point& operator-= (const Point& other) throw()                      { x -= other.x; y -= other.y; return *this; }

    /** Returns a point whose coordinates are multiplied by a given value. */
    const Point operator* (const ValueType multiplier) const throw()    { return Point (x * multiplier, y * multiplier); }

    /** Multiplies the point's co-ordinates by a value. */
    Point& operator*= (const ValueType multiplier) throw()              { x *= multiplier; y *= multiplier; return *this; }

    /** Returns a point whose coordinates are divided by a given value. */
    const Point operator/ (const ValueType divisor) const throw()       { return Point (x / divisor, y / divisor); }

    /** Divides the point's co-ordinates by a value. */
    Point& operator/= (const ValueType divisor) throw()                 { x /= divisor; y /= divisor; return *this; }

    /** Returns the inverse of this point. */
    const Point operator-() const throw()                               { return Point (-x, -y); }

    /** Returns the straight-line distance between this point and another one. */
    ValueType getDistanceFromOrigin() const throw()                     { return (ValueType) juce_hypot (x, y); }

    /** Returns the straight-line distance between this point and another one. */
    ValueType getDistanceFrom (const Point& other) const throw()        { return (ValueType) juce_hypot (x - other.x, y - other.y); }

    /** Returns the angle from this point to another one.

        The return value is the number of radians clockwise from the 3 o'clock direction,
        where this point is the centre and the other point is on the circumference.
    */
    ValueType getAngleToPoint (const Point& other) const throw()        { return (ValueType) std::atan2 (other.x - x, other.y - y); }

    /** Uses a transform to change the point's co-ordinates.
        This will only compile if ValueType = float!
        @see AffineTransform::transformPoint
    */
    void applyTransform (const AffineTransform& transform) throw()      { transform.transformPoint (x, y); }

    /** Returns the position of this point, if it is transformed by a given AffineTransform. */
    const Point transformedBy (const AffineTransform& transform) const throw()    { ValueType x2 (x), y2 (y); transform.transformPoint (x2, y2); return Point (x2, y2); }

    /** Casts this point to a Point<float> object. */
    const Point<float> toFloat() const throw()                          { return Point<float> (static_cast <float> (x), static_cast<float> (y)); }

    /** Returns the point as a string in the form "x, y". */
    const String toString() const                                       { return String (x) + ", " + String (y); }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ValueType x, y;
};


#endif   // __JUCE_POINT_JUCEHEADER__
