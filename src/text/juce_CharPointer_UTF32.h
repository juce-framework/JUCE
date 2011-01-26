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

#ifndef __JUCE_CHARPOINTER_UTF32_JUCEHEADER__
#define __JUCE_CHARPOINTER_UTF32_JUCEHEADER__


//==============================================================================
/**
    Wraps a pointer to a null-terminated UTF-32 character string, and provides
    various methods to operate on the data.
    @see CharPointer_UTF8, CharPointer_UTF16
*/
class CharPointer_UTF32
{
public:
    typedef juce_wchar CharType;

    inline CharPointer_UTF32 (const CharType* const rawPointer) throw()
        : data (const_cast <CharType*> (rawPointer))
    {
    }

    inline CharPointer_UTF32 (const CharPointer_UTF32& other) throw()
        : data (other.data)
    {
    }

    inline CharPointer_UTF32& operator= (const CharPointer_UTF32& other) throw()
    {
        data = other.data;
        return *this;
    }

    /** Returns the address that this pointer is pointing to. */
    inline CharType* getAddress() const throw()     { return data; }

    /** Returns true if this pointer is pointing to a null character. */
    inline bool isEmpty() const throw()             { return *data == 0; }

    /** Returns the unicode character that this pointer is pointing to. */
    inline juce_wchar operator*() const throw()     { return *data; }

    /** Moves this pointer along to the next character in the string. */
    inline CharPointer_UTF32& operator++() throw()
    {
        ++data;
        return *this;
    }

    /** Moves this pointer to the previous character in the string. */
    inline CharPointer_UTF32& operator--() throw()
    {
        --data;
        return *this;
    }

    /** Returns the character that this pointer is currently pointing to, and then
        advances the pointer to point to the next character. */
    inline juce_wchar getAndAdvance() throw()   { return *data++; }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF32 operator++ (int) throw()
    {
        CharPointer_UTF32 temp (*this);
        ++*this;
        return temp;
    }

    /** Moves this pointer forwards by the specified number of characters. */
    inline void operator+= (int numToSkip) throw()
    {
        data += numToSkip;
    }

    inline void operator-= (int numToSkip) throw()
    {
        data -= numToSkip;
    }

