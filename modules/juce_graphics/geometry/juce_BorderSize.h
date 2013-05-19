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

#ifndef __JUCE_BORDERSIZE_JUCEHEADER__
#define __JUCE_BORDERSIZE_JUCEHEADER__

#include "juce_Rectangle.h"


//==============================================================================
/**
    Specifies a set of gaps to be left around the sides of a rectangle.

    This is basically the size of the spaces at the top, bottom, left and right of
    a rectangle. It's used by various component classes to specify borders.

    @see Rectangle
*/
template <typename ValueType>
class BorderSize
{
public:
    //==============================================================================
    /** Creates a null border.
        All sizes are left as 0.
    */
    BorderSize() noexcept
        : top(), left(), bottom(), right()
    {
    }

    /** Creates a copy of another border. */
    BorderSize (const BorderSize& other) noexcept
        : top (other.top), left (other.left), bottom (other.bottom), right (other.right)
    {
    }

    /** Creates a border with the given gaps. */
    BorderSize (ValueType topGap, ValueType leftGap, ValueType bottomGap, ValueType rightGap) noexcept
        : top (topGap), left (leftGap), bottom (bottomGap), right (rightGap)
    {
    }

    /** Creates a border with the given gap on all sides. */
    explicit BorderSize (ValueType allGaps) noexcept
        : top (allGaps), left (allGaps), bottom (allGaps), right (allGaps)
    {
    }

    //==============================================================================
    /** Returns the gap that should be left at the top of the region. */
    ValueType getTop() const noexcept                   { return top; }

    /** Returns the gap that should be left at the top of the region. */
    ValueType getLeft() const noexcept                  { return left; }

    /** Returns the gap that should be left at the top of the region. */
    ValueType getBottom() const noexcept                { return bottom; }

    /** Returns the gap that should be left at the top of the region. */
    ValueType getRight() const noexcept                 { return right; }

    /** Returns the sum of the top and bottom gaps. */
    ValueType getTopAndBottom() const noexcept          { return top + bottom; }

    /** Returns the sum of the left and right gaps. */
    ValueType getLeftAndRight() const noexcept          { return left + right; }

    /** Returns true if this border has no thickness along any edge. */
    bool isEmpty() const noexcept                       { return left + right + top + bottom == ValueType(); }

    //==============================================================================
    /** Changes the top gap. */
    void setTop (ValueType newTopGap) noexcept          { top = newTopGap; }

    /** Changes the left gap. */
    void setLeft (ValueType newLeftGap) noexcept        { left = newLeftGap; }

    /** Changes the bottom gap. */
    void setBottom (ValueType newBottomGap) noexcept    { bottom = newBottomGap; }

    /** Changes the right gap. */
    void setRight (ValueType newRightGap) noexcept      { right = newRightGap; }

    //==============================================================================
    /** Returns a rectangle with these borders removed from it. */
    Rectangle<ValueType> subtractedFrom (const Rectangle<ValueType>& original) const noexcept
    {
        return Rectangle<ValueType> (original.getX() + left,
                                     original.getY() + top,
                                     original.getWidth() - (left + right),
                                     original.getHeight() - (top + bottom));
    }

    /** Removes this border from a given rectangle. */
    void subtractFrom (Rectangle<ValueType>& rectangle) const noexcept
    {
        rectangle = subtractedFrom (rectangle);
    }

    /** Returns a rectangle with these borders added around it. */
    Rectangle<ValueType> addedTo (const Rectangle<ValueType>& original) const noexcept
    {
        return Rectangle<ValueType> (original.getX() - left,
                                     original.getY() - top,
                                     original.getWidth() + (left + right),
                                     original.getHeight() + (top + bottom));
    }


    /** Adds this border around a given rectangle. */
    void addTo (Rectangle<ValueType>& rectangle) const noexcept
    {
        rectangle = addedTo (rectangle);
    }

    //==============================================================================
    bool operator== (const BorderSize& other) const noexcept
    {
        return top == other.top && left == other.left && bottom == other.bottom && right == other.right;
    }

    bool operator!= (const BorderSize& other) const noexcept
    {
        return ! operator== (other);
    }

private:
    //==============================================================================
    ValueType top, left, bottom, right;
};


#endif   // __JUCE_BORDERSIZE_JUCEHEADER__
