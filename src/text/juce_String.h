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

#ifndef __JUCE_STRING_JUCEHEADER__
#define __JUCE_STRING_JUCEHEADER__

#include "juce_CharacterFunctions.h"
class OutputStream;


//==============================================================================
/**
    The JUCE String class!

    Using a reference-counted internal representation, these strings are fast
    and efficient, and there are methods to do just about any operation you'll ever
    dream of.

    @see StringArray, StringPairArray
*/
class JUCE_API  String
{
public:
    //==============================================================================
    /** Creates an empty string.

        @see empty
    */
    String() throw();

    /** Creates a copy of another string. */
    String (const String& other) throw();

    /** Creates a string from a zero-terminated text string.
        The string is assumed to be stored in the default system encoding.
    */
    String (const char* text);

    /** Creates a string from an string of characters.

        This will use up the the first maxChars characters of the string (or
        less if the string is actually shorter)
    */
    String (const char* text, size_t maxChars);

    /** Creates a string from a zero-terminated unicode text string. */
    String (const juce_wchar* unicodeText);

    /** Creates a string from a unicode text string.

        This will use up the the first maxChars characters of the string (or
        less if the string is actually shorter)
    */
    String (const juce_wchar* unicodeText, size_t maxChars);

    /** Creates a string from a single character. */
    static const String charToString (juce_wchar character);

    /** Destructor. */
    ~String() throw();

    //==============================================================================
    /** This is an empty string that can be used whenever one is needed.

        It's better to use this than String() because it explains what's going on
        and is more efficient.
    */
    static const String empty;

    //==============================================================================
    /** Generates a probably-unique 32-bit hashcode from this string. */
    int hashCode() const throw();

    /** Generates a probably-unique 64-bit hashcode from this string. */
    int64 hashCode64() const throw();

    /** Returns the number of characters in the string. */
    int length() const throw();

    //==============================================================================
    // Assignment and concatenation operators..

    /** Replaces this string's contents with another string. */
    String& operator= (const String& other) throw();

    /** Appends another string at the end of this one. */
    String& operator+= (const juce_wchar* textToAppend);
    /** Appends another string at the end of this one. */
    String& operator+= (const String& stringToAppend);
    /** Appends a character at the end of this string. */
    String& operator+= (char characterToAppend);
    /** Appends a character at the end of this string. */
    String& operator+= (juce_wchar characterToAppend);
    /** Appends a decimal number at the end of this string. */
    String& operator+= (int numberToAppend);
    /** Appends a decimal number at the end of this string. */
    String& operator+= (unsigned int numberToAppend);

    /** Appends a string at the end of this one.

        @param textToAppend     the string to add
        @param maxCharsToTake   the maximum number of characters to take from the string passed in
    */
    void append (const juce_wchar* textToAppend, int maxCharsToTake);

    //==============================================================================
    // Comparison methods..

    /** Returns true if the string contains no characters.

        Note that there's also an isNotEmpty() method to help write readable code.

        @see containsNonWhitespaceChars()
    */
    inline bool isEmpty() const throw()                     { return text[0] == 0; }

    /** Returns true if the string contains at least one character.

        Note that there's also an isEmpty() method to help write readable code.

        @see containsNonWhitespaceChars()
    */
    inline bool isNotEmpty() const throw()                  { return text[0] != 0; }

    /** Case-insensitive comparison with another string. */
    bool equalsIgnoreCase (const String& other) const throw();

    /** Case-insensitive comparison with another string. */
    bool equalsIgnoreCase (const juce_wchar* other) const throw();

    /** Case-insensitive comparison with another string. */
    bool equalsIgnoreCase (const char* other) const throw();

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string
                     comes before the other one alphabetically, or positive if it
                     comes after it.
    */
    int compare (const String& other) const throw();

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string
                     comes before the other one alphabetically, or positive if it
                     comes after it.
    */
    int compare (const char* other) const throw();

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string
                     comes before the other one alphabetically, or positive if it
                     comes after it.
    */
    int compare (const juce_wchar* other) const throw();

    /** Case-insensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string
                     comes before the other one alphabetically, or positive if it
                     comes after it.
    */
    int compareIgnoreCase (const String& other) const throw();

