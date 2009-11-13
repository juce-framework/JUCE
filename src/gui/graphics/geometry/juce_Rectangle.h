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

#ifndef __JUCE_RECTANGLE_JUCEHEADER__
#define __JUCE_RECTANGLE_JUCEHEADER__

#include "../../../text/juce_String.h"


//==============================================================================
/**
    A rectangle, specified using integer co-ordinates.

    @see RectangleList, Path, Line, Point
*/
class JUCE_API  Rectangle
{
public:
    //==============================================================================
    /** Creates a rectangle of zero size.

        The default co-ordinates will be (0, 0, 0, 0).
    */
    Rectangle() throw();

    /** Creates a copy of another rectangle. */
    Rectangle (const Rectangle& other) throw();

    /** Creates a rectangle with a given position and size. */
    Rectangle (const int x, const int y,
               const int width, const int height) throw();

    /** Creates a rectangle with a given size, and a position of (0, 0). */
    Rectangle (const int width, const int height) throw();

    /** Destructor. */
    ~Rectangle() throw();

    //==============================================================================
    /** Returns the x co-ordinate of the rectangle's left-hand-side. */
    inline int getX() const throw()                         { return x; }

    /** Returns the y co-ordinate of the rectangle's top edge. */
    inline int getY() const throw()                         { return y; }

    /** Returns the width of the rectangle. */
    inline int getWidth() const throw()                     { return w; }

    /** Returns the height of the rectangle. */
    inline int getHeight() const throw()                    { return h; }

    /** Returns the x co-ordinate of the rectangle's right-hand-side. */
    inline int getRight() const throw()                     { return x + w; }

    /** Returns the y co-ordinate of the rectangle's bottom edge. */
    inline int getBottom() const throw()                    { return y + h; }

    /** Returns the x co-ordinate of the rectangle's centre. */
    inline int getCentreX() const throw()                   { return x + (w >> 1); }

    /** Returns the y co-ordinate of the rectangle's centre. */
    inline int getCentreY() const throw()                   { return y + (h >> 1); }

    /** Returns true if the rectangle's width and height are both zero or less */
    bool isEmpty() const throw();

    /** Changes the position of the rectangle's top-left corner (leaving its size unchanged). */
    void setPosition (const int x, const int y) throw();

    /** Changes the rectangle's size, leaving the position of its top-left corner unchanged. */
    void setSize (const int w, const int h) throw();

    /** Changes all the rectangle's co-ordinates. */
    void setBounds (const int newX, const int newY,
                    const int newWidth, const int newHeight) throw();

    /** Changes the rectangle's width */
    void setWidth (const int newWidth) throw();

    /** Changes the rectangle's height */
    void setHeight (const int newHeight) throw();

    /** Moves the x position, adjusting the width so that the right-hand edge remains in the same place.
        If the x is moved to be on the right of the current right-hand edge, the width will be set to zero.
    */
    void setLeft (const int newLeft) throw();

    /** Moves the y position, adjusting the height so that the bottom edge remains in the same place.
        If the y is moved to be below the current bottom edge, the height will be set to zero.
    */
    void setTop (const int newTop) throw();

    /** Adjusts the width so that the right-hand edge of the rectangle has this new value.
        If the new right is below the current X value, the X will be pushed down to match it.
        @see getRight
    */
    void setRight (const int newRight) throw();

    /** Adjusts the height so that the bottom edge of the rectangle has this new value.
        If the new bottom is lower than the current Y value, the Y will be pushed down to match it.
        @see getBottom
    */
    void setBottom (const int newBottom) throw();

    /** Moves the rectangle's position by adding amount to its x and y co-ordinates. */
    void translate (const int deltaX,
                    const int deltaY) throw();

    /** Returns a rectangle which is the same as this one moved by a given amount. */
    const Rectangle translated (const int deltaX,
                                const int deltaY) const throw();

