/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_RECTANGLE_JUCEHEADER__
#define __JUCE_RECTANGLE_JUCEHEADER__

#include "juce_Point.h"
class RectangleList;


//==============================================================================
/**
    Manages a rectangle and allows geometric operations to be performed on it.

    @see RectangleList, Path, Line, Point
*/
template <typename ValueType>
class Rectangle
{
public:
    //==============================================================================
    /** Creates a rectangle of zero size.
        The default co-ordinates will be (0, 0, 0, 0).
    */
    Rectangle() noexcept
      : w(), h()
    {
    }

    /** Creates a copy of another rectangle. */
    Rectangle (const Rectangle& other) noexcept
      : pos (other.pos), w (other.w), h (other.h)
    {
    }

    /** Creates a rectangle with a given position and size. */
    Rectangle (const ValueType initialX, const ValueType initialY,
               const ValueType width, const ValueType height) noexcept
      : pos (initialX, initialY),
        w (width), h (height)
    {
    }

    /** Creates a rectangle with a given size, and a position of (0, 0). */
    Rectangle (const ValueType width, const ValueType height) noexcept
      : w (width), h (height)
    {
    }

    /** Creates a Rectangle from the positions of two opposite corners. */
    Rectangle (const Point<ValueType>& corner1, const Point<ValueType>& corner2) noexcept
      : pos (jmin (corner1.x, corner2.x),
             jmin (corner1.y, corner2.y)),
        w (corner1.x - corner2.x),
        h (corner1.y - corner2.y)
    {
        if (w < ValueType()) w = -w;
        if (h < ValueType()) h = -h;
    }

    /** Creates a Rectangle from a set of left, right, top, bottom coordinates.
        The right and bottom values must be larger than the left and top ones, or the resulting
        rectangle will have a negative size.
    */
    static Rectangle leftTopRightBottom (const ValueType left, const ValueType top,
                                         const ValueType right, const ValueType bottom) noexcept
    {
        return Rectangle (left, top, right - left, bottom - top);
    }

    Rectangle& operator= (const Rectangle& other) noexcept
    {
        pos = other.pos;
        w = other.w; h = other.h;
        return *this;
    }

    /** Destructor. */
    ~Rectangle() noexcept {}

    //==============================================================================
    /** Returns true if the rectangle's width and height are both zero or less */
    bool isEmpty() const noexcept                                   { return w <= ValueType() || h <= ValueType(); }

    /** Returns the x co-ordinate of the rectangle's left-hand-side. */
    inline ValueType getX() const noexcept                          { return pos.x; }

    /** Returns the y co-ordinate of the rectangle's top edge. */
    inline ValueType getY() const noexcept                          { return pos.y; }

    /** Returns the width of the rectangle. */
    inline ValueType getWidth() const noexcept                      { return w; }

    /** Returns the height of the rectangle. */
    inline ValueType getHeight() const noexcept                     { return h; }

    /** Returns the x co-ordinate of the rectangle's right-hand-side. */
    inline ValueType getRight() const noexcept                      { return pos.x + w; }

    /** Returns the y co-ordinate of the rectangle's bottom edge. */
    inline ValueType getBottom() const noexcept                     { return pos.y + h; }

    /** Returns the x co-ordinate of the rectangle's centre. */
    ValueType getCentreX() const noexcept                           { return pos.x + w / (ValueType) 2; }

    /** Returns the y co-ordinate of the rectangle's centre. */
    ValueType getCentreY() const noexcept                           { return pos.y + h / (ValueType) 2; }

    /** Returns the centre point of the rectangle. */
    Point<ValueType> getCentre() const noexcept                     { return Point<ValueType> (pos.x + w / (ValueType) 2,
                                                                                               pos.y + h / (ValueType) 2); }

    /** Returns the aspect ratio of the rectangle's width / height.
        If widthOverHeight is true, it returns width / height; if widthOverHeight is false,
        it returns height / width. */
    ValueType getAspectRatio (const bool widthOverHeight = true) const noexcept                     { return widthOverHeight ? w / h : h / w; }

    //==============================================================================
    /** Returns the rectangle's top-left position as a Point. */
    inline const Point<ValueType>& getPosition() const noexcept                                     { return pos; }

