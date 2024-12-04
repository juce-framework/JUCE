/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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

    /** Returns true if the text being displayed is read-only or false if editable. */
    virtual bool isReadOnly() const = 0;

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

    /** Returns the full text. */
    String getAllText() const { return getText ({ 0, getTotalNumCharacters() }); }

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
