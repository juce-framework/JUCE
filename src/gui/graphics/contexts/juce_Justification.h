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

#ifndef __JUCE_JUSTIFICATION_JUCEHEADER__
#define __JUCE_JUSTIFICATION_JUCEHEADER__


//==============================================================================
/**
    Represents a type of justification to be used when positioning graphical items.

    e.g. it indicates whether something should be placed top-left, top-right,
    centred, etc.

    It is used in various places wherever this kind of information is needed.
*/
class JUCE_API  Justification
{
public:
    //==============================================================================
    /** Creates a Justification object using a combination of flags. */
    inline Justification (const int flags_) throw()  : flags (flags_) {}

    /** Creates a copy of another Justification object. */
    Justification (const Justification& other) throw();

    /** Copies another Justification object. */
    const Justification& operator= (const Justification& other) throw();


    //==============================================================================
    /** Returns the raw flags that are set for this Justification object. */
    inline int getFlags() const throw()                             { return flags; }

    /** Tests a set of flags for this object.

        @returns true if any of the flags passed in are set on this object.
    */
    inline bool testFlags (const int flagsToTest) const throw()     { return (flags & flagsToTest) != 0; }

    /** Returns just the flags from this object that deal with vertical layout. */
    int getOnlyVerticalFlags() const throw();

    /** Returns just the flags from this object that deal with horizontal layout. */
    int getOnlyHorizontalFlags() const throw();

    //==============================================================================
    /** Adjusts the position of a rectangle to fit it into a space.

        The (x, y) position of the rectangle will be updated to position it inside the
        given space according to the justification flags.
    */
    void applyToRectangle (int& x, int& y,
                           const int w, const int h,
                           const int spaceX, const int spaceY,
                           const int spaceW, const int spaceH) const throw();


    //==============================================================================
    /** Flag values that can be combined and used in the constructor. */
    enum
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

#endif   // __JUCE_JUSTIFICATION_JUCEHEADER__
