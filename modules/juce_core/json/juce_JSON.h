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

#ifndef __JUCE_JSON_JUCEHEADER__
#define __JUCE_JSON_JUCEHEADER__

#include "../misc/juce_Result.h"
#include "../containers/juce_Variant.h"
class InputStream;
class OutputStream;
class File;


//==============================================================================
/**
    Contains static methods for converting JSON-formatted text to and from var objects.

    The var class is structurally compatible with JSON-formatted data, so these
    functions allow you to parse JSON into a var object, and to convert a var
    object to JSON-formatted text.

    @see var
*/
class JSON
{
public:
    //==============================================================================
    /** Parses a string of JSON-formatted text, and returns a result code containing
        any parse errors.

        This will return the parsed structure in the parsedResult parameter, and will
        return a Result object to indicate whether parsing was successful, and if not,
        it will contain an error message.

        If you're not interested in the error message, you can use one of the other
        shortcut parse methods, which simply return a var::null if the parsing fails.
    */
    static Result parse (const String& text, var& parsedResult);

    /** Attempts to parse some JSON-formatted text, and returns the result as a var object.

        If the parsing fails, this simply returns var::null - if you need to find out more
        detail about the parse error, use the alternative parse() method which returns a Result.
    */
    static var parse (const String& text);

    /** Attempts to parse some JSON-formatted text from a file, and returns the result
        as a var object.

        Note that this is just a short-cut for reading the entire file into a string and
        parsing the result.

        If the parsing fails, this simply returns var::null - if you need to find out more
        detail about the parse error, use the alternative parse() method which returns a Result.
    */
    static var parse (const File& file);

    /** Attempts to parse some JSON-formatted text from a stream, and returns the result
        as a var object.

        Note that this is just a short-cut for reading the entire stream into a string and
        parsing the result.

        If the parsing fails, this simply returns var::null - if you need to find out more
        detail about the parse error, use the alternative parse() method which returns a Result.
    */
    static var parse (InputStream& input);

    //==============================================================================
    /** Returns a string which contains a JSON-formatted representation of the var object.
        If allOnOneLine is true, the result will be compacted into a single line of text
        with no carriage-returns. If false, it will be laid-out in a more human-readable format.
        @see writeToStream
    */
    static String toString (const var& objectToFormat,
                            bool allOnOneLine = false);

    /** Writes a JSON-formatted representation of the var object to the given stream.
        If allOnOneLine is true, the result will be compacted into a single line of text
        with no carriage-returns. If false, it will be laid-out in a more human-readable format.
        @see toString
    */
    static void writeToStream (OutputStream& output,
                               const var& objectToFormat,
                               bool allOnOneLine = false);

private:
    //==============================================================================
    JSON(); // This class can't be instantiated - just use its static methods.
};


#endif   // __JUCE_JSON_JUCEHEADER__