    /** Lexicographic comparison with another string.

        The comparison used here is case-insensitive and ignores leading non-alphanumeric
        characters, making it good for sorting human-readable strings.

        @returns     0 if the two strings are identical; negative if this string
                     comes before the other one alphabetically, or positive if it
                     comes after it.
    */
    int compareLexicographically (const String& other) const throw();

    /** Tests whether the string begins with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    bool startsWith (const String& text) const throw();

    /** Tests whether the string begins with a particular character.
        If the character is 0, this will always return false.
        Uses a case-sensitive comparison.
    */
    bool startsWithChar (juce_wchar character) const throw();

    /** Tests whether the string begins with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-insensitive comparison.
    */
    bool startsWithIgnoreCase (const String& text) const throw();

    /** Tests whether the string ends with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    bool endsWith (const String& text) const throw();

    /** Tests whether the string ends with a particular character.
        If the character is 0, this will always return false.
        Uses a case-sensitive comparison.
    */
    bool endsWithChar (juce_wchar character) const throw();

    /** Tests whether the string ends with another string.
        If the parameter is an empty string, this will always return true.
        Uses a case-insensitive comparison.
    */
    bool endsWithIgnoreCase (const String& text) const throw();

    /** Tests whether the string contains another substring.
        If the parameter is an empty string, this will always return true.
        Uses a case-sensitive comparison.
    */
    bool contains (const String& text) const throw();

    /** Tests whether the string contains a particular character.
        Uses a case-sensitive comparison.
    */
    bool containsChar (juce_wchar character) const throw();

    /** Tests whether the string contains another substring.
        Uses a case-insensitive comparison.
    */
    bool containsIgnoreCase (const String& text) const throw();

    /** Tests whether the string contains another substring as a distict word.

        @returns    true if the string contains this word, surrounded by
                    non-alphanumeric characters
        @see indexOfWholeWord, containsWholeWordIgnoreCase
    */
    bool containsWholeWord (const String& wordToLookFor) const throw();

    /** Tests whether the string contains another substring as a distict word.

        @returns    true if the string contains this word, surrounded by
                    non-alphanumeric characters
        @see indexOfWholeWordIgnoreCase, containsWholeWord
    */
    bool containsWholeWordIgnoreCase (const String& wordToLookFor) const throw();

    /** Finds an instance of another substring if it exists as a distict word.

        @returns    if the string contains this word, surrounded by non-alphanumeric characters,
                    then this will return the index of the start of the substring. If it isn't
                    found, then it will return -1
        @see indexOfWholeWordIgnoreCase, containsWholeWord
    */
    int indexOfWholeWord (const String& wordToLookFor) const throw();

    /** Finds an instance of another substring if it exists as a distict word.

        @returns    if the string contains this word, surrounded by non-alphanumeric characters,
                    then this will return the index of the start of the substring. If it isn't
                    found, then it will return -1
        @see indexOfWholeWord, containsWholeWordIgnoreCase
    */
    int indexOfWholeWordIgnoreCase (const String& wordToLookFor) const throw();

    /** Looks for any of a set of characters in the string.

        Uses a case-sensitive comparison.

        @returns    true if the string contains any of the characters from
                    the string that is passed in.
    */
    bool containsAnyOf (const String& charactersItMightContain) const throw();

    /** Looks for a set of characters in the string.

        Uses a case-sensitive comparison.

        @returns    Returns false if any of the characters in this string do not occur in
                    the parameter string. If this string is empty, the return value will
                    always be true.
    */
    bool containsOnly (const String& charactersItMightContain) const throw();

    /** Returns true if this string contains any non-whitespace characters.

        This will return false if the string contains only whitespace characters, or
        if it's empty.

        It is equivalent to calling "myString.trim().isNotEmpty()".
    */
    bool containsNonWhitespaceChars() const throw();

    /** Returns true if the string matches this simple wildcard expression.

        So for example String ("abcdef").matchesWildcard ("*DEF", true) would return true.

        This isn't a full-blown regex though! The only wildcard characters supported
        are "*" and "?". It's mainly intended for filename pattern matching.
    */
    bool matchesWildcard (const String& wildcard, bool ignoreCase) const throw();

    //==============================================================================
    // Substring location methods..