    /** Changes the position of the rectangle's top-left corner (leaving its size unchanged). */
    inline void setPosition (const Point<ValueType>& newPos) noexcept                               { pos = newPos; }

    /** Changes the position of the rectangle's top-left corner (leaving its size unchanged). */
    inline void setPosition (const ValueType newX, const ValueType newY) noexcept                   { pos.setXY (newX, newY); }

    /** Returns the rectangle's top-left position as a Point. */
    const Point<ValueType>& getTopLeft() const noexcept                                             { return pos; }

    /** Returns the rectangle's top-right position as a Point. */
    Point<ValueType> getTopRight() const noexcept                                                   { return Point<ValueType> (pos.x + w, pos.y); }

    /** Returns the rectangle's bottom-left position as a Point. */
    Point<ValueType> getBottomLeft() const noexcept                                                 { return Point<ValueType> (pos.x, pos.y + h); }

    /** Returns the rectangle's bottom-right position as a Point. */
    Point<ValueType> getBottomRight() const noexcept                                                { return Point<ValueType> (pos.x + w, pos.y + h); }

    /** Changes the rectangle's size, leaving the position of its top-left corner unchanged. */
    void setSize (const ValueType newWidth, const ValueType newHeight) noexcept                     { w = newWidth; h = newHeight; }

    /** Changes all the rectangle's co-ordinates. */
    void setBounds (const ValueType newX, const ValueType newY,
                    const ValueType newWidth, const ValueType newHeight) noexcept                   { pos.x = newX; pos.y = newY; w = newWidth; h = newHeight; }

    /** Changes the rectangle's X coordinate */
    inline void setX (const ValueType newX) noexcept                        { pos.x = newX; }

    /** Changes the rectangle's Y coordinate */
    inline void setY (const ValueType newY) noexcept                        { pos.y = newY; }

    /** Changes the rectangle's width */
    inline void setWidth (const ValueType newWidth) noexcept                { w = newWidth; }

    /** Changes the rectangle's height */
    inline void setHeight (const ValueType newHeight) noexcept              { h = newHeight; }

    /** Returns a rectangle which has the same size and y-position as this one, but with a different x-position. */
    Rectangle withX (const ValueType newX) const noexcept                   { return Rectangle (newX, pos.y, w, h); }

    /** Returns a rectangle which has the same size and x-position as this one, but with a different y-position. */
    Rectangle withY (const ValueType newY) const noexcept                   { return Rectangle (pos.x, newY, w, h); }

    /** Returns a rectangle with the same size as this one, but a new position. */
    Rectangle withPosition (const ValueType newX, const ValueType newY) const noexcept   { return Rectangle (newX, newY, w, h); }

    /** Returns a rectangle with the same size as this one, but a new position. */
    Rectangle withPosition (const Point<ValueType>& newPos) const noexcept  { return Rectangle (newPos.x, newPos.y, w, h); }

    /** Returns a rectangle whose size is the same as this one, but whose top-left position is (0, 0). */
    Rectangle withZeroOrigin() const noexcept                               { return Rectangle (w, h); }

    /** Returns a rectangle which has the same position and height as this one, but with a different width. */
    Rectangle withWidth (const ValueType newWidth) const noexcept           { return Rectangle (pos.x, pos.y, newWidth, h); }

    /** Returns a rectangle which has the same position and width as this one, but with a different height. */
    Rectangle withHeight (const ValueType newHeight) const noexcept         { return Rectangle (pos.x, pos.y, w, newHeight); }

    /** Returns a rectangle with the same position as this one, but a new size. */
    Rectangle withSize (const ValueType newWidth, const ValueType newHeight) const noexcept         { return Rectangle (pos.x, pos.y, newWidth, newHeight); }

    /** Moves the x position, adjusting the width so that the right-hand edge remains in the same place.
        If the x is moved to be on the right of the current right-hand edge, the width will be set to zero.
        @see withLeft
    */
    void setLeft (const ValueType newLeft) noexcept                   { w = jmax (ValueType(), pos.x + w - newLeft); pos.x = newLeft; }

    /** Returns a new rectangle with a different x position, but the same right-hand edge as this one.
        If the new x is beyond the right of the current right-hand edge, the width will be set to zero.
        @see setLeft
    */
    Rectangle withLeft (const ValueType newLeft) const noexcept       { return Rectangle (newLeft, pos.y, jmax (ValueType(), pos.x + w - newLeft), h); }

