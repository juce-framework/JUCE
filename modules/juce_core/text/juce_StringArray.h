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

#ifndef __JUCE_STRINGARRAY_JUCEHEADER__
#define __JUCE_STRINGARRAY_JUCEHEADER__

#include "juce_String.h"
#include "../containers/juce_Array.h"


//==============================================================================
/**
    A special array for holding a list of strings.

    @see String, StringPairArray
*/
class JUCE_API  StringArray
{
public:
    //==============================================================================
    /** Creates an empty string array */
    StringArray() noexcept;

    /** Creates a copy of another string array */
    StringArray (const StringArray& other);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    StringArray (StringArray&& other) noexcept;
   #endif

    /** Creates an array containing a single string. */
    explicit StringArray (const String& firstValue);

    /** Creates a copy of an array of string literals.
        @param strings          an array of strings to add. Null pointers in the array will be
                                treated as empty strings
        @param numberOfStrings  how many items there are in the array
    */
    StringArray (const char* const* strings, int numberOfStrings);

    /** Creates a copy of a null-terminated array of string literals.

        Each item from the array passed-in is added, until it encounters a null pointer,
        at which point it stops.
    */
    explicit StringArray (const char* const* strings);

    /** Creates a copy of a null-terminated array of string literals.
        Each item from the array passed-in is added, until it encounters a null pointer,
        at which point it stops.
    */
    explicit StringArray (const wchar_t* const* strings);

    /** Creates a copy of an array of string literals.
        @param strings          an array of strings to add. Null pointers in the array will be
                                treated as empty strings
        @param numberOfStrings  how many items there are in the array
    */
    StringArray (const wchar_t* const* strings, int numberOfStrings);

    /** Destructor. */
    ~StringArray();

    /** Copies the contents of another string array into this one */
    StringArray& operator= (const StringArray& other);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    StringArray& operator= (StringArray&& other) noexcept;
   #endif

    /** Swaps the contents of this and another StringArray. */
    void swapWith (StringArray& other) noexcept;

    //==============================================================================
    /** Compares two arrays.
        Comparisons are case-sensitive.
        @returns    true only if the other array contains exactly the same strings in the same order
    */
    bool operator== (const StringArray& other) const noexcept;

    /** Compares two arrays.
        Comparisons are case-sensitive.
        @returns    false if the other array contains exactly the same strings in the same order
    */
    bool operator!= (const StringArray& other) const noexcept;

    //==============================================================================
    /** Returns the number of strings in the array */
    inline int size() const noexcept                                    { return strings.size(); };

    /** Returns one of the strings from the array.

        If the index is out-of-range, an empty string is returned.

        Obviously the reference returned shouldn't be stored for later use, as the
        string it refers to may disappear when the array changes.
    */
    const String& operator[] (int index) const noexcept;

    /** Returns a reference to one of the strings in the array.
        This lets you modify a string in-place in the array, but you must be sure that
        the index is in-range.
    */
    String& getReference (int index) noexcept;

    /** Returns a pointer to the first String in the array.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    inline String* begin() const noexcept
    {
        return strings.begin();
    }

    /** Returns a pointer to the String which follows the last element in the array.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    inline String* end() const noexcept
    {
        return strings.end();
    }

    /** Searches for a string in the array.

        The comparison will be case-insensitive if the ignoreCase parameter is true.

        @returns    true if the string is found inside the array
    */
    bool contains (const String& stringToLookFor,
                   bool ignoreCase = false) const;

    /** Searches for a string in the array.

        The comparison will be case-insensitive if the ignoreCase parameter is true.

        @param stringToLookFor  the string to try to find
        @param ignoreCase       whether the comparison should be case-insensitive
        @param startIndex       the first index to start searching from
        @returns                the index of the first occurrence of the string in this array,
                                or -1 if it isn't found.
    */
    int indexOf (const String& stringToLookFor,
                 bool ignoreCase = false,
                 int startIndex = 0) const;

    //==============================================================================
    /** Appends a string at the end of the array. */
    void add (const String& stringToAdd);

    /** Inserts a string into the array.

        This will insert a string into the array at the given index, moving
        up the other elements to make room for it.
        If the index is less than zero or greater than the size of the array,
        the new string will be added to the end of the array.
    */
    void insert (int index, const String& stringToAdd);

    /** Adds a string to the array as long as it's not already in there.

        The search can optionally be case-insensitive.
    */
    void addIfNotAlreadyThere (const String& stringToAdd, bool ignoreCase = false);

    /** Replaces one of the strings in the array with another one.

        If the index is higher than the array's size, the new string will be
        added to the end of the array; if it's less than zero nothing happens.
    */
    void set (int index, const String& newString);

    /** Appends some strings from another array to the end of this one.

        @param other                the array to add
        @param startIndex           the first element of the other array to add
        @param numElementsToAdd     the maximum number of elements to add (if this is
                                    less than zero, they are all added)
    */
    void addArray (const StringArray& other,
                   int startIndex = 0,
                   int numElementsToAdd = -1);

    /** Breaks up a string into tokens and adds them to this array.

        This will tokenise the given string using whitespace characters as the
        token delimiters, and will add these tokens to the end of the array.

        @returns    the number of tokens added
    */
    int addTokens (const String& stringToTokenise,
                   bool preserveQuotedStrings);