    /** Searches for a character inside this string.

        Uses a case-sensitive comparison.

        @returns    the index of the first occurrence of the character in this
                    string, or -1 if it's not found.
    */
    int indexOfChar (juce_wchar characterToLookFor) const throw();

    /** Searches for a character inside this string.

        Uses a case-sensitive comparison.

        @param startIndex           the index from which the search should proceed
        @param characterToLookFor   the character to look for
        @returns            the index of the first occurrence of the character in this
                            string, or -1 if it's not found.
    */
    int indexOfChar (int startIndex, juce_wchar characterToLookFor) const throw();

    /** Returns the index of the first character that matches one of the characters
        passed-in to this method.

        This scans the string, beginning from the startIndex supplied, and if it finds
        a character that appears in the string charactersToLookFor, it returns its index.

        If none of these characters are found, it returns -1.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see indexOfChar, lastIndexOfAnyOf
    */
    int indexOfAnyOf (const String& charactersToLookFor,
                      int startIndex = 0,
                      bool ignoreCase = false) const throw();

    /** Searches for a substring within this string.

        Uses a case-sensitive comparison.

        @returns    the index of the first occurrence of this substring, or -1 if it's not found.
    */
    int indexOf (const String& text) const throw();

    /** Searches for a substring within this string.

        Uses a case-sensitive comparison.

        @param startIndex       the index from which the search should proceed
        @param textToLookFor    the string to search for
        @returns                the index of the first occurrence of this substring, or -1 if it's not found.
    */
    int indexOf (int startIndex,
                 const String& textToLookFor) const throw();

    /** Searches for a substring within this string.

        Uses a case-insensitive comparison.

        @returns    the index of the first occurrence of this substring, or -1 if it's not found.
    */
    int indexOfIgnoreCase (const String& textToLookFor) const throw();

    /** Searches for a substring within this string.

        Uses a case-insensitive comparison.

        @param startIndex       the index from which the search should proceed
        @param textToLookFor    the string to search for
        @returns                the index of the first occurrence of this substring, or -1 if it's not found.
    */
    int indexOfIgnoreCase (int startIndex,
                           const String& textToLookFor) const throw();

    /** Searches for a character inside this string (working backwards from the end of the string).

        Uses a case-sensitive comparison.

        @returns            the index of the last occurrence of the character in this
                            string, or -1 if it's not found.
    */
    int lastIndexOfChar (juce_wchar character) const throw();

    /** Searches for a substring inside this string (working backwards from the end of the string).

        Uses a case-sensitive comparison.

        @returns            the index of the start of the last occurrence of the
                            substring within this string, or -1 if it's not found.
    */
    int lastIndexOf (const String& textToLookFor) const throw();

    /** Searches for a substring inside this string (working backwards from the end of the string).

        Uses a case-insensitive comparison.

        @returns            the index of the start of the last occurrence of the
                            substring within this string, or -1 if it's not found.
    */
    int lastIndexOfIgnoreCase (const String& textToLookFor) const throw();

    /** Returns the index of the last character in this string that matches one of the
        characters passed-in to this method.

        This scans the string backwards, starting from its end, and if it finds
        a character that appears in the string charactersToLookFor, it returns its index.

        If none of these characters are found, it returns -1.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see lastIndexOf, indexOfAnyOf
    */
    int lastIndexOfAnyOf (const String& charactersToLookFor,
                          bool ignoreCase = false) const throw();


    //==============================================================================
    // Substring extraction and manipulation methods..

    /** Returns the character at this index in the string.

        No checks are made to see if the index is within a valid range, so be careful!
    */
    inline const juce_wchar& operator[] (int index) const throw()  { jassert (isPositiveAndNotGreaterThan (index, length())); return text [index]; }

    /** Returns a character from the string such that it can also be altered.

        This can be used as a way of easily changing characters in the string.

        Note that the index passed-in is not checked to see whether it's in-range, so
        be careful when using this.
    */
    juce_wchar& operator[] (int index);

    /** Returns the final character of the string.

        If the string is empty this will return 0.
    */
    juce_wchar getLastCharacter() const throw();

