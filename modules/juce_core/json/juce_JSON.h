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

//==============================================================================
/**
    Contains static methods for converting JSON-formatted text to and from var objects.

    The var class is structurally compatible with JSON-formatted data, so these
    functions allow you to parse JSON into a var object, and to convert a var
    object to JSON-formatted text.

    @see var

    @tags{Core}
*/
class JUCE_API  JSON
{
public:
    //==============================================================================
    /** Parses a string of JSON-formatted text, and returns a result code containing
        any parse errors.

        This will return the parsed structure in the parsedResult parameter, and will
        return a Result object to indicate whether parsing was successful, and if not,
        it will contain an error message.

        If you're not interested in the error message, you can use one of the other
        shortcut parse methods, which simply return a var() if the parsing fails.

        Note that this will only parse valid JSON, which means that the item given must
        be either an object or an array definition. If you want to also be able to parse
        any kind of primitive JSON object, use the fromString() method.
    */
    static Result parse (const String& text, var& parsedResult);

    /** Attempts to parse some JSON-formatted text, and returns the result as a var object.

        If the parsing fails, this simply returns var() - if you need to find out more
        detail about the parse error, use the alternative parse() method which returns a Result.

        Note that this will only parse valid JSON, which means that the item given must
        be either an object or an array definition. If you want to also be able to parse
        any kind of primitive JSON object, use the fromString() method.
    */
    static var parse (const String& text);

    /** Attempts to parse some JSON-formatted text from a file, and returns the result
        as a var object.

        Note that this is just a short-cut for reading the entire file into a string and
        parsing the result.

        If the parsing fails, this simply returns var() - if you need to find out more
        detail about the parse error, use the alternative parse() method which returns a Result.
    */
    static var parse (const File& file);

    /** Attempts to parse some JSON-formatted text from a stream, and returns the result
        as a var object.

        Note that this is just a short-cut for reading the entire stream into a string and
        parsing the result.

        If the parsing fails, this simply returns var() - if you need to find out more
        detail about the parse error, use the alternative parse() method which returns a Result.
    */
    static var parse (InputStream& input);

    enum class Spacing
    {
        none,           ///< All optional whitespace should be omitted
        singleLine,     ///< All output should be on a single line, but with some additional spacing, e.g. after commas and colons
        multiLine,      ///< Newlines and spaces will be included in the output, in order to make it easy to read for humans
    };

    enum class Encoding
    {
        utf8,           ///< Use UTF-8 avoiding escape sequences for non-ASCII characters, this is the default behaviour
        ascii,          ///< Use ASCII characters only, unicode characters will be encoded using UTF-16 escape sequences
    };

    /**
        Allows formatting var objects as JSON with various configurable options.
    */
    class [[nodiscard]] FormatOptions
    {
    public:
        /** Returns a copy of this Formatter with the specified spacing. */
        FormatOptions withSpacing (Spacing x) const
        {
            return withMember (*this, &FormatOptions::spacing, x);
        }

        /** Returns a copy of this Formatter with the specified maximum number of decimal places.
            This option determines the precision of floating point numbers in scientific notation.
        */
        FormatOptions withMaxDecimalPlaces (int x) const
        {
            return withMember (*this, &FormatOptions::maxDecimalPlaces, x);
        }

        /** Returns a copy of this Formatter with the specified indent level.
            This should only be necessary when serialising multiline nested types.
        */
        FormatOptions withIndentLevel (int x) const
        {
            return withMember (*this, &FormatOptions::indent, x);
        }

        /** Returns a copy of this Formatter with the specified encoding.
            Use this to force a JSON to be ASCII characters only.
        */
        FormatOptions withEncoding (Encoding x) const
        {
            return withMember (*this, &FormatOptions::encoding, x);
        }

        /** Returns the spacing used by this Formatter. */
        Spacing getSpacing()      const { return spacing; }

        /** Returns the maximum number of decimal places used by this Formatter. */
        int getMaxDecimalPlaces() const { return maxDecimalPlaces; }

        /** Returns the indent level of this Formatter. */
        int getIndentLevel()      const { return indent; }

        /** Returns the encoding of this Formatter. */
        Encoding getEncoding()    const { return encoding; }

    private:
        Spacing spacing = Spacing::multiLine;
        Encoding encoding = Encoding::utf8;
        int maxDecimalPlaces = 15;
        int indent = 0;
    };

    //==============================================================================
    /** Returns a string which contains a JSON-formatted representation of the var object.
        If allOnOneLine is true, the result will be compacted into a single line of text
        with no carriage-returns. If false, it will be laid-out in a more human-readable format.
        The maximumDecimalPlaces parameter determines the precision of floating point numbers
        in scientific notation.
        @see writeToStream
    */
    static String toString (const var& objectToFormat,
                            bool allOnOneLine = false,
                            int maximumDecimalPlaces = 15);

    /** Returns a string which contains a JSON-formatted representation of the var object, using
        formatting described by the FormatOptions parameter.
        @see writeToStream
    */
    static String toString (const var& objectToFormat,
                            const FormatOptions& formatOptions);

    /** Parses a string that was created with the toString() method.
        This is slightly different to the parse() methods because they will reject primitive
        values and only accept array or object definitions, whereas this method will handle
        either.
    */
    static var fromString (StringRef);

    /** Writes a JSON-formatted representation of the var object to the given stream.
        If allOnOneLine is true, the result will be compacted into a single line of text
        with no carriage-returns. If false, it will be laid-out in a more human-readable format.
        The maximumDecimalPlaces parameter determines the precision of floating point numbers
        in scientific notation.
        @see toString
    */
    static void writeToStream (OutputStream& output,
                               const var& objectToFormat,
                               bool allOnOneLine = false,
                               int maximumDecimalPlaces = 15);

    /** Writes a JSON-formatted representation of the var object to the given stream, using
        formatting described by the FormatOptions parameter.
        @see toString
    */
    static void writeToStream (OutputStream& output,
                               const var& objectToFormat,
                               const FormatOptions& formatOptions);

    /** Returns a version of a string with any extended characters escaped. */
    static String escapeString (StringRef);

    /** Parses a quoted string-literal in JSON format, returning the un-escaped result in the
        result parameter, and an error message in case the content was illegal.
        This advances the text parameter, leaving it positioned after the closing quote.
    */
    static Result parseQuotedString (String::CharPointerType& text, var& result);

private:
    //==============================================================================
    JSON() = delete; // This class can't be instantiated - just use its static methods.
};

} // namespace juce
