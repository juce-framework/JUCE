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

#ifndef __JUCE_STRING_JUCEHEADER__
#define __JUCE_STRING_JUCEHEADER__

#include "juce_CharacterFunctions.h"


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
    String (const char* const text) throw();

    /** Creates a string from an string of characters.

        This will use up the the first maxChars characters of the string (or
        less if the string is actually shorter)
    */
    String (const char* const text,
            const size_t maxChars) throw();

    /** Creates a string from a zero-terminated unicode text string. */
    String (const juce_wchar* const unicodeText) throw();

    /** Creates a string from a unicode text string.

        This will use up the the first maxChars characters of the string (or
        less if the string is actually shorter)
    */
    String (const juce_wchar* const unicodeText,
            const size_t maxChars) throw();

    /** Creates a string from a single character. */
    static const String charToString (const tchar character) throw();

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
    const String& operator= (const tchar* const other) throw();

    /** Replaces this string's contents with another string. */
    const String& operator= (const String& other) throw();

    /** Appends another string at the end of this one. */
    const String& operator+= (const tchar* const textToAppend) throw();
    /** Appends another string at the end of this one. */
    const String& operator+= (const String& stringToAppend) throw();
    /** Appends a character at the end of this string. */
    const String& operator+= (const char characterToAppend) throw();
    /** Appends a character at the end of this string. */
    const String& operator+= (const juce_wchar characterToAppend) throw();

    /** Appends a string at the end of this one.

        @param textToAppend     the string to add
        @param maxCharsToTake   the maximum number of characters to take from the string passed in
    */
    void append (const tchar* const textToAppend,
                 const int maxCharsToTake) throw();

    /** Appends a string at the end of this one.
        @returns     the concatenated string
    */
    const String operator+ (const String& stringToAppend) const throw();

    /** Appends a string at the end of this one.
        @returns     the concatenated string
    */
    const String operator+ (const tchar* const textToAppend) const throw();

    /** Appends a character at the end of this one.
        @returns     the concatenated string
    */
    const String operator+ (const tchar characterToAppend) const throw();

    /** Appends a character at the end of this string. */
    String& operator<< (const char n) throw();
    /** Appends a character at the end of this string. */
    String& operator<< (const juce_wchar n) throw();
    /** Appends another string at the end of this one. */
    String& operator<< (const char* const text) throw();
    /** Appends another string at the end of this one. */
    String& operator<< (const juce_wchar* const text) throw();
    /** Appends another string at the end of this one. */
    String& operator<< (const String& text) throw();

    /** Appends a decimal number at the end of this string. */
    String& operator<< (const short number) throw();
    /** Appends a decimal number at the end of this string. */
    String& operator<< (const int number) throw();
    /** Appends a decimal number at the end of this string. */
    String& operator<< (const unsigned int number) throw();
    /** Appends a decimal number at the end of this string. */
    String& operator<< (const long number) throw();
    /** Appends a decimal number at the end of this string. */
    String& operator<< (const unsigned long number) throw();
    /** Appends a decimal number at the end of this string. */
    String& operator<< (const float number) throw();
    /** Appends a decimal number at the end of this string. */
    String& operator<< (const double number) throw();

    //==============================================================================
    // Comparison methods..

    /** Returns true if the string contains no characters.

        Note that there's also an isNotEmpty() method to help write readable code.

        @see containsNonWhitespaceChars()
    */
    inline bool isEmpty() const throw()                     { return text->text[0] == 0; }

    /** Returns true if the string contains at least one character.

        Note that there's also an isEmpty() method to help write readable code.

        @see containsNonWhitespaceChars()
    */
    inline bool isNotEmpty() const throw()                  { return text->text[0] != 0; }

    /** Case-sensitive comparison with another string. */
    bool operator== (const String& other) const throw();
    /** Case-sensitive comparison with another string. */
    bool operator== (const tchar* const other) const throw();

    /** Case-sensitive comparison with another string. */
    bool operator!= (const String& other) const throw();
    /** Case-sensitive comparison with another string. */
    bool operator!= (const tchar* const other) const throw();

    /** Case-insensitive comparison with another string. */
    bool equalsIgnoreCase (const String& other) const throw();
    /** Case-insensitive comparison with another string. */
    bool equalsIgnoreCase (const tchar* const other) const throw();

    /** Case-sensitive comparison with another string. */
    bool operator> (const String& other) const throw();
    /** Case-sensitive comparison with another string. */
    bool operator< (const tchar* const other) const throw();

    /** Case-sensitive comparison with another string. */
    bool operator>= (const String& other) const throw();
    /** Case-sensitive comparison with another string. */
    bool operator<= (const tchar* const other) const throw();

    /** Case-sensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string
                     comes before the other one alphabetically, or positive if it
                     comes after it.
    */
    int compare (const tchar* const other) const throw();

    /** Case-insensitive comparison with another string.
        @returns     0 if the two strings are identical; negative if this string
                     comes before the other one alphabetically, or positive if it
                     comes after it.
    */
    int compareIgnoreCase (const tchar* const other) const throw();

    /** Lexicographic comparison with another string.

        The comparison used here is case-insensitive and ignores leading non-alphanumeric
        characters, making it good for sorting human-readable strings.

        @returns     0 if the two strings are identical; negative if this string
                     comes before the other one alphabetically, or positive if it
                     comes after it.
    */
    int compareLexicographically (const tchar* const other) const throw();

    /** Tests whether the string begins with another string.

        Uses a case-sensitive comparison.
    */
    bool startsWith (const tchar* const text) const throw();

    /** Tests whether the string begins with a particular character.

        Uses a case-sensitive comparison.
    */
    bool startsWithChar (const tchar character) const throw();

    /** Tests whether the string begins with another string.

        Uses a case-insensitive comparison.
    */
    bool startsWithIgnoreCase (const tchar* const text) const throw();

    /** Tests whether the string ends with another string.

        Uses a case-sensitive comparison.
    */
    bool endsWith (const tchar* const text) const throw();

    /** Tests whether the string ends with a particular character.

        Uses a case-sensitive comparison.
    */
    bool endsWithChar (const tchar character) const throw();

    /** Tests whether the string ends with another string.

        Uses a case-insensitive comparison.
    */
    bool endsWithIgnoreCase (const tchar* const text) const throw();

    /** Tests whether the string contains another substring.

        Uses a case-sensitive comparison.
    */
    bool contains (const tchar* const text) const throw();

    /** Tests whether the string contains a particular character.

        Uses a case-sensitive comparison.
    */
    bool containsChar (const tchar character) const throw();

    /** Tests whether the string contains another substring.

        Uses a case-insensitive comparison.
    */
    bool containsIgnoreCase (const tchar* const text) const throw();

    /** Tests whether the string contains another substring as a distict word.

        @returns    true if the string contains this word, surrounded by
                    non-alphanumeric characters
        @see indexOfWholeWord, containsWholeWordIgnoreCase
    */
    bool containsWholeWord (const tchar* const wordToLookFor) const throw();

    /** Tests whether the string contains another substring as a distict word.

        @returns    true if the string contains this word, surrounded by
                    non-alphanumeric characters
        @see indexOfWholeWordIgnoreCase, containsWholeWord
    */
    bool containsWholeWordIgnoreCase (const tchar* const wordToLookFor) const throw();

    /** Finds an instance of another substring if it exists as a distict word.

        @returns    if the string contains this word, surrounded by non-alphanumeric characters,
                    then this will return the index of the start of the substring. If it isn't
                    found, then it will return -1
        @see indexOfWholeWordIgnoreCase, containsWholeWord
    */
    int indexOfWholeWord (const tchar* const wordToLookFor) const throw();

    /** Finds an instance of another substring if it exists as a distict word.

        @returns    if the string contains this word, surrounded by non-alphanumeric characters,
                    then this will return the index of the start of the substring. If it isn't
                    found, then it will return -1
        @see indexOfWholeWord, containsWholeWordIgnoreCase
    */
    int indexOfWholeWordIgnoreCase (const tchar* const wordToLookFor) const throw();

    /** Looks for any of a set of characters in the string.

        Uses a case-sensitive comparison.

        @returns    true if the string contains any of the characters from
                    the string that is passed in.
    */
    bool containsAnyOf (const tchar* const charactersItMightContain) const throw();

    /** Looks for a set of characters in the string.

        Uses a case-sensitive comparison.

        @returns    true if the all the characters in the string are also found in the
                    string that is passed in.
    */
    bool containsOnly (const tchar* const charactersItMightContain) const throw();

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
    bool matchesWildcard (const tchar* wildcard, const bool ignoreCase) const throw();

    //==============================================================================
    // Substring location methods..

    /** Searches for a character inside this string.

        Uses a case-sensitive comparison.

        @returns    the index of the first occurrence of the character in this
                    string, or -1 if it's not found.
    */
    int indexOfChar (const tchar characterToLookFor) const throw();

    /** Searches for a character inside this string.

        Uses a case-sensitive comparison.

        @param startIndex           the index from which the search should proceed
        @param characterToLookFor   the character to look for
        @returns            the index of the first occurrence of the character in this
                            string, or -1 if it's not found.
    */
    int indexOfChar (const int startIndex, const tchar characterToLookFor) const throw();

    /** Returns the index of the first character that matches one of the characters
        passed-in to this method.

        This scans the string, beginning from the startIndex supplied, and if it finds
        a character that appears in the string charactersToLookFor, it returns its index.

        If none of these characters are found, it returns -1.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see indexOfChar, lastIndexOfAnyOf
    */
    int indexOfAnyOf (const tchar* const charactersToLookFor,
                      const int startIndex = 0,
                      const bool ignoreCase = false) const throw();

    /** Searches for a substring within this string.

        Uses a case-sensitive comparison.

        @returns    the index of the first occurrence of this substring, or -1 if it's not found.
    */
    int indexOf (const tchar* const text) const throw();

    /** Searches for a substring within this string.

        Uses a case-sensitive comparison.

        @param startIndex       the index from which the search should proceed
        @param textToLookFor    the string to search for
        @returns                the index of the first occurrence of this substring, or -1 if it's not found.
    */
    int indexOf (const int startIndex,
                 const tchar* const textToLookFor) const throw();

    /** Searches for a substring within this string.

        Uses a case-insensitive comparison.

        @returns    the index of the first occurrence of this substring, or -1 if it's not found.
    */
    int indexOfIgnoreCase (const tchar* const textToLookFor) const throw();

    /** Searches for a substring within this string.

        Uses a case-insensitive comparison.

        @param startIndex       the index from which the search should proceed
        @param textToLookFor    the string to search for
        @returns                the index of the first occurrence of this substring, or -1 if it's not found.
    */
    int indexOfIgnoreCase (const int startIndex,
                           const tchar* const textToLookFor) const throw();

    /** Searches for a character inside this string (working backwards from the end of the string).

        Uses a case-sensitive comparison.

        @returns            the index of the last occurrence of the character in this
                            string, or -1 if it's not found.
    */
    int lastIndexOfChar (const tchar character) const throw();

    /** Searches for a substring inside this string (working backwards from the end of the string).

        Uses a case-sensitive comparison.

        @returns            the index of the start of the last occurrence of the
                            substring within this string, or -1 if it's not found.
    */
    int lastIndexOf (const tchar* const textToLookFor) const throw();

    /** Searches for a substring inside this string (working backwards from the end of the string).

        Uses a case-insensitive comparison.

        @returns            the index of the start of the last occurrence of the
                            substring within this string, or -1 if it's not found.
    */
    int lastIndexOfIgnoreCase (const tchar* const textToLookFor) const throw();

    /** Returns the index of the last character in this string that matches one of the
        characters passed-in to this method.

        This scans the string backwards, starting from its end, and if it finds
        a character that appears in the string charactersToLookFor, it returns its index.

        If none of these characters are found, it returns -1.

        If ignoreCase is true, the comparison will be case-insensitive.

        @see lastIndexOf, indexOfAnyOf
    */
    int lastIndexOfAnyOf (const tchar* const charactersToLookFor,
                          const bool ignoreCase = false) const throw();


    //==============================================================================
    // Substring extraction and manipulation methods..

    /** Returns the character at this index in the string.

        No checks are made to see if the index is within a valid range, so be careful!
    */
    inline const tchar& operator[] (const int index) const throw()  { jassert (((unsigned int) index) <= (unsigned int) length()); return text->text [index]; }

    /** Returns a character from the string such that it can also be altered.

        This can be used as a way of easily changing characters in the string.

        Note that the index passed-in is not checked to see whether it's in-range, so
        be careful when using this.
    */
    tchar& operator[] (const int index) throw();

    /** Returns the final character of the string.

        If the string is empty this will return 0.
    */
    tchar getLastCharacter() const throw();

    //==============================================================================
    /** Returns a subsection of the string.

        If the range specified is beyond the limits of the string, as much as
        possible is returned.

        @param startIndex   the index of the start of the substring needed
        @param endIndex     all characters from startIndex up to (but not including)
                            this index are returned
        @see fromFirstOccurrenceOf, dropLastCharacters, getLastCharacters, upToFirstOccurrenceOf
    */
    const String substring (int startIndex,
                            int endIndex) const throw();

    /** Returns a section of the string, starting from a given position.

        @param startIndex   the first character to include. If this is beyond the end
                            of the string, an empty string is returned. If it is zero or
                            less, the whole string is returned.
        @returns            the substring from startIndex up to the end of the string
        @see dropLastCharacters, getLastCharacters, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf
    */
    const String substring (const int startIndex) const throw();

    /** Returns a version of this string with a number of characters removed
        from the end.

        @param numberToDrop     the number of characters to drop from the end of the
                                string. If this is greater than the length of the string,
                                an empty string will be returned. If zero or less, the
                                original string will be returned.
        @see substring, fromFirstOccurrenceOf, upToFirstOccurrenceOf, fromLastOccurrenceOf, getLastCharacter
    */
    const String dropLastCharacters (const int numberToDrop) const throw();

    /** Returns a number of characters from the end of the string.

        This returns the last numCharacters characters from the end of the string. If the
        string is shorter than numCharacters, the whole string is returned.

        @see substring, dropLastCharacters, getLastCharacter
    */
    const String getLastCharacters (const int numCharacters) const throw();

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
    const String fromFirstOccurrenceOf (const tchar* const substringToStartFrom,
                                        const bool includeSubStringInResult,
                                        const bool ignoreCase) const throw();

    /** Returns a section of the string starting from the last occurrence of a given substring.

        Similar to fromFirstOccurrenceOf(), but using the last occurrence of the substring, and
        unlike fromFirstOccurrenceOf(), if the substring isn't found, this method will
        return the whole of the original string.

        @see fromFirstOccurrenceOf, upToLastOccurrenceOf
    */
    const String fromLastOccurrenceOf (const tchar* const substringToFind,
                                       const bool includeSubStringInResult,
                                       const bool ignoreCase) const throw();

    /** Returns the start of this string, up to the first occurrence of a substring.

        This will search for the first occurrence of a given substring, and then
        return a copy of the string, up to the position of this substring,
        optionally including or excluding the substring itself in the result.

        e.g. for the string "123456", upTo ("34", false) would return "12", and
                                      upTo ("34", true) would return "1234".

        If the substring isn't found, this will return the whole of the original string.

        @see upToLastOccurrenceOf, fromFirstOccurrenceOf
    */
    const String upToFirstOccurrenceOf (const tchar* const substringToEndWith,
                                        const bool includeSubStringInResult,
                                        const bool ignoreCase) const throw();

    /** Returns the start of this string, up to the last occurrence of a substring.

        Similar to upToFirstOccurrenceOf(), but this finds the last occurrence rather than the first.
        If the substring isn't found, this will return an empty string.

        @see upToFirstOccurrenceOf, fromFirstOccurrenceOf
    */
    const String upToLastOccurrenceOf (const tchar* substringToFind,
                                       const bool includeSubStringInResult,
                                       const bool ignoreCase) const throw();

    //==============================================================================
    /** Returns a copy of this string with any whitespace characters removed from the start and end. */
    const String trim() const throw();
    /** Returns a copy of this string with any whitespace characters removed from the start. */
    const String trimStart() const throw();
    /** Returns a copy of this string with any whitespace characters removed from the end. */
    const String trimEnd() const throw();

    /** Returns a copy of this string, having removed a specified set of characters from its start.
        Characters are removed from the start of the string until it finds one that is not in the
        specified set, and then it stops.
        @param charactersToTrim     the set of characters to remove. This must not be null.
        @see trim, trimStart, trimCharactersAtEnd
    */
    const String trimCharactersAtStart (const tchar* charactersToTrim) const throw();

    /** Returns a copy of this string, having removed a specified set of characters from its end.
        Characters are removed from the end of the string until it finds one that is not in the
        specified set, and then it stops.
        @param charactersToTrim     the set of characters to remove. This must not be null.
        @see trim, trimEnd, trimCharactersAtStart
    */
    const String trimCharactersAtEnd (const tchar* charactersToTrim) const throw();

    //==============================================================================
    /** Returns an upper-case version of this string. */
    const String toUpperCase() const throw();

    /** Returns an lower-case version of this string. */
    const String toLowerCase() const throw();

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
                                 const tchar* const stringToInsert) const throw();

    /** Replaces all occurrences of a substring with another string.

        Returns a copy of this string, with any occurrences of stringToReplace
        swapped for stringToInsertInstead.

        Note that this is a const method, and won't alter the string itself.
    */
    const String replace (const tchar* const stringToReplace,
                          const tchar* const stringToInsertInstead,
                          const bool ignoreCase = false) const throw();

    /** Returns a string with all occurrences of a character replaced with a different one. */
    const String replaceCharacter (const tchar characterToReplace,
                                   const tchar characterToInsertInstead) const throw();

    /** Replaces a set of characters with another set.

        Returns a string in which each character from charactersToReplace has been replaced
        by the character at the equivalent position in newCharacters (so the two strings
        passed in must be the same length).

        e.g. translate ("abc", "def") replaces 'a' with 'd', 'b' with 'e', etc.

        Note that this is a const method, and won't affect the string itself.
    */
    const String replaceCharacters (const String& charactersToReplace,
                                    const tchar* const charactersToInsertInstead) const throw();

    /** Returns a version of this string that only retains a fixed set of characters.

        This will return a copy of this string, omitting any characters which are not
        found in the string passed-in.

        e.g. for "1122334455", retainCharacters ("432") would return "223344"

        Note that this is a const method, and won't alter the string itself.
    */
    const String retainCharacters (const tchar* const charactersToRetain) const throw();

    /** Returns a version of this string with a set of characters removed.

        This will return a copy of this string, omitting any characters which are
        found in the string passed-in.

        e.g. for "1122334455", removeCharacters ("432") would return "1155"

        Note that this is a const method, and won't alter the string itself.
    */
    const String removeCharacters (const tchar* const charactersToRemove) const throw();

    /** Returns a section from the start of the string that only contains a certain set of characters.

        This returns the leftmost section of the string, up to (and not including) the
        first character that doesn't appear in the string passed in.
    */
    const String initialSectionContainingOnly (const tchar* const permittedCharacters) const throw();

    /** Returns a section from the start of the string that only contains a certain set of characters.

        This returns the leftmost section of the string, up to (and not including) the
        first character that occurs in the string passed in.
    */
    const String initialSectionNotContaining (const tchar* const charactersToStopAt) const throw();

    //==============================================================================
    /** Checks whether the string might be in quotation marks.

        @returns    true if the string begins with a quote character (either a double or single quote).
                    It is also true if there is whitespace before the quote, but it doesn't check the end of the string.
        @see unquoted, quoted
    */
    bool isQuotedString() const throw();

    /** Removes quotation marks from around the string, (if there are any).

        Returns a copy of this string with any quotes removed from its ends. Quotes that aren't
        at the ends of the string are not affected. If there aren't any quotes, the original string
        is returned.

        Note that this is a const method, and won't alter the string itself.

        @see isQuotedString, quoted
    */
    const String unquoted() const throw();

    /** Adds quotation marks around a string.

        This will return a copy of the string with a quote at the start and end, (but won't
        add the quote if there's already one there, so it's safe to call this on strings that
        may already have quotes around them).

        Note that this is a const method, and won't alter the string itself.

        @param quoteCharacter   the character to add at the start and end
        @see isQuotedString, unquoted
    */
    const String quoted (const tchar quoteCharacter = JUCE_T('"')) const throw();


    //==============================================================================
    /** Writes text into this string, using printf style-arguments.

        This will replace the contents of the string with the output of this
        formatted printf.

        Note that using the %s token with a juce string is probably a bad idea, as
        this may expect differect encodings on different platforms.

        @see formatted
    */
    void printf (const tchar* const format, ...) throw();

    /** Returns a string, created using arguments in the style of printf.

        This will return a string which is the result of a sprintf using the
        arguments passed-in.

        Note that using the %s token with a juce string is probably a bad idea, as
        this may expect differect encodings on different platforms.

        @see printf, vprintf
    */
    static const String formatted (const tchar* const format, ...) throw();

    /** Writes text into this string, using a printf style, but taking a va_list argument.

        This will replace the contents of the string with the output of this
        formatted printf. Used by other methods, this is public in case it's
        useful for other purposes where you want to pass a va_list through directly.

        Note that using the %s token with a juce string is probably a bad idea, as
        this may expect differect encodings on different platforms.

        @see printf, formatted
    */
    void vprintf (const tchar* const format, va_list& args) throw();

    //==============================================================================
    /** Creates a string which is a version of a string repeated and joined together.

        @param stringToRepeat         the string to repeat
        @param numberOfTimesToRepeat  how many times to repeat it
    */
    static const String repeatedString (const tchar* const stringToRepeat,
                                        int numberOfTimesToRepeat) throw();

    /** Creates a string from data in an unknown format.

        This looks at some binary data and tries to guess whether it's Unicode
        or 8-bit characters, then returns a string that represents it correctly.

        Should be able to handle Unicode endianness correctly, by looking at
        the first two bytes.
    */
    static const String createStringFromData (const void* const data,
                                              const int size) throw();

    //==============================================================================
    // Numeric conversions..

    /** Creates a string containing this signed 32-bit integer as a decimal number.

        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (const int decimalInteger) throw();

    /** Creates a string containing this unsigned 32-bit integer as a decimal number.

        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (const unsigned int decimalInteger) throw();

    /** Creates a string containing this signed 16-bit integer as a decimal number.

        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (const short decimalInteger) throw();

    /** Creates a string containing this unsigned 16-bit integer as a decimal number.

        @see getIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (const unsigned short decimalInteger) throw();

    /** Creates a string containing this signed 64-bit integer as a decimal number.

        @see getLargeIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (const int64 largeIntegerValue) throw();

    /** Creates a string containing this unsigned 64-bit integer as a decimal number.

        @see getLargeIntValue, getFloatValue, getDoubleValue, toHexString
    */
    explicit String (const uint64 largeIntegerValue) throw();

    /** Creates a string representing this floating-point number.

        @param floatValue               the value to convert to a string
        @param numberOfDecimalPlaces    if this is > 0, it will format the number using that many
                                        decimal places, and will not use exponent notation. If 0 or
                                        less, it will use exponent notation if necessary.
        @see getDoubleValue, getIntValue
    */
    explicit String (const float floatValue,
                     const int numberOfDecimalPlaces = 0) throw();

    /** Creates a string representing this floating-point number.

        @param doubleValue              the value to convert to a string
        @param numberOfDecimalPlaces    if this is > 0, it will format the number using that many
                                        decimal places, and will not use exponent notation. If 0 or
                                        less, it will use exponent notation if necessary.

        @see getFloatValue, getIntValue
    */
    explicit String (const double doubleValue,
                     const int numberOfDecimalPlaces = 0) throw();

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
    static const String toHexString (const int number) throw();

    /** Creates a string representing this 64-bit value in hexadecimal. */
    static const String toHexString (const int64 number) throw();

    /** Creates a string representing this 16-bit value in hexadecimal. */
    static const String toHexString (const short number) throw();

    /** Creates a string containing a hex dump of a block of binary data.

        @param data         the binary data to use as input
        @param size         how many bytes of data to use
        @param groupSize    how many bytes are grouped together before inserting a
                            space into the output. e.g. group size 0 has no spaces,
                            group size 1 looks like: "be a1 c2 ff", group size 2 looks
                            like "bea1 c2ff".
    */
    static const String toHexString (const unsigned char* data,
                                     const int size,
                                     const int groupSize = 1) throw();

    //==============================================================================
    // Casting to character arrays..