    //==============================================================================
    /** Returns a subsection of the string.

        If the range specified is beyond the limits of the string, as much as
        possible is returned.

        @param startIndex   the index of the start of the substring needed
        @param endIndex     all characters from startIndex up to (but not including)
                            this index are returned
        @see fromFirstOccurrenceOf, dropLastCharacters, getLastCharacters, upToFirstOccurrenceOf
    */
    const String substring (int startIndex, int endIndex) const;

    /** Returns a section of the string, starting from a given position.

        @param startIndex   the first character to include. If this is beyond the end
                            of the string, an empty string is returned. If it is zero or
                            less, the whole string is returned.
        @returns            the substring from startIndex up to the end of the string
        @see dropLastCharacters, getLastCharacters, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf
    */
    const String substring (int startIndex) const;

    /** Returns a version of this string with a number of characters removed
        from the end.

        @param numberToDrop     the number of characters to drop from the end of the
                                string. If this is greater than the length of the string,
                                an empty string will be returned. If zero or less, the
                                original string will be returned.
        @see substring, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf, getLastCharacter
    */
    const String dropLastCharacters (int numberToDrop) const;

    /** Returns a number of characters from the end of the string.

        This returns the last numCharacters characters from the end of the string. If the
        string is shorter than numCharacters, the whole string is returned.

        @see substring, dropLastCharacters, getLastCharacter
    */
    const String getLastCharacters (int numCharacters) const;

    //==============================================================================
    /** Returns a section of the string starting from a given substring.

        This will search for the first occurrence of the given substring, and
        return the section of the string starting from the point where this is
        found (optionally not including the substring itself).

        e.g. for the string "123456", fromFirstOccurrenceOf ("34", true) would return "3456", and
                                      fromFirstOccurrenceOf ("34", false) would return "56".

        If the substring isn't found, the method will return an empty string.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see upToFirstOccurrenceOf, fromLastOccurrenceOf
    */
    const String fromFirstOccurrenceOf (const String& substringToStartFrom,
                                        bool includeSubStringInResult,
                                        bool ignoreCase) const;

    /** Returns a section of the string starting from the last occurrence of a given substring.

        Similar to fromFirstOccurrenceOf(), but using the last occurrence of the substring, and
        unlike fromFirstOccurrenceOf(), if the substring isn't found, this method will
        return the whole of the original string.

        @see fromFirstOccurrenceOf, upToLastOccurrenceOf
    */
    const String fromLastOccurrenceOf (const String& substringToFind,
                                       bool includeSubStringInResult,
                                       bool ignoreCase) const;

    /** Returns the start of this string, up to the first occurrence of a substring.

        This will search for the first occurrence of a given substring, and then
        return a copy of the string, up to the position of this substring,
        optionally including or excluding the substring itself in the result.

        e.g. for the string "123456", upTo ("34", false) would return "12", and
                                      upTo ("34", true) would return "1234".

        If the substring isn't found, this will return the whole of the original string.

        @see upToLastOccurrenceOf, fromFirstOccurrenceOf
    */
    const String upToFirstOccurrenceOf (const String& substringToEndWith,
                                        bool includeSubStringInResult,
                                        bool ignoreCase) const;

    /** Returns the start of this string, up to the last occurrence of a substring.

        Similar to upToFirstOccurrenceOf(), but this finds the last occurrence rather than the first.
        If the substring isn't found, this will return the whole of the original string.

        @see upToFirstOccurrenceOf, fromFirstOccurrenceOf
    */
    const String upToLastOccurrenceOf (const String& substringToFind,
                                       bool includeSubStringInResult,
                                       bool ignoreCase) const;

    //==============================================================================
    /** Returns a copy of this string with any whitespace characters removed from the start and end. */
    const String trim() const;
    /** Returns a copy of this string with any whitespace characters removed from the start. */
    const String trimStart() const;
    /** Returns a copy of this string with any whitespace characters removed from the end. */
    const String trimEnd() const;

    /** Returns a copy of this string, having removed a specified set of characters from its start.
        Characters are removed from the start of the string until it finds one that is not in the
        specified set, and then it stops.
        @param charactersToTrim     the set of characters to remove.
        @see trim, trimStart, trimCharactersAtEnd
    */
    const String trimCharactersAtStart (const String& charactersToTrim) const;

