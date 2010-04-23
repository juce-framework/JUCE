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

    /** Returns the point's x co-ordinate. */
    inline ValueType getX() const throw()                               { return x; }

    /** Returns the point's y co-ordinate. */
    inline ValueType getY() const throw()                               { return y; }

    inline bool operator== (const Point& other) const throw()           { return x == other.x && y == other.y; }
    inline bool operator!= (const Point& other) const throw()           { return x != other.x || y != other.y; }

    /** Returns true if the point is (0, 0). */
    bool isOrigin() const throw()                                       { return x == ValueType() && y == ValueType(); }

    /** Changes the point's x and y co-ordinates. */
    void setXY (const ValueType newX, const ValueType newY) throw()     { x = newX; y = newY; }

    /** Adds a pair of co-ordinates to this value. */
    void addXY (const ValueType xToAdd, const ValueType yToAdd) throw() { x += xToAdd; y += yToAdd; }

    /** Adds two points together. */
    const Point operator+ (const Point& other) const throw()            { return Point (x + other.x, y + other.y); }

    /** Adds another point's co-ordinates to this one. */
    Point& operator+= (const Point& other) throw()                      { x += other.x; y += other.y; return *this; }

    /** Subtracts one points from another. */
    const Point operator- (const Point& other) const throw()            { return Point (x - other.x, y - other.y); }

    /** Subtracts another point's co-ordinates to this one. */
    Point& operator-= (const Point& other) throw()                      { x -= other.x; y -= other.y; return *this; }

    /** Returns the inverse of this point. */
    const Point operator-() const throw()                               { return Point (-x, -y); }

    /** Returns the straight-line distance between this point and another one. */
    ValueType getDistanceFrom (const Point& other) const throw()        { return (ValueType) juce_hypot (x - other.x, y - other.y); }

    /** Uses a transform to change the point's co-ordinates.
        This will only compile if ValueType = float!
        @see AffineTransform::transformPoint
    */
    void applyTransform (const AffineTransform& transform) throw()      { transform.transformPoint (x, y); }

    /** Returns the point as a string in the form "x, y". */
    const String toString() const                                       { return String (x) + ", " + String (y); }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ValueType x, y;
};


#endif   // __JUCE_POINT_JUCEHEADER__