#if JUCE_STRINGS_ARE_UNICODE
    /** Returns a version of this string using the default 8-bit system encoding.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.
    */
    operator const char*() const throw();

    /** Returns a unicode version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.
    */
    inline operator const juce_wchar*() const throw()   { return text->text; }
#else
    /** Returns a version of this string using the default 8-bit system encoding.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.
    */
    inline operator const char*() const throw()         { return text->text; }

    /** Returns a unicode version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.
    */
    operator const juce_wchar*() const throw();
#endif

    /** Copies the string to a buffer.

        @param destBuffer       the place to copy it to
        @param maxCharsToCopy   the maximum number of characters to copy to the buffer,
                                not including the tailing zero, so this shouldn't be
                                larger than the size of your destination buffer - 1
    */
    void copyToBuffer (char* const destBuffer,
                       const int maxCharsToCopy) const throw();

    /** Copies the string to a unicode buffer.

        @param destBuffer       the place to copy it to
        @param maxCharsToCopy   the maximum number of characters to copy to the buffer,
                                not including the tailing zero, so this shouldn't be
                                larger than the size of your destination buffer - 1
    */
    void copyToBuffer (juce_wchar* const destBuffer,
                       const int maxCharsToCopy) const throw();

    //==============================================================================
    /** Copies the string to a buffer as UTF-8 characters.

        Returns the number of bytes copied to the buffer, including the terminating null
        character.

        @param destBuffer       the place to copy it to; if this is a null pointer,
                                the method just returns the number of bytes required
                                (including the terminating null character).
        @param maxBufferSizeBytes  the size of the destination buffer, in bytes. If the
                                string won't fit, it'll put in as many as it can while
                                still allowing for a terminating null char at the end,
                                and will return the number of bytes that were actually
                                used. If this value is < 0, no limit is used.
    */
    int copyToUTF8 (uint8* const destBuffer, const int maxBufferSizeBytes = 0x7fffffff) const throw();

    /** Returns a pointer to a UTF-8 version of this string.

        Because it returns a reference to the string's internal data, the pointer
        that is returned must not be stored anywhere, as it can be deleted whenever the
        string changes.
    */
    const char* toUTF8() const throw();

    /** Creates a String from a UTF-8 encoded buffer.

        If the size is < 0, it'll keep reading until it hits a zero.
    */
    static const String fromUTF8 (const uint8* const utf8buffer,
                                  int bufferSizeBytes = -1) throw();

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
    void preallocateStorage (const size_t numCharsNeeded) throw();

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

        Concatenator (const Concatenator&);
        const Concatenator& operator= (const Concatenator&);
    };

    //==============================================================================
    juce_UseDebuggingNewOperator // (adds debugging info to find leaked objects)

