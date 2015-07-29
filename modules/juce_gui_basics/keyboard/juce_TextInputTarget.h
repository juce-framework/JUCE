/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_TEXTINPUTTARGET_H_INCLUDED
#define JUCE_TEXTINPUTTARGET_H_INCLUDED


//==============================================================================
/**
    An abstract base class which can be implemented by components that function as
    text editors.

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
    virtual Range<int> getHighlightedRegion() const = 0;

    /** Sets the currently-selected text region. */
    virtual void setHighlightedRegion (const Range<int>& newRange) = 0;

    /** Sets a number of temporarily underlined sections.
        This is needed by MS Windows input method UI.
    */
    virtual void setTemporaryUnderlining (const Array <Range<int> >& underlinedRegions) = 0;

    /** Returns a specified sub-section of the text. */
    virtual String getTextInRange (const Range<int>& range) const = 0;

    /** Inserts some text, overwriting the selected text region, if there is one. */
    virtual void insertTextAtCaret (const String& textToInsert) = 0;

    /** Returns the position of the caret, relative to the component's origin. */
    virtual Rectangle<int> getCaretRectangle() = 0;

    /** A set of possible on-screen keyboard types, for use in the
        getKeyboardType() method.
    */
    enum VirtualKeyboardType
    {
        textKeyboard = 0,
        numericKeyboard,
        decimalKeyboard,
        urlKeyboard,
        emailAddressKeyboard,
        phoneNumberKeyboard
    };

    /** Returns the target's preference for the type of keyboard that would be most appropriate.
        This may be ignored, depending on the capabilities of the OS.
    */
    virtual VirtualKeyboardType getKeyboardType()       { return textKeyboard; }
};


#endif   // JUCE_TEXTINPUTTARGET_H_INCLUDED
