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
    Represents a type of justification to be used when positioning graphical items.

    e.g. it indicates whether something should be placed top-left, top-right,
    centred, etc.

    It is used in various places wherever this kind of information is needed.
*/
class Justification
{
public:
    //==============================================================================
    /** Creates a Justification object using a combination of flags from the Flags enum. */
    Justification (int justificationFlags) noexcept   : flags (justificationFlags) {}

    /** Creates a copy of another Justification object. */
    Justification (const Justification& other) noexcept   : flags (other.flags) {}

    /** Copies another Justification object. */
    Justification& operator= (const Justification& other) noexcept
    {
        flags = other.flags;
        return *this;
    }

    bool operator== (const Justification& other) const noexcept     { return flags == other.flags; }
    bool operator!= (const Justification& other) const noexcept     { return flags != other.flags; }

    //==============================================================================
    /** Returns the raw flags that are set for this Justification object. */
    inline int getFlags() const noexcept                        { return flags; }

    /** Tests a set of flags for this object.
        @returns true if any of the flags passed in are set on this object.
    */
    inline bool testFlags (int flagsToTest) const noexcept      { return (flags & flagsToTest) != 0; }

    /** Returns just the flags from this object that deal with vertical layout. */
    int getOnlyVerticalFlags() const noexcept                   { return flags & (top | bottom | verticallyCentred); }

    /** Returns just the flags from this object that deal with horizontal layout. */
    int getOnlyHorizontalFlags() const noexcept                 { return flags & (left | right | horizontallyCentred | horizontallyJustified); }

    //==============================================================================
    /** Adjusts the position of a rectangle to fit it into a space.

        The (x, y) position of the rectangle will be updated to position it inside the
        given space according to the justification flags.
    */
    template <typename ValueType>
    void applyToRectangle (ValueType& x, ValueType& y, ValueType w, ValueType h,
                           ValueType spaceX, ValueType spaceY, ValueType spaceW, ValueType spaceH) const noexcept
    {
        x = spaceX;
        if ((flags & horizontallyCentred) != 0)     x += (spaceW - w) / (ValueType) 2;
        else if ((flags & right) != 0)              x += spaceW - w;

        y = spaceY;
        if ((flags & verticallyCentred) != 0)       y += (spaceH - h) / (ValueType) 2;
        else if ((flags & bottom) != 0)             y += spaceH - h;
    }

    /** Returns the new position of a rectangle that has been justified to fit within a given space.
    */
    template <typename ValueType>
    const Rectangle<ValueType> appliedToRectangle (const Rectangle<ValueType>& areaToAdjust,
                                                   const Rectangle<ValueType>& targetSpace) const noexcept
    {
        ValueType x = areaToAdjust.getX(), y = areaToAdjust.getY();
        applyToRectangle (x, y, areaToAdjust.getWidth(), areaToAdjust.getHeight(),
                          targetSpace.getX(), targetSpace.getY(), targetSpace.getWidth(), targetSpace.getHeight());
        return areaToAdjust.withPosition (x, y);
    }

    //==============================================================================
    /** Flag values that can be combined and used in the constructor. */
    enum Flags
    {
        //==============================================================================
        /** Indicates that the item should be aligned against the left edge of the available space. */
        left                            = 1,

        /** Indicates that the item should be aligned against the right edge of the available space. */
        right                           = 2,

        /** Indicates that the item should be placed in the centre between the left and right
            sides of the available space. */
        horizontallyCentred             = 4,

        //==============================================================================
        /** Indicates that the item should be aligned against the top edge of the available space. */
        top                             = 8,

        /** Indicates that the item should be aligned against the bottom edge of the available space. */
        bottom                          = 16,

        /** Indicates that the item should be placed in the centre between the top and bottom
            sides of the available space. */
        verticallyCentred               = 32,

        //==============================================================================
        /** Indicates that lines of text should be spread out to fill the maximum width
            available, so that both margins are aligned vertically.
        */
        horizontallyJustified           = 64,

        //==============================================================================
        /** Indicates that the item should be centred vertically and horizontally.
            This is equivalent to (horizontallyCentred | verticallyCentred)
        */
        centred                         = 36,

        /** Indicates that the item should be centred vertically but placed on the left hand side.
            This is equivalent to (left | verticallyCentred)
        */
        centredLeft                     = 33,

        /** Indicates that the item should be centred vertically but placed on the right hand side.
            This is equivalent to (right | verticallyCentred)
        */
        centredRight                    = 34,

        /** Indicates that the item should be centred horizontally and placed at the top.
            This is equivalent to (horizontallyCentred | top)
        */
        centredTop                      = 12,

        /** Indicates that the item should be centred horizontally and placed at the bottom.
            This is equivalent to (horizontallyCentred | bottom)
        */
        centredBottom                   = 20,

        /** Indicates that the item should be placed in the top-left corner.
            This is equivalent to (left | top)
        */
        topLeft                         = 9,

        /** Indicates that the item should be placed in the top-right corner.
            This is equivalent to (right | top)
        */
        topRight                        = 10,

        /** Indicates that the item should be placed in the bottom-left corner.
            This is equivalent to (left | bottom)
        */
        bottomLeft                      = 17,

        /** Indicates that the item should be placed in the bottom-left corner.
            This is equivalent to (right | bottom)
        */
        bottomRight                     = 18
    };


private:
    //==============================================================================
    int flags;
};

} // namespace juce