    /** Returns a copy of this string, having removed a specified set of characters from its end.
        Characters are removed from the end of the string until it finds one that is not in the
        specified set, and then it stops.
        @param charactersToTrim     the set of characters to remove.
        @see trim, trimEnd, trimCharactersAtStart
    */
    const String trimCharactersAtEnd (const String& charactersToTrim) const;

    //==============================================================================
    /** Returns an upper-case version of this string. */
    const String toUpperCase() const;

    /** Returns an lower-case version of this string. */
    const String toLowerCase() const;

    //==============================================================================
    /** Replaces a sub-section of the string with another string.

        This will return a copy of this string, with a set of characters
        from startIndex to startIndex + numCharsToReplace removed, and with
        a new string inserted in their place.

        Note that this is a const method, and won't alter the string itself.

        @param startIndex               the first character to remove. If this is beyond the bounds of the string,
                                        it will be constrained to a valid range.
        @param numCharactersToReplace   the number of characters to remove. If zero or less, no
                                        characters will be taken out.
        @param stringToInsert           the new string to insert at startIndex after the characters have been
                                        removed.
    */
    const String replaceSection (int startIndex,
                                 int numCharactersToReplace,
                                 const String& stringToInsert) const;

    /** Replaces all occurrences of a substring with another string.

        Returns a copy of this string, with any occurrences of stringToReplace
        swapped for stringToInsertInstead.

        Note that this is a const method, and won't alter the string itself.
    */
    const String replace (const String& stringToReplace,
                          const String& stringToInsertInstead,
                          bool ignoreCase = false) const;

    /** Returns a string with all occurrences of a character replaced with a different one. */
    const String replaceCharacter (juce_wchar characterToReplace,
                                   juce_wchar characterToInsertInstead) const;

    /** Replaces a set of characters with another set.

        Returns a string in which each character from charactersToReplace has been replaced
        by the character at the equivalent position in newCharacters (so the two strings
        passed in must be the same length).

        e.g. replaceCharacters ("abc", "def") replaces 'a' with 'd', 'b' with 'e', etc.

        Note that this is a const method, and won't affect the string itself.
    */
    const String replaceCharacters (const String& charactersToReplace,
                                    const String& charactersToInsertInstead) const;

    /** Returns a version of this string that only retains a fixed set of characters.

        This will return a copy of this string, omitting any characters which are not
        found in the string passed-in.

        e.g. for "1122334455", retainCharacters ("432") would return "223344"

        Note that this is a const method, and won't alter the string itself.
    */
    const String retainCharacters (const String& charactersToRetain) const;

    /** Returns a version of this string with a set of characters removed.

        This will return a copy of this string, omitting any characters which are
        found in the string passed-in.

        e.g. for "1122334455", removeCharacters ("432") would return "1155"

        Note that this is a const method, and won't alter the string itself.
    */
    const String removeCharacters (const String& charactersToRemove) const;

    /** Returns a section from the start of the string that only contains a certain set of characters.

        This returns the leftmost section of the string, up to (and not including) the
        first character that doesn't appear in the string passed in.
    */
    const String initialSectionContainingOnly (const String& permittedCharacters) const;

    /** Returns a section from the start of the string that only contains a certain set of characters.

        This returns the leftmost section of the string, up to (and not including) the
        first character that occurs in the string passed in.
    */
    const String initialSectionNotContaining (const String& charactersToStopAt) const;

    //==============================================================================
    /** Checks whether the string might be in quotation marks.

        @returns    true if the string begins with a quote character (either a double or single quote).
                    It is also true if there is whitespace before the quote, but it doesn't check the end of the string.
        @see unquoted, quoted
    */
    bool isQuotedString() const;

    /** Removes quotation marks from around the string, (if there are any).

        Returns a copy of this string with any quotes removed from its ends. Quotes that aren't
        at the ends of the string are not affected. If there aren't any quotes, the original string
        is returned.

        Note that this is a const method, and won't alter the string itself.

        @see isQuotedString, quoted
    */
    const String unquoted() const;

