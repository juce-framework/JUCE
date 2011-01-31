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

#ifndef __JUCE_CHARPOINTER_UTF16_JUCEHEADER__
#define __JUCE_CHARPOINTER_UTF16_JUCEHEADER__


//==============================================================================
/**
    Wraps a pointer to a null-terminated UTF-16 character string, and provides
    various methods to operate on the data.
    @see CharPointer_UTF8, CharPointer_UTF32
*/
class CharPointer_UTF16
{
public:
   #if JUCE_WINDOWS && ! DOXYGEN
    typedef wchar_t CharType;
   #else
    typedef int16 CharType;
   #endif

    inline explicit CharPointer_UTF16 (const CharType* const rawPointer) throw()
        : data (const_cast <CharType*> (rawPointer))
    {
    }

    inline CharPointer_UTF16 (const CharPointer_UTF16& other) throw()
        : data (other.data)
    {
    }

    inline CharPointer_UTF16& operator= (const CharPointer_UTF16& other) throw()
    {
        data = other.data;
        return *this;
    }

    inline CharPointer_UTF16& operator= (const CharType* text) throw()
    {
        data = const_cast <CharType*> (text);
        return *this;
    }

    /** This is a pointer comparison, it doesn't compare the actual text. */
    inline bool operator== (const CharPointer_UTF16& other) const throw()
    {
        return data == other.data;
    }

    /** This is a pointer comparison, it doesn't compare the actual text. */
    inline bool operator!= (const CharPointer_UTF16& other) const throw()
    {
        return data == other.data;
    }

    /** Returns the address that this pointer is pointing to. */
    inline CharType* getAddress() const throw()         { return data; }

    /** Returns the address that this pointer is pointing to. */
    inline operator const CharType*() const throw()     { return data; }

    /** Returns true if this pointer is pointing to a null character. */
    inline bool isEmpty() const throw()                 { return *data == 0; }

