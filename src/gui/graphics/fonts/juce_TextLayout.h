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

#ifndef __JUCE_TEXTLAYOUT_JUCEHEADER__
#define __JUCE_TEXTLAYOUT_JUCEHEADER__

#include "juce_Font.h"
#include "../contexts/juce_Justification.h"
#include "../../../containers/juce_VoidArray.h"
class Graphics;


//==============================================================================
/**
    A laid-out arrangement of text.

    You can add text in different fonts to a TextLayout object, then call its
    layout() method to word-wrap it into lines. The layout can then be drawn
    using a graphics context.

    It's handy if you've got a message to display, because you can format it,
    measure the extent of the layout, and then create a suitably-sized window
    to show it in.

    @see Font, Graphics::drawFittedText, GlyphArrangement
*/
class JUCE_API  TextLayout
{
public:
    //==============================================================================
    /** Creates an empty text layout.

        Text can then be appended using the appendText() method.
    */
    TextLayout() throw();

    /** Creates a copy of another layout object. */
    TextLayout (const TextLayout& other) throw();

    /** Creates a text layout from an initial string and font. */
    TextLayout (const String& text, const Font& font) throw();

    /** Destructor. */
    ~TextLayout() throw();

    /** Copies another layout onto this one. */
    const TextLayout& operator= (const TextLayout& layoutToCopy) throw();

    //==============================================================================
    /** Clears the layout, removing all its text. */
    void clear() throw();

    /** Adds a string to the end of the arrangement.

        The string will be broken onto new lines wherever it contains
        carriage-returns or linefeeds. After adding it, you can call layout()
        to wrap long lines into a paragraph and justify it.
    */
    void appendText (const String& textToAppend,
                     const Font& fontToUse) throw();

    /** Replaces all the text with a new string.

        This is equivalent to calling clear() followed by appendText().
    */
    void setText (const String& newText,
                  const Font& fontToUse) throw();

    //==============================================================================
    /** Breaks the text up to form a paragraph with the given width.

        @param maximumWidth                 any text wider than this will be split
                                            across multiple lines
        @param justification                how the lines are to be laid-out horizontally
        @param attemptToBalanceLineLengths  if true, it will try to split the lines at a
                                            width that keeps all the lines of text at a
                                            similar length - this is good when you're displaying
                                            a short message and don't want it to get split
                                            onto two lines with only a couple of words on
                                            the second line, which looks untidy.
    */
    void layout (int maximumWidth,
                 const Justification& justification,
                 const bool attemptToBalanceLineLengths) throw();


    //==============================================================================
    /** Returns the overall width of the entire text layout. */
    int getWidth() const throw();

    /** Returns the overall height of the entire text layout. */
    int getHeight() const throw();

    /** Returns the total number of lines of text. */
    int getNumLines() const throw()                 { return totalLines; }

    /** Returns the width of a particular line of text.

        @param lineNumber   the line, from 0 to (getNumLines() - 1)
    */
    int getLineWidth (const int lineNumber) const throw();

    //==============================================================================
    /** Renders the text at a specified position using a graphics context.
    */
    void draw (Graphics& g,
               const int topLeftX,
               const int topLeftY) const throw();

    /** Renders the text within a specified rectangle using a graphics context.

        The justification flags dictate how the block of text should be positioned
        within the rectangle.
    */
    void drawWithin (Graphics& g,
                     int x, int y, int w, int h,
                     const Justification& layoutFlags) const throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

    /** @internal */
    class Token;

private:
    OwnedArray <Token> tokens;
    int totalLines;
};


#endif   // __JUCE_TEXTLAYOUT_JUCEHEADER__
