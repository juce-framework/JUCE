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

} // namespace juce