    /** Returns the unicode character that this pointer is pointing to. */
    juce_wchar operator*() const throw()
    {
        uint32 n = (uint32) (uint16) *data;

        if (n >= 0xd800 && n <= 0xdfff && ((uint32) (uint16) data[1]) >= 0xdc00)
            n = 0x10000 + (((n - 0xd800) << 10) | (((uint32) (uint16) data[1]) - 0xdc00));

        return (juce_wchar) n;
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF16& operator++() throw()
    {
        const juce_wchar n = *data++;

        if (n >= 0xd800 && n <= 0xdfff && ((uint32) (uint16) *data) >= 0xdc00)
            ++data;

        return *this;
    }

    /** Returns the character that this pointer is currently pointing to, and then
        advances the pointer to point to the next character. */
    juce_wchar getAndAdvance() throw()
    {
        uint32 n = (uint32) (uint16) *data++;

        if (n >= 0xd800 && n <= 0xdfff && ((uint32) (uint16) *data) >= 0xdc00)
            n = 0x10000 + ((((n - 0xd800) << 10) | (((uint32) (uint16) *data++) - 0xdc00)));

        return (juce_wchar) n;
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF16 operator++ (int) throw()
    {
        CharPointer_UTF16 temp (*this);
        ++*this;
        return temp;
    }

    /** Moves this pointer forwards by the specified number of characters. */
    void operator+= (int numToSkip) throw()
    {
        jassert (numToSkip >= 0);

        while (--numToSkip >= 0)
            ++*this;
    }

    /** Returns the character at a given character index from the start of the string. */
    juce_wchar operator[] (const int characterIndex) const throw()
    {
        CharPointer_UTF16 p (*this);
        p += characterIndex;
        return *p;
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_UTF16 operator+ (const int numToSkip) const throw()
    {
        CharPointer_UTF16 p (*this);
        p += numToSkip;
        return p;
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    void write (juce_wchar charToWrite) throw()
    {
        if (charToWrite >= 0x10000)
        {
            charToWrite -= 0x10000;
            *data++ = (CharType) (0xd800 + (charToWrite >> 10));
            *data++ = (CharType) (0xdc00 + (charToWrite & 0x3ff));
        }
        else
        {
            *data++ = (CharType) charToWrite;
        }
    }

    /** Writes a null character to this string (leaving the pointer's position unchanged). */
    inline void writeNull() const throw()
    {
        *data = 0;
    }

    /** Returns the number of characters in this string. */
    size_t length() const throw()
    {
        const CharType* d = data;
        size_t count = 0;

        for (;;)
        {
            const int n = *d++;

            if (n >= 0xd800 && n <= 0xdfff)
            {
                if (*d++ == 0)
                    break;
            }
            else if (n == 0)
                break;

            ++count;
        }

        return count;
    }

    /** Returns the number of characters in this string, or the given value, whichever is lower. */
    size_t lengthUpTo (const size_t maxCharsToCount) const throw()
    {
        return CharacterFunctions::lengthUpTo (*this, maxCharsToCount);
    }

    /** Returns the number of bytes that are used to represent this string.
        This includes the terminating null character.
    */
    size_t sizeInBytes() const throw()
    {
        return sizeof (CharType) * (findNullIndex (data) + 1);
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static size_t getBytesRequiredFor (const juce_wchar charToWrite) throw()
    {
        return (charToWrite >= 0x10000) ? (sizeof (CharType) * 2) : sizeof (CharType);
    }

    /** Returns the number of bytes that would be needed to represent the given
        string in this encoding format.
        The value returned does NOT include the terminating null character.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (CharPointer text) throw()
    {
        size_t count = 0;
        juce_wchar n;

        while ((n = text.getAndAdvance()) != 0)
            count += getBytesRequiredFor (n);

        return count;
    }

    /** Returns a pointer to the null character that terminates this string. */
    CharPointer_UTF16 findTerminatingNull() const throw()
    {
        const CharType* t = data;

        while (*t != 0)
            ++t;

        return CharPointer_UTF16 (t);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    template <typename CharPointer>
    void writeAll (const CharPointer& src) throw()
    {
        CharacterFunctions::copyAll (*this, src);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    void writeAll (const CharPointer_UTF16& src) throw()
    {
        const CharType* s = src.data;

        while ((*data = *s) != 0)
        {
            ++data;
            ++s;
        }
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxDestBytes parameter specifies the maximum number of bytes that can be written
        to the destination buffer before stopping.
    */
    template <typename CharPointer>
    int writeWithDestByteLimit (const CharPointer& src, const int maxDestBytes) throw()
    {
        return CharacterFunctions::copyWithDestByteLimit (*this, src, maxDestBytes);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxChars parameter specifies the maximum number of characters that can be
        written to the destination buffer before stopping (including the terminating null).
    */
    template <typename CharPointer>
    void writeWithCharLimit (const CharPointer& src, const int maxChars) throw()
    {
        CharacterFunctions::copyWithCharLimit (*this, src, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compare (const CharPointer& other) const throw()
    {
        return CharacterFunctions::compare (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareUpTo (const CharPointer& other, const int maxChars) const throw()
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compareIgnoreCase (const CharPointer& other) const throw()
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareIgnoreCaseUpTo (const CharPointer& other, const int maxChars) const throw()
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

   #if JUCE_WINDOWS && ! DOXYGEN
    int compareIgnoreCase (const CharPointer_UTF16& other) const throw()
    {
        return _wcsicmp (data, other.data);
    }

    int compareIgnoreCaseUpTo (const CharPointer_UTF16& other, int maxChars) const throw()
    {
        return _wcsnicmp (data, other.data, maxChars);
    }

    int indexOf (const CharPointer_UTF16& stringToFind) const throw()
    {
        const CharType* const t = wcsstr (data, stringToFind.getAddress());
        return t == 0 ? -1 : (int) (t - data);
    }
   #endif

    /** Returns the character index of a substring, or -1 if it isn't found. */
    template <typename CharPointer>
    int indexOf (const CharPointer& stringToFind) const throw()
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const juce_wchar charToFind) const throw()
    {
        return CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const juce_wchar charToFind, const bool ignoreCase) const throw()
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          : CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Returns true if the first character of this string is whitespace. */
    bool isWhitespace() const throw()       { return CharacterFunctions::isWhitespace (operator*()) != 0; }
    /** Returns true if the first character of this string is a digit. */
    bool isDigit() const throw()            { return CharacterFunctions::isDigit (operator*()) != 0; }
    /** Returns true if the first character of this string is a letter. */
    bool isLetter() const throw()           { return CharacterFunctions::isLetter (operator*()) != 0; }
    /** Returns true if the first character of this string is a letter or digit. */
    bool isLetterOrDigit() const throw()    { return CharacterFunctions::isLetterOrDigit (operator*()) != 0; }
    /** Returns true if the first character of this string is upper-case. */
    bool isUpperCase() const throw()        { return CharacterFunctions::isUpperCase (operator*()) != 0; }
    /** Returns true if the first character of this string is lower-case. */
    bool isLowerCase() const throw()        { return CharacterFunctions::isLowerCase (operator*()) != 0; }

    /** Returns an upper-case version of the first character of this string. */
    juce_wchar toUpperCase() const throw()  { return CharacterFunctions::toUpperCase (operator*()); }
    /** Returns a lower-case version of the first character of this string. */
    juce_wchar toLowerCase() const throw()  { return CharacterFunctions::toLowerCase (operator*()); }

    /** Parses this string as a 32-bit integer. */
    int getIntValue32() const throw()
    {
       #if JUCE_WINDOWS
        return _wtoi (data);
       #else
        return CharacterFunctions::getIntValue <int, CharPointer_UTF16> (*this);
       #endif
    }

    /** Parses this string as a 64-bit integer. */
    int64 getIntValue64() const throw()
    {
       #if JUCE_WINDOWS
        return _wtoi64 (data);
       #else
        return CharacterFunctions::getIntValue <int64, CharPointer_UTF16> (*this);
       #endif
    }

    /** Parses this string as a floating point double. */
    double getDoubleValue() const throw()   { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_UTF16 findEndOfWhitespace() const throw()    { return CharacterFunctions::findEndOfWhitespace (*this); }

    /** Returns true if the given unicode character can be represented in this encoding. */
    static bool canRepresent (juce_wchar character) throw()
    {
        return ((unsigned int) character) < (unsigned int) 0x10ffff
                 && (((unsigned int) character) < 0xd800 || ((unsigned int) character) > 0xdfff);
    }

    /** Returns true if this data contains a valid string in this encoding. */
    static bool isValidString (const CharType* dataToTest, int maxBytesToRead)
    {
        maxBytesToRead /= sizeof (CharType);

        while (--maxBytesToRead >= 0 && *dataToTest != 0)
        {
            const uint32 n = (uint32) (uint16) *dataToTest++;

            if (n >= 0xd800)
            {
                if (n > 0x10ffff)
                    return false;

                if (n <= 0xdfff)
                {
                    if (n > 0xdc00)
                        return false;

                    const uint32 nextChar = (uint32) (uint16) *dataToTest++;

                    if (nextChar < 0xdc00 || nextChar > 0xdfff)
                        return false;
                }
            }
        }

        return true;
    }

    /** These values are the byte-order-mark (BOM) values for a UTF-16 stream. */
    enum
    {
        byteOrderMarkBE1 = 0xfe,
        byteOrderMarkBE2 = 0xff,
        byteOrderMarkLE1 = 0xff,
        byteOrderMarkLE2 = 0xfe
    };

private:
    CharType* data;

    static int findNullIndex (const CharType* const t) throw()
    {
        int n = 0;

        while (t[n] != 0)
            ++n;

        return n;
    }
};


#endif   // __JUCE_CHARPOINTER_UTF16_JUCEHEADER__
