/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_NEWLINE_H_INCLUDED
#define JUCE_NEWLINE_H_INCLUDED


//==============================================================================
/** This class is used for represent a new-line character sequence.

    To write a new-line to a stream, you can use the predefined 'newLine' variable, e.g.
    @code
    myOutputStream << "Hello World" << newLine << newLine;
    @endcode

    The exact character sequence that will be used for the new-line can be set and
    retrieved with OutputStream::setNewLineString() and OutputStream::getNewLineString().
*/
class JUCE_API  NewLine
{
public:
    /** Returns the default new-line sequence that the library uses.
        @see OutputStream::setNewLineString()
    */
    static const char* getDefault() noexcept        { return "\r\n"; }

    /** Returns the default new-line sequence that the library uses.
        @see getDefault()
    */
    operator String() const                         { return getDefault(); }

    /** Returns the default new-line sequence that the library uses.
        @see OutputStream::setNewLineString()
    */
    operator StringRef() const noexcept             { return getDefault(); }
};

//==============================================================================
/** A predefined object representing a new-line, which can be written to a string or stream.

    To write a new-line to a stream, you can use the predefined 'newLine' variable like this:
    @code
    myOutputStream << "Hello World" << newLine << newLine;
    @endcode
*/
extern NewLine newLine;

//==============================================================================
/** Writes a new-line sequence to a string.
    You can use the predefined object 'newLine' to invoke this, e.g.
    @code
    myString << "Hello World" << newLine << newLine;
    @endcode
*/
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const NewLine&);

#if JUCE_STRING_UTF_TYPE != 8 && ! defined (DOXYGEN)
 inline String operator+ (String s1, const NewLine&)      { return s1 += NewLine::getDefault(); }
#endif

#endif   // JUCE_NEWLINE_H_INCLUDED