    /** Moves the y position, adjusting the height so that the bottom edge remains in the same place.
        If the y is moved to be below the current bottom edge, the height will be set to zero.
        @see withTop
    */
    void setTop (const ValueType newTop) noexcept                     { h = jmax (ValueType(), pos.y + h - newTop); pos.y = newTop; }

    /** Returns a new rectangle with a different y position, but the same bottom edge as this one.
        If the new y is beyond the bottom of the current rectangle, the height will be set to zero.
        @see setTop
    */
    Rectangle withTop (const ValueType newTop) const noexcept         { return Rectangle (pos.x, newTop, w, jmax (ValueType(), pos.y + h - newTop)); }

    /** Adjusts the width so that the right-hand edge of the rectangle has this new value.
        If the new right is below the current X value, the X will be pushed down to match it.
        @see getRight, withRight
    */
    void setRight (const ValueType newRight) noexcept                 { pos.x = jmin (pos.x, newRight); w = newRight - pos.x; }

    /** Returns a new rectangle with a different right-hand edge position, but the same left-hand edge as this one.
        If the new right edge is below the current left-hand edge, the width will be set to zero.
        @see setRight
    */
    Rectangle withRight (const ValueType newRight) const noexcept     { return Rectangle (jmin (pos.x, newRight), pos.y, jmax (ValueType(), newRight - pos.x), h); }

    /** Adjusts the height so that the bottom edge of the rectangle has this new value.
        If the new bottom is lower than the current Y value, the Y will be pushed down to match it.
        @see getBottom, withBottom
    */
    void setBottom (const ValueType newBottom) noexcept               { pos.y = jmin (pos.y, newBottom); h = newBottom - pos.y; }

    /** Returns a new rectangle with a different bottom edge position, but the same top edge as this one.
        If the new y is beyond the bottom of the current rectangle, the height will be set to zero.
        @see setBottom
    */
    Rectangle withBottom (const ValueType newBottom) const noexcept   { return Rectangle (pos.x, jmin (pos.y, newBottom), w, jmax (ValueType(), newBottom - pos.y)); }

    //==============================================================================
    /** Moves the rectangle's position by adding amount to its x and y co-ordinates. */
    void translate (const ValueType deltaX,
                    const ValueType deltaY) noexcept
    {
        pos.x += deltaX;
        pos.y += deltaY;
    }

    /** Returns a rectangle which is the same as this one moved by a given amount. */
    Rectangle translated (const ValueType deltaX,
                          const ValueType deltaY) const noexcept
    {
        return Rectangle (pos.x + deltaX, pos.y + deltaY, w, h);
    }

    /** Returns a rectangle which is the same as this one moved by a given amount. */
    Rectangle operator+ (const Point<ValueType>& deltaPosition) const noexcept
    {
        return Rectangle (pos.x + deltaPosition.x, pos.y + deltaPosition.y, w, h);
    }

    /** Moves this rectangle by a given amount. */
    Rectangle& operator+= (const Point<ValueType>& deltaPosition) noexcept
    {
        pos += deltaPosition;
        return *this;
    }

    /** Returns a rectangle which is the same as this one moved by a given amount. */
    Rectangle operator- (const Point<ValueType>& deltaPosition) const noexcept
    {
        return Rectangle (pos.x - deltaPosition.x, pos.y - deltaPosition.y, w, h);
    }

    /** Moves this rectangle by a given amount. */
    Rectangle& operator-= (const Point<ValueType>& deltaPosition) noexcept
    {
        pos -= deltaPosition;
        return *this;
    }

    /** Scales this rectangle by the given amount, centred around the origin. */
    template <typename FloatType>
    Rectangle operator* (FloatType scaleFactor) const noexcept
    {
        Rectangle r (*this);
        r *= scaleFactor;
        return r;
    }

    /** Scales this rectangle by the given amount, centred around the origin. */
    template <typename FloatType>
    Rectangle operator*= (FloatType scaleFactor) noexcept
    {
        pos *= scaleFactor;
        w *= scaleFactor;
        h *= scaleFactor;
        return *this;
    }

