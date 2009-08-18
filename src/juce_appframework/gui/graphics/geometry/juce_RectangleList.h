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

#ifndef __JUCE_RECTANGLELIST_JUCEHEADER__
#define __JUCE_RECTANGLELIST_JUCEHEADER__

#include "juce_Rectangle.h"
#include "juce_Path.h"


//==============================================================================
/**
    Maintains a set of rectangles as a complex region.

    This class allows a set of rectangles to be treated as a solid shape, and can
    add and remove rectangular sections of it, and simplify overlapping or
    adjacent rectangles.

    @see Rectangle
*/
class JUCE_API  RectangleList
{
public:
    //==============================================================================
    /** Creates an empty RectangleList */
    RectangleList() throw();

    /** Creates a copy of another list */
    RectangleList (const RectangleList& other) throw();

    /** Creates a list containing just one rectangle. */
    RectangleList (const Rectangle& rect) throw();

    /** Copies this list from another one. */
    const RectangleList& operator= (const RectangleList& other) throw();

    /** Destructor. */
    ~RectangleList() throw();

    //==============================================================================
    /** Returns true if the region is empty. */
    bool isEmpty() const throw();

    /** Returns the number of rectangles in the list. */
    int getNumRectangles() const throw()                        { return rects.size(); }

    /** Returns one of the rectangles at a particular index.

        @returns    the rectangle at the index, or an empty rectangle if the
                    index is out-of-range.
    */
    const Rectangle getRectangle (const int index) const throw();


    //==============================================================================
    /** Removes all rectangles to leave an empty region. */
    void clear() throw();

    /** Merges a new rectangle into the list.

        The rectangle being added will first be clipped to remove any parts of it
        that overlap existing rectangles in the list.
    */
    void add (const int x, const int y,
              const int w, const int h) throw();

    /** Merges a new rectangle into the list.

        The rectangle being added will first be clipped to remove any parts of it
        that overlap existing rectangles in the list, and adjacent rectangles will be
        merged into it.
    */
    void add (const Rectangle& rect) throw();

    /** Dumbly adds a rectangle to the list without checking for overlaps.

        This simply adds the rectangle to the end, it doesn't merge it or remove
        any overlapping bits.
    */
    void addWithoutMerging (const Rectangle& rect) throw();

    /** Merges another rectangle list into this one.

        Any overlaps between the two lists will be clipped, so that the result is
        the union of both lists.
    */
    void add (const RectangleList& other) throw();

    /** Removes a rectangular region from the list.

        Any rectangles in the list which overlap this will be clipped and subdivided
        if necessary.
    */
    void subtract (const Rectangle& rect) throw();

    /** Removes all areas in another RectangleList from this one.

        Any rectangles in the list which overlap this will be clipped and subdivided
        if necessary.
    */
    void subtract (const RectangleList& otherList) throw();

    /** Removes any areas of the region that lie outside a given rectangle.

        Any rectangles in the list which overlap this will be clipped and subdivided
        if necessary.

        Returns true if the resulting region is not empty, false if it is empty.

        @see getIntersectionWith
    */
    bool clipTo (const Rectangle& rect) throw();

    /** Removes any areas of the region that lie outside a given rectangle list.

        Any rectangles in this object which overlap the specified list will be clipped
        and subdivided if necessary.

        Returns true if the resulting region is not empty, false if it is empty.

        @see getIntersectionWith
    */
    bool clipTo (const RectangleList& other) throw();

    /** Creates a region which is the result of clipping this one to a given rectangle.

        Unlike the other clipTo method, this one doesn't affect this object - it puts the
        resulting region into the list whose reference is passed-in.

        Returns true if the resulting region is not empty, false if it is empty.

        @see clipTo
    */
    bool getIntersectionWith (const Rectangle& rect, RectangleList& destRegion) const throw();

    /** Swaps the contents of this and another list.

        This swaps their internal pointers, so is hugely faster than using copy-by-value
        to swap them.
    */
    void swapWith (RectangleList& otherList) throw();

    //==============================================================================
    /** Checks whether the region contains a given point.

        @returns true if the point lies within one of the rectangles in the list
    */
    bool containsPoint (const int x, const int y) const throw();

    /** Checks whether the region contains the whole of a given rectangle.

        @returns    true all parts of the rectangle passed in lie within the region
                    defined by this object
        @see intersectsRectangle, containsPoint
    */
    bool containsRectangle (const Rectangle& rectangleToCheck) const throw();

    /** Checks whether the region contains any part of a given rectangle.

        @returns    true if any part of the rectangle passed in lies within the region
                    defined by this object
        @see containsRectangle
    */
    bool intersectsRectangle (const Rectangle& rectangleToCheck) const throw();

    /** Checks whether this region intersects any part of another one.

        @see intersectsRectangle
    */
    bool intersects (const RectangleList& other) const throw();

    //==============================================================================
    /** Returns the smallest rectangle that can enclose the whole of this region. */
    const Rectangle getBounds() const throw();

    /** Optimises the list into a minimum number of constituent rectangles.

        This will try to combine any adjacent rectangles into larger ones where
        possible, to simplify lists that might have been fragmented by repeated
        add/subtract calls.
    */
    void consolidate() throw();

    /** Adds an x and y value to all the co-ordinates. */
    void offsetAll (const int dx, const int dy) throw();

    //==============================================================================
    /** Creates a Path object to represent this region. */
    const Path toPath() const throw();


    //==============================================================================
    /** An iterator for accessing all the rectangles in a RectangleList. */
    class Iterator
    {
    public:
        //==============================================================================
        Iterator (const RectangleList& list) throw();
        ~Iterator() throw();

        //==============================================================================
        /** Advances to the next rectangle, and returns true if it's not finished.

            Call this before using getRectangle() to find the rectangle that was returned.
        */
        bool next() throw();

        /** Returns the current rectangle. */
        const Rectangle* getRectangle() const throw()       { return current; }


        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        const Rectangle* current;
        const RectangleList& owner;
        int index;

        Iterator (const Iterator&);
        const Iterator& operator= (const Iterator&);
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class Iterator;
    Array <Rectangle> rects;
};


#endif   // __JUCE_RECTANGLELIST_JUCEHEADER__
