/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
class JUCE_API  BorderSize
{
public:
    //==============================================================================
    /** Creates a null border.

        All sizes are left as 0.
    */
    BorderSize() throw();

    /** Creates a copy of another border. */
    BorderSize (const BorderSize& other) throw();

    /** Creates a border with the given gaps. */
    BorderSize (const int topGap,
                const int leftGap,
                const int bottomGap,
                const int rightGap) throw();

    /** Creates a border with the given gap on all sides. */
    BorderSize (const int allGaps) throw();

    /** Destructor. */
    ~BorderSize() throw();

    //==============================================================================
    /** Returns the gap that should be left at the top of the region. */
    int getTop() const throw()                          { return top; }

    /** Returns the gap that should be left at the top of the region. */
    int getLeft() const throw()                         { return left; }

    /** Returns the gap that should be left at the top of the region. */
    int getBottom() const throw()                       { return bottom; }

    /** Returns the gap that should be left at the top of the region. */
    int getRight() const throw()                        { return right; }

    /** Returns the sum of the top and bottom gaps. */
    int getTopAndBottom() const throw()                 { return top + bottom; }

    /** Returns the sum of the left and right gaps. */
    int getLeftAndRight() const throw()                 { return left + right; }

    //==============================================================================
    /** Changes the top gap. */
    void setTop (const int newTopGap) throw();

    /** Changes the left gap. */
    void setLeft (const int newLeftGap) throw();

    /** Changes the bottom gap. */
    void setBottom (const int newBottomGap) throw();

    /** Changes the right gap. */
    void setRight (const int newRightGap) throw();

    //==============================================================================
    /** Returns a rectangle with these borders removed from it. */
    const Rectangle subtractedFrom (const Rectangle& original) const throw();

    /** Removes this border from a given rectangle. */
    void subtractFrom (Rectangle& rectangle) const throw();

    /** Returns a rectangle with these borders added around it. */
    const Rectangle addedTo (const Rectangle& original) const throw();

    /** Adds this border around a given rectangle. */
    void addTo (Rectangle& original) const throw();

    //==============================================================================
    bool operator== (const BorderSize& other) const throw();
    bool operator!= (const BorderSize& other) const throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    int top, left, bottom, right;
};


#endif   // __JUCE_BORDERSIZE_JUCEHEADER__