    /** Expands the rectangle by a given amount.

        Effectively, its new size is (x - deltaX, y - deltaY, w + deltaX * 2, h + deltaY * 2).
        @see expanded, reduce, reduced
    */
    void expand (const ValueType deltaX,
                 const ValueType deltaY) noexcept
    {
        const ValueType nw = jmax (ValueType(), w + deltaX * 2);
        const ValueType nh = jmax (ValueType(), h + deltaY * 2);
        setBounds (pos.x - deltaX, pos.y - deltaY, nw, nh);
    }

    /** Returns a rectangle that is larger than this one by a given amount.

        Effectively, the rectangle returned is (x - deltaX, y - deltaY, w + deltaX * 2, h + deltaY * 2).
        @see expand, reduce, reduced
    */
    Rectangle expanded (const ValueType deltaX,
                        const ValueType deltaY) const noexcept
    {
        const ValueType nw = jmax (ValueType(), w + deltaX * 2);
        const ValueType nh = jmax (ValueType(), h + deltaY * 2);
        return Rectangle (pos.x - deltaX, pos.y - deltaY, nw, nh);
    }

    /** Returns a rectangle that is larger than this one by a given amount.

        Effectively, the rectangle returned is (x - delta, y - delta, w + delta * 2, h + delta * 2).
        @see expand, reduce, reduced
    */
    Rectangle expanded (const ValueType delta) const noexcept
    {
        return expanded (delta, delta);
    }

    /** Shrinks the rectangle by a given amount.

        Effectively, its new size is (x + deltaX, y + deltaY, w - deltaX * 2, h - deltaY * 2).
        @see reduced, expand, expanded
    */
    void reduce (const ValueType deltaX,
                 const ValueType deltaY) noexcept
    {
        expand (-deltaX, -deltaY);
    }

    /** Returns a rectangle that is smaller than this one by a given amount.

        Effectively, the rectangle returned is (x + deltaX, y + deltaY, w - deltaX * 2, h - deltaY * 2).
        @see reduce, expand, expanded
    */
    Rectangle reduced (const ValueType deltaX,
                       const ValueType deltaY) const noexcept
    {
        return expanded (-deltaX, -deltaY);
    }

    /** Returns a rectangle that is smaller than this one by a given amount.

        Effectively, the rectangle returned is (x + delta, y + delta, w - delta * 2, h - delta * 2).
        @see reduce, expand, expanded
    */
    Rectangle reduced (const ValueType delta) const noexcept
    {
        return reduced (delta, delta);
    }

    /** Removes a strip from the top of this rectangle, reducing this rectangle
        by the specified amount and returning the section that was removed.

        E.g. if this rectangle is (100, 100, 300, 300) and amountToRemove is 50, this will
        return (100, 100, 300, 50) and leave this rectangle as (100, 150, 300, 250).

        If amountToRemove is greater than the height of this rectangle, it'll be clipped to
        that value.
    */
    Rectangle removeFromTop (const ValueType amountToRemove) noexcept
    {
        const Rectangle r (pos.x, pos.y, w, jmin (amountToRemove, h));
        pos.y += r.h; h -= r.h;
        return r;
    }

    /** Removes a strip from the left-hand edge of this rectangle, reducing this rectangle
        by the specified amount and returning the section that was removed.

        E.g. if this rectangle is (100, 100, 300, 300) and amountToRemove is 50, this will
        return (100, 100, 50, 300) and leave this rectangle as (150, 100, 250, 300).

        If amountToRemove is greater than the width of this rectangle, it'll be clipped to
        that value.
    */
    Rectangle removeFromLeft (const ValueType amountToRemove) noexcept
    {
        const Rectangle r (pos.x, pos.y, jmin (amountToRemove, w), h);
        pos.x += r.w; w -= r.w;
        return r;
    }

    /** Removes a strip from the right-hand edge of this rectangle, reducing this rectangle
        by the specified amount and returning the section that was removed.

        E.g. if this rectangle is (100, 100, 300, 300) and amountToRemove is 50, this will
        return (250, 100, 50, 300) and leave this rectangle as (100, 100, 250, 300).

        If amountToRemove is greater than the width of this rectangle, it'll be clipped to
        that value.
    */
    Rectangle removeFromRight (ValueType amountToRemove) noexcept
    {
        amountToRemove = jmin (amountToRemove, w);
        const Rectangle r (pos.x + w - amountToRemove, pos.y, amountToRemove, h);
        w -= amountToRemove;
        return r;
    }