    /** Adds quotation marks around a string.

        This will return a copy of the string with a quote at the start and end, (but won't
        add the quote if there's already one there, so it's safe to call this on strings that
        may already have quotes around them).

        Note that this is a const method, and won't alter the string itself.

        @param quoteCharacter   the character to add at the start and end
        @see isQuotedString, unquoted
    */
    const String quoted (juce_wchar quoteCharacter = '"') const;


    //==============================================================================
    /** Creates a string which is a version of a string repeated and joined together.

        @param stringToRepeat         the string to repeat
        @param numberOfTimesToRepeat  how many times to repeat it
    */
    static const String repeatedString (const String& stringToRepeat,
                                        int numberOfTimesToRepeat);

    /** Returns a copy of this string with the specified character repeatedly added to its
        beginning until the total length is at least the minimum length specified.
    */
    const String paddedLeft (juce_wchar padCharacter, int minimumLength) const;

    /** Returns a copy of this string with the specified character repeatedly added to its
        end until the total length is at least the minimum length specified.
    */
    const String paddedRight (juce_wchar padCharacter, int minimumLength) const;

    /** Creates a string from data in an unknown format.

        This looks at some binary data and tries to guess whether it's Unicode
        or 8-bit characters, then returns a string that represents it correctly.

        Should be able to handle Unicode endianness correctly, by looking at
        the first two bytes.
    */
    static const String createStringFromData (const void* data, int size);

    /** Creates a String from a printf-style parameter list.

        I don't like this method. I don't use it myself, and I recommend avoiding it and
        using the operator<< methods or pretty much anything else instead. It's only provided
        here because of the popular unrest that was stirred-up when I tried to remove it...

        If you're really determined to use it, at least make sure that you never, ever,
        pass any String objects to it as parameters.
    */
    static const String formatted (const juce_wchar* formatString, ... );

    //==============================================================================
    // Numeric conversions..

    /** Creates a string containing this signed 32-bit integer as a decimal number.

        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (int decimalInteger);

    /** Creates a string containing this unsigned 32-bit integer as a decimal number.

        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (unsigned int decimalInteger);

    /** Creates a string containing this signed 16-bit integer as a decimal number.

        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (short decimalInteger);

    /** Creates a string containing this unsigned 16-bit integer as a decimal number.

        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (unsigned short decimalInteger);

    /** Creates a string containing this signed 64-bit integer as a decimal number.

        @see getLargeIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (int64 largeIntegerValue);

    /** Creates a string containing this unsigned 64-bit integer as a decimal number.

        @see getLargeIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (uint64 largeIntegerValue);

    /** Creates a string representing this floating-point number.

        @param floatValue               the value to convert to a string
        @param numberOfDecimalPlaces    if this is > 0, it will format the number using that many
                                        decimal places, and will not use exponent notation. If 0 or
                                        less, it will use exponent notation if necessary.
        @see getDoubleValue, getIntValue
    */
    explicit String (float floatValue,
                     int numberOfDecimalPlaces = 0);

    /** Creates a string representing this floating-point number.

        @param doubleValue              the value to convert to a string
        @param numberOfDecimalPlaces    if this is > 0, it will format the number using that many
                                        decimal places, and will not use exponent notation. If 0 or
                                        less, it will use exponent notation if necessary.

        @see getFloatValue, getIntValue
    */
    explicit String (double doubleValue,
                     int numberOfDecimalPlaces = 0);

    /** Reads the value of the string as a decimal number (up to 32 bits in size).

        @returns the value of the string as a 32 bit signed base-10 integer.
        @see getTrailingIntValue, getHexValue32, getHexValue64
    */
    int getIntValue() const throw();

    /** Reads the value of the string as a decimal number (up to 64 bits in size).

        @returns the value of the string as a 64 bit signed base-10 integer.
    */
    int64 getLargeIntValue() const throw();

    /** Parses a decimal number from the end of the string.

        This will look for a value at the end of the string.
        e.g. for "321 xyz654" it will return 654; for "2 3 4" it'll return 4.

        Negative numbers are not handled, so "xyz-5" returns 5.

        @see getIntValue
    */
    int getTrailingIntValue() const throw();

    /** Parses this string as a floating point number.

        @returns    the value of the string as a 32-bit floating point value.
        @see getDoubleValue
    */
    float getFloatValue() const throw();

