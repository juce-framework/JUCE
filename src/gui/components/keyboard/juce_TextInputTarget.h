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

#ifndef __JUCE_TEXTINPUTTARGET_JUCEHEADER__
#define __JUCE_TEXTINPUTTARGET_JUCEHEADER__

#include "../../../maths/juce_Range.h"


//==============================================================================
/** An abstract base class that is implemented by components that wish to be used
    as text editors.

    This class allows different types of text editor component to provide a uniform
    interface, which can be used by things like OS-specific input methods, on-screen
    keyboards, etc.
*/
class JUCE_API  TextInputTarget
{
public:
    //==============================================================================
    /** */
    TextInputTarget() {}

    /** Destructor. */
    virtual ~TextInputTarget() {}

    /** Returns true if this input target is currently accepting input.
        For example, a text editor might return false if it's in read-only mode.
    */
    virtual bool isTextInputActive() const = 0;

    /** Returns the extents of the selected text region, or an empty range if
        nothing is selected,
    */
    virtual const Range<int> getHighlightedRegion() const = 0;

    /** Sets the currently-selected text region.
    */
    virtual void setHighlightedRegion (const Range<int>& newRange) = 0;

    /** Returns a specified sub-section of the text.
    */
    virtual const String getTextInRange (const Range<int>& range) const = 0;

    /** Inserts some text, overwriting the selected text region, if there is one. */
    virtual void insertTextAtCaret (const String& textToInsert) = 0;
};


#endif   // __JUCE_TEXTINPUTTARGET_JUCEHEADER__