    /** Removes a strip from the bottom of this rectangle, reducing this rectangle
        by the specified amount and returning the section that was removed.

        E.g. if this rectangle is (100, 100, 300, 300) and amountToRemove is 50, this will
        return (100, 250, 300, 50) and leave this rectangle as (100, 100, 300, 250).

        If amountToRemove is greater than the height of this rectangle, it'll be clipped to
        that value.
    */
    Rectangle removeFromBottom (ValueType amountToRemove) noexcept
    {
        amountToRemove = jmin (amountToRemove, h);
        const Rectangle r (pos.x, pos.y + h - amountToRemove, w, amountToRemove);
        h -= amountToRemove;
        return r;
    }

    //==============================================================================
    /** Returns true if the two rectangles are identical. */
    bool operator== (const Rectangle& other) const noexcept     { return pos == other.pos && w == other.w && h == other.h; }

    /** Returns true if the two rectangles are not identical. */
    bool operator!= (const Rectangle& other) const noexcept     { return pos != other.pos || w != other.w || h != other.h; }

    /** Returns true if this co-ordinate is inside the rectangle. */
    bool contains (const ValueType xCoord, const ValueType yCoord) const noexcept
    {
        return xCoord >= pos.x && yCoord >= pos.y && xCoord < pos.x + w && yCoord < pos.y + h;
    }

    /** Returns true if this co-ordinate is inside the rectangle. */
    bool contains (const Point<ValueType>& point) const noexcept
    {
        return point.x >= pos.x && point.y >= pos.y && point.x < pos.x + w && point.y < pos.y + h;
    }

    /** Returns true if this other rectangle is completely inside this one. */
    bool contains (const Rectangle& other) const noexcept
    {
        return pos.x <= other.pos.x && pos.y <= other.pos.y
            && pos.x + w >= other.pos.x + other.w && pos.y + h >= other.pos.y + other.h;
    }

    /** Returns the nearest point to the specified point that lies within this rectangle. */
    Point<ValueType> getConstrainedPoint (const Point<ValueType>& point) const noexcept
    {
        return Point<ValueType> (jlimit (pos.x, pos.x + w, point.x),
                                 jlimit (pos.y, pos.y + h, point.y));
    }

    /** Returns true if any part of another rectangle overlaps this one. */
    bool intersects (const Rectangle& other) const noexcept
    {
        return pos.x + w > other.pos.x
            && pos.y + h > other.pos.y
            && pos.x < other.pos.x + other.w
            && pos.y < other.pos.y + other.h
            && w > ValueType() && h > ValueType();
    }

    /** Returns the region that is the overlap between this and another rectangle.
        If the two rectangles don't overlap, the rectangle returned will be empty.
    */
    Rectangle getIntersection (const Rectangle& other) const noexcept
    {
        const ValueType nx = jmax (pos.x, other.pos.x);
        const ValueType ny = jmax (pos.y, other.pos.y);
        const ValueType nw = jmin (pos.x + w, other.pos.x + other.w) - nx;
        const ValueType nh = jmin (pos.y + h, other.pos.y + other.h) - ny;

        if (nw >= ValueType() && nh >= ValueType())
            return Rectangle (nx, ny, nw, nh);

        return Rectangle();
    }

    /** Clips a rectangle so that it lies only within this one.
        This is a non-static version of intersectRectangles().
        Returns false if the two regions didn't overlap.
    */
    bool intersectRectangle (ValueType& otherX, ValueType& otherY, ValueType& otherW, ValueType& otherH) const noexcept
    {
        const ValueType maxX (jmax (otherX, pos.x));
        otherW = jmin (otherX + otherW, pos.x + w) - maxX;

        if (otherW > ValueType())
        {
            const ValueType maxY (jmax (otherY, pos.y));
            otherH = jmin (otherY + otherH, pos.y + h) - maxY;

            if (otherH > ValueType())
            {
                otherX = maxX; otherY = maxY;
                return true;
            }
        }

        return false;
    }

