/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_NEWLINE_JUCEHEADER__
#define __JUCE_NEWLINE_JUCEHEADER__


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


#endif   // __JUCE_NEWLINE_JUCEHEADER__