    /** Parses this string as a floating point number.

        @returns    the value of the string as a 64-bit floating point value.
        @see getFloatValue
    */
    double getDoubleValue() const throw();

    /** Parses the string as a hexadecimal number.

        Non-hexadecimal characters in the string are ignored.

        If the string contains too many characters, then the lowest significant
        digits are returned, e.g. "ffff12345678" would produce 0x12345678.

        @returns    a 32-bit number which is the value of the string in hex.
    */
    int getHexValue32() const throw();

    /** Parses the string as a hexadecimal number.

        Non-hexadecimal characters in the string are ignored.

        If the string contains too many characters, then the lowest significant
        digits are returned, e.g. "ffff1234567812345678" would produce 0x1234567812345678.

        @returns    a 64-bit number which is the value of the string in hex.
    */
    int64 getHexValue64() const throw();

    /** Creates a string representing this 32-bit value in hexadecimal. */
    static const String toHexString (int number);

    /** Creates a string representing this 64-bit value in hexadecimal. */
    static const String toHexString (int64 number);

    /** Creates a string representing this 16-bit value in hexadecimal. */
    static const String toHexString (short number);

    /** Creates a string containing a hex dump of a block of binary data.

        @param data         the binary data to use as input
        @param size         how many bytes of data to use
        @param groupSize    how many bytes are grouped together before inserting a
                            space into the output. e.g. group size 0 has no spaces,
                            group size 1 looks like: "be a1 c2 ff", group size 2 looks
                            like "bea1 c2ff".
    */
    static const String toHexString (const unsigned char* data,
                                     int size,
                                     int groupSize = 1);

    //==============================================================================
    /** Returns a unicode version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can become invalid whenever
        any string methods (even some const ones!) are called.
    */
    inline operator const juce_wchar*() const throw()   { return text; }

    //==============================================================================
    /** Returns a unicode version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can become invalid whenever
        any string methods (even some const ones!) are called.
    */
    inline operator juce_wchar*() throw()               { return text; }

    //==============================================================================
    /** Returns a pointer to a UTF-8 version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        @see getNumBytesAsUTF8, fromUTF8, copyToUTF8, toCString
    */
    const char* toUTF8() const;

    /** Creates a String from a UTF-8 encoded buffer.

        If the size is < 0, it'll keep reading until it hits a zero.
    */
    static const String fromUTF8 (const char* utf8buffer, int bufferSizeBytes = -1);

    /** Returns the number of bytes required to represent this string as UTF8.
        The number returned does NOT include the trailing zero.
        @see toUTF8, copyToUTF8
    */
    int getNumBytesAsUTF8() const throw();

    /** Copies the string to a buffer as UTF-8 characters.

        Returns the number of bytes copied to the buffer, including the terminating null
        character.

        @param destBuffer       the place to copy it to; if this is a null pointer,
                                the method just returns the number of bytes required
                                (including the terminating null character).
        @param maxBufferSizeBytes  the size of the destination buffer, in bytes. If the
                                string won't fit, it'll put in as many as it can while
                                still allowing for a terminating null char at the end, and
                                will return the number of bytes that were actually used.
    */
    int copyToUTF8 (char* destBuffer, int maxBufferSizeBytes) const throw();

    //==============================================================================
    /** Returns a version of this string using the default 8-bit multi-byte system encoding.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.

        @see getNumBytesAsCString, copyToCString, toUTF8
    */
    const char* toCString() const;

    /** Returns the number of bytes
    */
    int getNumBytesAsCString() const throw();

    /** Copies the string to a buffer.

        @param destBuffer       the place to copy it to; if this is a null pointer,
                                the method just returns the number of bytes required
                                (including the terminating null character).
        @param maxBufferSizeBytes  the size of the destination buffer, in bytes. If the
                                string won't fit, it'll put in as many as it can while
                                still allowing for a terminating null char at the end, and
                                will return the number of bytes that were actually used.
    */
    int copyToCString (char* destBuffer, int maxBufferSizeBytes) const throw();

    //==============================================================================
    /** Copies the string to a unicode buffer.

        @param destBuffer       the place to copy it to
        @param maxCharsToCopy   the maximum number of characters to copy to the buffer,
                                NOT including the trailing zero, so this shouldn't be
                                larger than the size of your destination buffer - 1
    */
    void copyToUnicode (juce_wchar* destBuffer, int maxCharsToCopy) const throw();