    /** Returns the smallest rectangle that contains both this one and the one passed-in.

        If either this or the other rectangle are empty, they will not be counted as
        part of the resulting region.
    */
    Rectangle getUnion (const Rectangle& other) const noexcept
    {
        if (other.isEmpty())  return *this;
        if (isEmpty())        return other;

        const ValueType newX = jmin (pos.x, other.pos.x);
        const ValueType newY = jmin (pos.y, other.pos.y);

        return Rectangle (newX, newY,
                          jmax (pos.x + w, other.pos.x + other.w) - newX,
                          jmax (pos.y + h, other.pos.y + other.h) - newY);
    }

    /** If this rectangle merged with another one results in a simple rectangle, this
        will set this rectangle to the result, and return true.

        Returns false and does nothing to this rectangle if the two rectangles don't overlap,
        or if they form a complex region.
    */
    bool enlargeIfAdjacent (const Rectangle& other) noexcept
    {
        if (pos.x == other.pos.x && getRight() == other.getRight()
             && (other.getBottom() >= pos.y && other.pos.y <= getBottom()))
        {
            const ValueType newY = jmin (pos.y, other.pos.y);
            h = jmax (getBottom(), other.getBottom()) - newY;
            pos.y = newY;
            return true;
        }
        else if (pos.y == other.pos.y && getBottom() == other.getBottom()
                  && (other.getRight() >= pos.x && other.pos.x <= getRight()))
        {
            const ValueType newX = jmin (pos.x, other.pos.x);
            w = jmax (getRight(), other.getRight()) - newX;
            pos.x = newX;
            return true;
        }

        return false;
    }

    /** If after removing another rectangle from this one the result is a simple rectangle,
        this will set this object's bounds to be the result, and return true.

        Returns false and does nothing to this rectangle if the two rectangles don't overlap,
        or if removing the other one would form a complex region.
    */
    bool reduceIfPartlyContainedIn (const Rectangle& other) noexcept
    {
        int inside = 0;
        const ValueType otherR (other.getRight());
        if (pos.x >= other.pos.x && pos.x < otherR) inside = 1;
        const ValueType otherB (other.getBottom());
        if (pos.y >= other.pos.y && pos.y < otherB) inside |= 2;
        const ValueType r (pos.x + w);
        if (r >= other.pos.x && r < otherR) inside |= 4;
        const ValueType b (pos.y + h);
        if (b >= other.pos.y && b < otherB) inside |= 8;

        switch (inside)
        {
            case 1 + 2 + 8:     w = r - otherR; pos.x = otherR; return true;
            case 1 + 2 + 4:     h = b - otherB; pos.y = otherB; return true;
            case 2 + 4 + 8:     w = other.pos.x - pos.x; return true;
            case 1 + 4 + 8:     h = other.pos.y - pos.y; return true;
        }

        return false;
    }

    /** Tries to fit this rectangle within a target area, returning the result.

        If this rectangle is not completely inside the target area, then it'll be
        shifted (without changing its size) so that it lies within the target. If it
        is larger than the target rectangle in either dimension, then that dimension
        will be reduced to fit within the target.
    */
    Rectangle constrainedWithin (const Rectangle& areaToFitWithin) const noexcept
    {
        const ValueType newW (jmin (w, areaToFitWithin.getWidth()));
        const ValueType newH (jmin (h, areaToFitWithin.getHeight()));

        return Rectangle (jlimit (areaToFitWithin.getX(), areaToFitWithin.getRight()  - newW, pos.x),
                          jlimit (areaToFitWithin.getY(), areaToFitWithin.getBottom() - newH, pos.y),
                          newW, newH);
    }

    /** Returns the smallest rectangle that can contain the shape created by applying
        a transform to this rectangle.

        This should only be used on floating point rectangles.
    */
    Rectangle transformed (const AffineTransform& transform) const noexcept
    {
        float x1 = pos.x,     y1 = pos.y;
        float x2 = pos.x + w, y2 = pos.y;
        float x3 = pos.x,     y3 = pos.y + h;
        float x4 = x2,        y4 = y3;

        transform.transformPoints (x1, y1, x2, y2);
        transform.transformPoints (x3, y3, x4, y4);

        const float rx = jmin (x1, x2, x3, x4);
        const float ry = jmin (y1, y2, y3, y4);

        return Rectangle (rx, ry,
                          jmax (x1, x2, x3, x4) - rx,
                          jmax (y1, y2, y3, y4) - ry);
    }