    /** Expands the rectangle by a given amount.

        Effectively, its new size is (x - deltaX, y - deltaY, w + deltaX * 2, h + deltaY * 2).
        @see expanded, reduce, reduced
    */
    void expand (const int deltaX,
                 const int deltaY) throw();

    /** Returns a rectangle that is larger than this one by a given amount.

        Effectively, the rectangle returned is (x - deltaX, y - deltaY, w + deltaX * 2, h + deltaY * 2).
        @see expand, reduce, reduced
    */
    const Rectangle expanded (const int deltaX,
                              const int deltaY) const throw();

    /** Shrinks the rectangle by a given amount.

        Effectively, its new size is (x + deltaX, y + deltaY, w - deltaX * 2, h - deltaY * 2).
        @see reduced, expand, expanded
    */
    void reduce (const int deltaX,
                 const int deltaY) throw();

    /** Returns a rectangle that is smaller than this one by a given amount.

        Effectively, the rectangle returned is (x + deltaX, y + deltaY, w - deltaX * 2, h - deltaY * 2).
        @see reduce, expand, expanded
    */
    const Rectangle reduced (const int deltaX,
                             const int deltaY) const throw();

    //==============================================================================
    /** Returns true if the two rectangles are identical. */
    bool operator== (const Rectangle& other) const throw();

    /** Returns true if the two rectangles are not identical. */
    bool operator!= (const Rectangle& other) const throw();

    /** Returns true if this co-ordinate is inside the rectangle. */
    bool contains (const int x, const int y) const throw();

    /** Returns true if this other rectangle is completely inside this one. */
    bool contains (const Rectangle& other) const throw();

    /** Returns true if any part of another rectangle overlaps this one. */
    bool intersects (const Rectangle& other) const throw();

    /** Returns the region that is the overlap between this and another rectangle.

        If the two rectangles don't overlap, the rectangle returned will be empty.
    */
    const Rectangle getIntersection (const Rectangle& other) const throw();

    /** Clips a rectangle so that it lies only within this one.

        This is a non-static version of intersectRectangles().

        Returns false if the two regions didn't overlap.
    */
    bool intersectRectangle (int& x, int& y, int& w, int& h) const throw();

    /** Returns the smallest rectangle that contains both this one and the one
        passed-in.
    */
    const Rectangle getUnion (const Rectangle& other) const throw();

    /** If this rectangle merged with another one results in a simple rectangle, this
        will set this rectangle to the result, and return true.

        Returns false and does nothing to this rectangle if the two rectangles don't overlap,
        or if they form a complex region.
    */
    bool enlargeIfAdjacent (const Rectangle& other) throw();

    /** If after removing another rectangle from this one the result is a simple rectangle,
        this will set this object's bounds to be the result, and return true.

        Returns false and does nothing to this rectangle if the two rectangles don't overlap,
        or if removing the other one would form a complex region.
    */
    bool reduceIfPartlyContainedIn (const Rectangle& other) throw();

    //==============================================================================
    /** Static utility to intersect two sets of rectangular co-ordinates.

        Returns false if the two regions didn't overlap.

        @see intersectRectangle
    */
    static bool intersectRectangles (int& x1, int& y1, int& w1, int& h1,
                                     int x2, int y2, int w2, int h2) throw();

    //==============================================================================
    /** Creates a string describing this rectangle.

        The string will be of the form "x y width height", e.g. "100 100 400 200".

        Coupled with the fromString() method, this is very handy for things like
        storing rectangles (particularly component positions) in XML attributes.

        @see fromString
    */
    const String toString() const throw();

    /** Parses a string containing a rectangle's details.

        The string should contain 4 integer tokens, in the form "x y width height". They
        can be comma or whitespace separated.

        This method is intended to go with the toString() method, to form an easy way
        of saving/loading rectangles as strings.

        @see toString
    */
    static const Rectangle fromString (const String& stringVersion);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class RectangleList;
    int x, y, w, h;
};


#endif   // __JUCE_RECTANGLE_JUCEHEADER__