    /** Returns the character at a given character index from the start of the string. */
    inline juce_wchar operator[] (int characterIndex) const throw()
    {
        return data [characterIndex];
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_UTF32 operator+ (int numToSkip) const throw()
    {
        return CharPointer_UTF32 (data + numToSkip);
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_UTF32 operator- (int numToSkip) const throw()
    {
        return CharPointer_UTF32 (data - numToSkip);
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    inline void write (const juce_wchar charToWrite) throw()
    {
        *data++ = charToWrite;
    }

    /** Returns the number of characters in this string. */
    size_t length() const throw()
    {
       #if JUCE_ANDROID
        size_t n = 0;
        while (data[n] == 0)
            ++n;
        return n;
       #else
        return wcslen (data);
       #endif
    }

    /** Returns the number of bytes that are used to represent this string.
        This includes the terminating null character.
    */
    size_t sizeInBytes() const throw()
    {
        return sizeof (CharType) * (length() + 1);
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static inline size_t getBytesRequiredFor (const juce_wchar) throw()
    {
        return sizeof (CharType);
    }

    /** Returns the number of bytes that would be needed to represent the given
        string in this encoding format.
        The value returned does NOT include the terminating null character.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (const CharPointer& text) throw()
    {
        return sizeof (CharType) * text.length();
    }

    /** Returns a pointer to the null character that terminates this string. */
    CharPointer_UTF32 findTerminatingNull() const throw()
    {
        return CharPointer_UTF32 (data + length());
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    template <typename CharPointer>
    void copyAndAdvance (const CharPointer& src) throw()
    {
        CharacterFunctions::copyAndAdvance (*this, src);
    }

   #if ! JUCE_ANDROID
    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    void copyAndAdvance (const CharPointer_UTF32& src) throw()
    {
        data = wcscpy (data, src.data);
    }
   #endif

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxBytes parameter specifies the maximum number of bytes that can be written
        to the destination buffer before stopping.
    */
    template <typename CharPointer>
    int copyAndAdvanceUpToBytes (const CharPointer& src, int maxBytes) throw()
    {
        return CharacterFunctions::copyAndAdvanceUpToBytes (*this, src, maxBytes);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxChars parameter specifies the maximum number of characters that can be
        written to the destination buffer before stopping (including the terminating null).
    */
    template <typename CharPointer>
    void copyAndAdvanceUpToNumChars (const CharPointer& src, int maxChars) throw()
    {
        CharacterFunctions::copyAndAdvanceUpToNumChars (*this, src, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compare (const CharPointer& other) const throw()
    {
        return CharacterFunctions::compare (*this, other);
    }

   #if ! JUCE_ANDROID
    /** Compares this string with another one. */
    int compare (const CharPointer_UTF32& other) const throw()
    {
        return wcscmp (data, other.data);
    }
   #endif

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareUpTo (const CharPointer& other, int maxChars) const throw()
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compareIgnoreCase (const CharPointer& other) const
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareIgnoreCaseUpTo (const CharPointer& other, int maxChars) const throw()
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

    /** Returns the character index of a substring, or -1 if it isn't found. */
    template <typename CharPointer>
    int indexOf (const CharPointer& stringToFind) const throw()
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const juce_wchar charToFind) const throw()
    {
        int i = 0;

        while (data[i] != 0)
        {
            if (data[i] == charToFind)
                return i;

            ++i;
        }

        return -1;
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const juce_wchar charToFind, const bool ignoreCase) const throw()
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          : CharacterFunctions::indexOfChar (*this, charToFind);
    }

   #if JUCE_WINDOWS && ! DOXYGEN
    int compareIgnoreCase (const CharPointer_UTF32& other) const throw()
    {
        return _wcsicmp (data, other.data);
    }

    int compareIgnoreCaseUpTo (const CharPointer_UTF32& other, int maxChars) const throw()
    {
        return _wcsnicmp (data, other.data, maxChars);
    }

    int indexOf (const CharPointer_UTF32& stringToFind) const throw()
    {
        const CharType* const t = wcsstr (data, stringToFind.getAddress());
        return t == 0 ? -1 : (t - data);
    }
   #endif

    /** Returns true if the first character of this string is whitespace. */
    bool isWhitespace() const               { return CharacterFunctions::isWhitespace (*data) != 0; }
    /** Returns true if the first character of this string is a digit. */
    bool isDigit() const                    { return CharacterFunctions::isDigit (*data) != 0; }
    /** Returns true if the first character of this string is a letter. */
    bool isLetter() const                   { return CharacterFunctions::isLetter (*data) != 0; }
    /** Returns true if the first character of this string is a letter or digit. */
    bool isLetterOrDigit() const            { return CharacterFunctions::isLetterOrDigit (*data) != 0; }
    /** Returns true if the first character of this string is upper-case. */
    bool isUpperCase() const                { return CharacterFunctions::isUpperCase (*data) != 0; }
    /** Returns true if the first character of this string is lower-case. */
    bool isLowerCase() const                { return CharacterFunctions::isLowerCase (*data) != 0; }

    /** Returns an upper-case version of the first character of this string. */
    juce_wchar toUpperCase() const throw()  { return CharacterFunctions::toUpperCase (*data); }
    /** Returns a lower-case version of the first character of this string. */
    juce_wchar toLowerCase() const throw()  { return CharacterFunctions::toLowerCase (*data); }

    /** Parses this string as a 32-bit integer. */
    int getIntValue32() const throw()
    {
       #if JUCE_WINDOWS
        return _wtoi (data);
       #else
        return CharacterFunctions::getIntValue <int, CharPointer_UTF32> (*this);
       #endif
    }

    /** Parses this string as a 64-bit integer. */
    int64 getIntValue64() const throw()
    {
       #if JUCE_WINDOWS
        return _wtoi64 (data);
       #else
        return CharacterFunctions::getIntValue <int64, CharPointer_UTF32> (*this);
       #endif
    }

    /** Parses this string as a floating point double. */
    double getDoubleValue() const throw()   { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_UTF32 findEndOfWhitespace() const throw()    { return CharacterFunctions::findEndOfWhitespace (*this); }

private:
    CharType* data;
};


#endif   // __JUCE_CHARPOINTER_UTF32_JUCEHEADER__