    /** Returns the smallest integer-aligned rectangle that completely contains this one.
        This is only relevent for floating-point rectangles, of course.
        @see toFloat()
    */
    Rectangle<int> getSmallestIntegerContainer() const noexcept
    {
        const int x1 = static_cast <int> (std::floor (static_cast<float> (pos.x)));
        const int y1 = static_cast <int> (std::floor (static_cast<float> (pos.y)));
        const int x2 = static_cast <int> (std::ceil  (static_cast<float> (pos.x + w)));
        const int y2 = static_cast <int> (std::ceil  (static_cast<float> (pos.y + h)));

        return Rectangle<int> (x1, y1, x2 - x1, y2 - y1);
    }

    /** Returns the smallest Rectangle that can contain a set of points. */
    static Rectangle findAreaContainingPoints (const Point<ValueType>* const points, const int numPoints) noexcept
    {
        if (numPoints == 0)
            return Rectangle();

        ValueType minX (points[0].x);
        ValueType maxX (minX);
        ValueType minY (points[0].y);
        ValueType maxY (minY);

        for (int i = 1; i < numPoints; ++i)
        {
            minX = jmin (minX, points[i].x);
            maxX = jmax (maxX, points[i].x);
            minY = jmin (minY, points[i].y);
            maxY = jmax (maxY, points[i].y);
        }

        return Rectangle (minX, minY, maxX - minX, maxY - minY);
    }

    /** Casts this rectangle to a Rectangle<float>.
        Obviously this is mainly useful for rectangles that use integer types.
        @see getSmallestIntegerContainer
    */
    Rectangle<float> toFloat() const noexcept
    {
        return Rectangle<float> (static_cast<float> (pos.x), static_cast<float> (pos.y),
                                 static_cast<float> (w),     static_cast<float> (h));
    }

    //==============================================================================
    /** Static utility to intersect two sets of rectangular co-ordinates.
        Returns false if the two regions didn't overlap.
        @see intersectRectangle
    */
    static bool intersectRectangles (ValueType& x1, ValueType& y1, ValueType& w1, ValueType& h1,
                                     const ValueType x2, const ValueType y2, const ValueType w2, const ValueType h2) noexcept
    {
        const ValueType x (jmax (x1, x2));
        w1 = jmin (x1 + w1, x2 + w2) - x;

        if (w1 > ValueType())
        {
            const ValueType y (jmax (y1, y2));
            h1 = jmin (y1 + h1, y2 + h2) - y;

            if (h1 > ValueType())
            {
                x1 = x; y1 = y;
                return true;
            }
        }

        return false;
    }

    //==============================================================================
    /** Creates a string describing this rectangle.

        The string will be of the form "x y width height", e.g. "100 100 400 200".

        Coupled with the fromString() method, this is very handy for things like
        storing rectangles (particularly component positions) in XML attributes.

        @see fromString
    */
    String toString() const
    {
        String s;
        s.preallocateBytes (32);
        s << pos.x << ' ' << pos.y << ' ' << w << ' ' << h;
        return s;
    }

    /** Parses a string containing a rectangle's details.

        The string should contain 4 integer tokens, in the form "x y width height". They
        can be comma or whitespace separated.

        This method is intended to go with the toString() method, to form an easy way
        of saving/loading rectangles as strings.

        @see toString
    */
    static Rectangle fromString (const String& stringVersion)
    {
        StringArray toks;
        toks.addTokens (stringVersion.trim(), ",; \t\r\n", String::empty);

        return Rectangle (parseIntAfterSpace (toks[0]),
                          parseIntAfterSpace (toks[1]),
                          parseIntAfterSpace (toks[2]),
                          parseIntAfterSpace (toks[3]));
    }

private:
    friend class RectangleList;
    Point<ValueType> pos;
    ValueType w, h;

    static int parseIntAfterSpace (const String& s) noexcept
        { return s.getCharPointer().findEndOfWhitespace().getIntValue32(); }
};


#endif   // __JUCE_RECTANGLE_JUCEHEADER__
