/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** An abstract interface which represents a UI element that supports a text interface.

    A UI element can use this interface to provide extended textual information which
    cannot be conveyed using just the title, description, and help text properties of
    AccessibilityHandler. This is typically for text that an accessibility client might
    want to read line-by-line, or provide text selection and input for.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityTextInterface
{
public:
    /** Destructor. */
    virtual ~AccessibilityTextInterface() = default;

    /** Returns true if the text being displayed is protected and should not be
        exposed to the user, for example a password entry field.
    */
    virtual bool isDisplayingProtectedText() const = 0;

    /** Returns the total number of characters in the text element. */
    virtual int getTotalNumCharacters() const = 0;

    /** Returns the range of characters that are currently selected, or an empty
        range if nothing is selected.
    */
    virtual Range<int> getSelection() const = 0;

    /** Selects a section of the text. */
    virtual void setSelection (Range<int> newRange) = 0;

    /** Gets the current text insertion position, if supported. */
    virtual int getTextInsertionOffset() const = 0;

    /** Returns a section of text. */
    virtual String getText (Range<int> range) const = 0;

    /** Replaces the text with a new string. */
    virtual void setText (const String& newText) = 0;

    /** Returns the bounding box in screen coordinates for a range of text.
        As the range may span multiple lines, this method returns a RectangleList.
    */
    virtual RectangleList<int> getTextBounds (Range<int> textRange) const = 0;

    /** Returns the index of the character at a given position in screen coordinates. */
    virtual int getOffsetAtPoint (Point<int> point) const = 0;
};

} // namespace juce