    //==============================================================================
    /** Increases the string's internally allocated storage.

        Although the string's contents won't be affected by this call, it will
        increase the amount of memory allocated internally for the string to grow into.

        If you're about to make a large number of calls to methods such
        as += or <<, it's more efficient to preallocate enough extra space
        beforehand, so that these methods won't have to keep resizing the string
        to append the extra characters.

        @param numCharsNeeded   the number of characters to allocate storage for. If this
                                value is less than the currently allocated size, it will
                                have no effect.
    */
    void preallocateStorage (size_t numCharsNeeded);

    /** Swaps the contents of this string with another one.
        This is a very fast operation, as no allocation or copying needs to be done.
    */
    void swapWith (String& other) throw();

    //==============================================================================
    /** A helper class to improve performance when concatenating many large strings
        together.

        Because appending one string to another involves measuring the length of
        both strings, repeatedly doing this for many long strings will become
        an exponentially slow operation. This class uses some internal state to
        avoid that, so that each append operation only needs to measure the length
        of the appended string.
    */
    class JUCE_API  Concatenator
    {
    public:
        Concatenator (String& stringToAppendTo);
        ~Concatenator();

        void append (const String& s);

    private:
        String& result;
        int nextIndex;

        JUCE_DECLARE_NON_COPYABLE (Concatenator);
    };

private:
    //==============================================================================
    juce_wchar* text;

    //==============================================================================
    struct Preallocation
    {
        explicit Preallocation (size_t);
        size_t numChars;
    };

    // This constructor preallocates a certain amount of memory
    explicit String (const Preallocation&);
    String (const String& stringToCopy, size_t charsToAllocate);

    void createInternal (const juce_wchar* text, size_t numChars);
    void appendInternal (const juce_wchar* text, int numExtraChars);

    // This private cast operator should prevent strings being accidentally cast
    // to bools (this is possible because the compiler can add an implicit cast
    // via a const char*)
    operator bool() const throw()   { return false; }
};

//==============================================================================
/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (const char* string1,       const String& string2);
/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (const juce_wchar* string1, const String& string2);
/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (char string1,              const String& string2);
/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (juce_wchar string1,        const String& string2);

/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (String string1, const String& string2);
/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (String string1, const char* string2);
/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (String string1, const juce_wchar* string2);
/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (String string1, char characterToAppend);
/** Concatenates two strings. */
JUCE_API const String JUCE_CALLTYPE operator+  (String string1, juce_wchar characterToAppend);

//==============================================================================
/** Appends a character at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, char characterToAppend);
/** Appends a character at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, juce_wchar characterToAppend);
/** Appends a string to the end of the first one. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const char* string2);
/** Appends a string to the end of the first one. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const juce_wchar* string2);
/** Appends a string to the end of the first one. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const String& string2);

/** Appends a decimal number at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, short number);
/** Appends a decimal number at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, int number);
/** Appends a decimal number at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, unsigned int number);
/** Appends a decimal number at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, long number);
/** Appends a decimal number at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, unsigned long number);
/** Appends a decimal number at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, float number);
/** Appends a decimal number at the end of a string. */
JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, double number);

//==============================================================================
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const String& string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const char* string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const juce_wchar* string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const String& string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const char* string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const juce_wchar* string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator>  (const String& string1, const String& string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator<  (const String& string1, const String& string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator>= (const String& string1, const String& string2) throw();
/** Case-sensitive comparison of two strings. */
JUCE_API bool JUCE_CALLTYPE operator<= (const String& string1, const String& string2) throw();

//==============================================================================
/** This streaming override allows you to pass a juce String directly into std output streams.
    This is very handy for writing strings to std::cout, std::cerr, etc.
*/
template <class charT, class traits>
JUCE_API std::basic_ostream <charT, traits>& JUCE_CALLTYPE operator<< (std::basic_ostream <charT, traits>& stream, const String& stringToWrite)
{
    return stream << stringToWrite.toUTF8();
}

/** Writes a string to an OutputStream as UTF8. */
JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const String& text);


#endif   // __JUCE_STRING_JUCEHEADER__