private:
    //==============================================================================
    struct InternalRefCountedStringHolder
    {
        int refCount;
        int allocatedNumChars;

#if JUCE_STRINGS_ARE_UNICODE
          wchar_t text[1];
#else
          char text[1];
#endif
    };

    InternalRefCountedStringHolder* text;
    static InternalRefCountedStringHolder emptyString;

    //==============================================================================
    // internal constructor that preallocates a certain amount of memory
    String (const int numChars, const int dummyVariable) throw();

    void deleteInternal() throw();
    void createInternal (const int numChars) throw();
    void createInternal (const tchar* const text, const tchar* const textEnd) throw();
    void appendInternal (const tchar* const text, const int numExtraChars) throw();
    void doubleToStringWithDecPlaces (double n, int numDecPlaces) throw();
    void dupeInternalIfMultiplyReferenced() throw();
};

//==============================================================================
/** Global operator to allow a String to be appended to a string literal.

    This allows the use of expressions such as "abc" + String (x)

    @see String
 */
const String JUCE_PUBLIC_FUNCTION   operator+ (const char* const string1,
                                               const String& string2) throw();


//==============================================================================
/** Global operator to allow a String to be appended to a string literal.

    This allows the use of expressions such as "abc" + String (x)

    @see String
 */
const String JUCE_PUBLIC_FUNCTION   operator+ (const juce_wchar* const string1,
                                               const String& string2) throw();


#endif   // __JUCE_STRING_JUCEHEADER__