    /** Breaks up a string into tokens and adds them to this array.

        This will tokenise the given string (using the string passed in to define the
        token delimiters), and will add these tokens to the end of the array.

        @param stringToTokenise     the string to tokenise
        @param breakCharacters      a string of characters, any of which will be considered
                                    to be a token delimiter.
        @param quoteCharacters      if this string isn't empty, it defines a set of characters
                                    which are treated as quotes. Any text occurring
                                    between quotes is not broken up into tokens.
        @returns    the number of tokens added
    */
    int addTokens (const String& stringToTokenise,
                   const String& breakCharacters,
                   const String& quoteCharacters);

    /** Breaks up a string into lines and adds them to this array.

        This breaks a string down into lines separated by \\n or \\r\\n, and adds each line
        to the array. Line-break characters are omitted from the strings that are added to
        the array.
    */
    int addLines (const String& stringToBreakUp);

    //==============================================================================
    /** Removes all elements from the array. */
    void clear();

    /** Removes a string from the array.

        If the index is out-of-range, no action will be taken.
    */
    void remove (int index);

    /** Finds a string in the array and removes it.

        This will remove the first occurrence of the given string from the array. The
        comparison may be case-insensitive depending on the ignoreCase parameter.
    */
    void removeString (const String& stringToRemove,
                       bool ignoreCase = false);

    /** Removes a range of elements from the array.

        This will remove a set of elements, starting from the given index,
        and move subsequent elements down to close the gap.

        If the range extends beyond the bounds of the array, it will
        be safely clipped to the size of the array.

        @param startIndex       the index of the first element to remove
        @param numberToRemove   how many elements should be removed
    */
    void removeRange (int startIndex, int numberToRemove);

    /** Removes any duplicated elements from the array.

        If any string appears in the array more than once, only the first occurrence of
        it will be retained.

        @param ignoreCase   whether to use a case-insensitive comparison
    */
    void removeDuplicates (bool ignoreCase);

    /** Removes empty strings from the array.

        @param removeWhitespaceStrings  if true, strings that only contain whitespace
                                        characters will also be removed
    */
    void removeEmptyStrings (bool removeWhitespaceStrings = true);

    /** Moves one of the strings to a different position.

        This will move the string to a specified index, shuffling along
        any intervening elements as required.

        So for example, if you have the array { 0, 1, 2, 3, 4, 5 } then calling
        move (2, 4) would result in { 0, 1, 3, 4, 2, 5 }.

        @param currentIndex     the index of the value to be moved. If this isn't a
                                valid index, then nothing will be done
        @param newIndex         the index at which you'd like this value to end up. If this
                                is less than zero, the value will be moved to the end
                                of the array
    */
    void move (int currentIndex, int newIndex) noexcept;

    /** Deletes any whitespace characters from the starts and ends of all the strings. */
    void trim();

    /** Adds numbers to the strings in the array, to make each string unique.

        This will add numbers to the ends of groups of similar strings.
        e.g. if there are two "moose" strings, they will become "moose (1)" and "moose (2)"

        @param ignoreCaseWhenComparing      whether the comparison used is case-insensitive
        @param appendNumberToFirstInstance  whether the first of a group of similar strings
                                            also has a number appended to it.
        @param preNumberString              when adding a number, this string is added before the number.
                                            If you pass 0, a default string will be used, which adds
                                            brackets around the number.
        @param postNumberString             this string is appended after any numbers that are added.
                                            If you pass 0, a default string will be used, which adds
                                            brackets around the number.
    */
    void appendNumbersToDuplicates (bool ignoreCaseWhenComparing,
                                    bool appendNumberToFirstInstance,
                                    CharPointer_UTF8 preNumberString = CharPointer_UTF8 (nullptr),
                                    CharPointer_UTF8 postNumberString = CharPointer_UTF8 (nullptr));

    //==============================================================================
    /** Joins the strings in the array together into one string.

        This will join a range of elements from the array into a string, separating
        them with a given string.

        e.g. joinIntoString (",") will turn an array of "a" "b" and "c" into "a,b,c".

        @param separatorString      the string to insert between all the strings
        @param startIndex           the first element to join
        @param numberOfElements     how many elements to join together. If this is less
                                    than zero, all available elements will be used.
    */
    String joinIntoString (const String& separatorString,
                           int startIndex = 0,
                           int numberOfElements = -1) const;

    //==============================================================================
    /** Sorts the array into alphabetical order.

        @param ignoreCase       if true, the comparisons used will be case-sensitive.
    */
    void sort (bool ignoreCase);

    //==============================================================================
    /** Reduces the amount of storage being used by the array.

        Arrays typically allocate slightly more storage than they need, and after
        removing elements, they may have quite a lot of unused space allocated.
        This method will reduce the amount of allocated storage to a minimum.
    */
    void minimiseStorageOverheads();


private:
    //==============================================================================
    Array <String> strings;

    JUCE_LEAK_DETECTOR (StringArray)
};


#endif   // __JUCE_STRINGARRAY_JUCEHEADER__
