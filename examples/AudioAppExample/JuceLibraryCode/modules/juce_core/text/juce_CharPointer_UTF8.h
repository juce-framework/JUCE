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

#ifndef JUCE_CHARPOINTER_UTF8_H_INCLUDED
#define JUCE_CHARPOINTER_UTF8_H_INCLUDED

//==============================================================================
/**
    Wraps a pointer to a null-terminated UTF-8 character string, and provides
    various methods to operate on the data.
    @see CharPointer_UTF16, CharPointer_UTF32
*/
class CharPointer_UTF8
{
public:
    typedef char CharType;

    inline explicit CharPointer_UTF8 (const CharType* const rawPointer) noexcept
        : data (const_cast <CharType*> (rawPointer))
    {
    }

    inline CharPointer_UTF8 (const CharPointer_UTF8& other) noexcept
        : data (other.data)
    {
    }

    inline CharPointer_UTF8 operator= (CharPointer_UTF8 other) noexcept
    {
        data = other.data;
        return *this;
    }

    inline CharPointer_UTF8 operator= (const CharType* text) noexcept
    {
        data = const_cast <CharType*> (text);
        return *this;
    }

    /** This is a pointer comparison, it doesn't compare the actual text. */
    inline bool operator== (CharPointer_UTF8 other) const noexcept      { return data == other.data; }
    inline bool operator!= (CharPointer_UTF8 other) const noexcept      { return data != other.data; }
    inline bool operator<= (CharPointer_UTF8 other) const noexcept      { return data <= other.data; }
    inline bool operator<  (CharPointer_UTF8 other) const noexcept      { return data <  other.data; }
    inline bool operator>= (CharPointer_UTF8 other) const noexcept      { return data >= other.data; }
    inline bool operator>  (CharPointer_UTF8 other) const noexcept      { return data >  other.data; }

    /** Returns the address that this pointer is pointing to. */
    inline CharType* getAddress() const noexcept        { return data; }

    /** Returns the address that this pointer is pointing to. */
    inline operator const CharType*() const noexcept    { return data; }

    /** Returns true if this pointer is pointing to a null character. */
    inline bool isEmpty() const noexcept                { return *data == 0; }

    /** Returns the unicode character that this pointer is pointing to. */
    juce_wchar operator*() const noexcept
    {
        const signed char byte = (signed char) *data;

        if (byte >= 0)
            return (juce_wchar) (uint8) byte;

        uint32 n = (uint32) (uint8) byte;
        uint32 mask = 0x7f;
        uint32 bit = 0x40;
        size_t numExtraValues = 0;

        while ((n & bit) != 0 && bit > 0x10)
        {
            mask >>= 1;
            ++numExtraValues;
            bit >>= 1;
        }

        n &= mask;

        for (size_t i = 1; i <= numExtraValues; ++i)
        {
            const uint8 nextByte = (uint8) data [i];

            if ((nextByte & 0xc0) != 0x80)
                break;

            n <<= 6;
            n |= (nextByte & 0x3f);
        }

        return (juce_wchar) n;
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF8& operator++() noexcept
    {
        jassert (*data != 0); // trying to advance past the end of the string?
        const signed char n = (signed char) *data++;

        if (n < 0)
        {
            juce_wchar bit = 0x40;

            while ((n & bit) != 0 && bit > 0x8)
            {
                ++data;
                bit >>= 1;
            }
        }

        return *this;
    }

    /** Moves this pointer back to the previous character in the string. */
    CharPointer_UTF8 operator--() noexcept
    {
        int count = 0;

        while ((*--data & 0xc0) == 0x80 && ++count < 4)
        {}

        return *this;
    }

    /** Returns the character that this pointer is currently pointing to, and then
        advances the pointer to point to the next character. */
    juce_wchar getAndAdvance() noexcept
    {
        const signed char byte = (signed char) *data++;

        if (byte >= 0)
            return (juce_wchar) (uint8) byte;

        uint32 n = (uint32) (uint8) byte;
        uint32 mask = 0x7f;
        uint32 bit = 0x40;
        int numExtraValues = 0;

        while ((n & bit) != 0 && bit > 0x8)
        {
            mask >>= 1;
            ++numExtraValues;
            bit >>= 1;
        }

        n &= mask;

        while (--numExtraValues >= 0)
        {
            const uint32 nextByte = (uint32) (uint8) *data;

            if ((nextByte & 0xc0) != 0x80)
                break;

            ++data;
            n <<= 6;
            n |= (nextByte & 0x3f);
        }

        return (juce_wchar) n;
    }

    /** Moves this pointer along to the next character in the string. */
    CharPointer_UTF8 operator++ (int) noexcept
    {
        CharPointer_UTF8 temp (*this);
        ++*this;
        return temp;
    }

    /** Moves this pointer forwards by the specified number of characters. */
    void operator+= (int numToSkip) noexcept
    {
        if (numToSkip < 0)
        {
            while (++numToSkip <= 0)
                --*this;
        }
        else
        {
            while (--numToSkip >= 0)
                ++*this;
        }
    }

    /** Moves this pointer backwards by the specified number of characters. */
    void operator-= (int numToSkip) noexcept
    {
        operator+= (-numToSkip);
    }

    /** Returns the character at a given character index from the start of the string. */
    juce_wchar operator[] (int characterIndex) const noexcept
    {
        CharPointer_UTF8 p (*this);
        p += characterIndex;
        return *p;
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_UTF8 operator+ (int numToSkip) const noexcept
    {
        CharPointer_UTF8 p (*this);
        p += numToSkip;
        return p;
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_UTF8 operator- (int numToSkip) const noexcept
    {
        CharPointer_UTF8 p (*this);
        p += -numToSkip;
        return p;
    }

    /** Returns the number of characters in this string. */
    size_t length() const noexcept
    {
        const CharType* d = data;
        size_t count = 0;

        for (;;)
        {
            const uint32 n = (uint32) (uint8) *d++;

            if ((n & 0x80) != 0)
            {
                while ((*d & 0xc0) == 0x80)
                    ++d;
            }
            else if (n == 0)
                break;

            ++count;
        }

        return count;
    }

    /** Returns the number of characters in this string, or the given value, whichever is lower. */
    size_t lengthUpTo (const size_t maxCharsToCount) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, maxCharsToCount);
    }

    /** Returns the number of characters in this string, or up to the given end pointer, whichever is lower. */
    size_t lengthUpTo (const CharPointer_UTF8 end) const noexcept
    {
        return CharacterFunctions::lengthUpTo (*this, end);
    }

    /** Returns the number of bytes that are used to represent this string.
        This includes the terminating null character.
    */
    size_t sizeInBytes() const noexcept
    {
        jassert (data != nullptr);
        return strlen (data) + 1;
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static size_t getBytesRequiredFor (const juce_wchar charToWrite) noexcept
    {
        size_t num = 1;
        const uint32 c = (uint32) charToWrite;

        if (c >= 0x80)
        {
            ++num;
            if (c >= 0x800)
            {
                ++num;
                if (c >= 0x10000)
                    ++num;
            }
        }

        return num;
    }

    /** Returns the number of bytes that would be needed to represent the given
        string in this encoding format.
        The value returned does NOT include the terminating null character.
    */
    template <class CharPointer>
    static size_t getBytesRequiredFor (CharPointer text) noexcept
    {
        size_t count = 0;
        juce_wchar n;

        while ((n = text.getAndAdvance()) != 0)
            count += getBytesRequiredFor (n);

        return count;
    }

    /** Returns a pointer to the null character that terminates this string. */
    CharPointer_UTF8 findTerminatingNull() const noexcept
    {
        return CharPointer_UTF8 (data + strlen (data));
    }

    /** Writes a unicode character to this string, and advances this pointer to point to the next position. */
    void write (const juce_wchar charToWrite) noexcept
    {
        const uint32 c = (uint32) charToWrite;

        if (c >= 0x80)
        {
            int numExtraBytes = 1;
            if (c >= 0x800)
            {
                ++numExtraBytes;
                if (c >= 0x10000)
                    ++numExtraBytes;
            }

            *data++ = (CharType) ((uint32) (0xff << (7 - numExtraBytes)) | (c >> (numExtraBytes * 6)));

            while (--numExtraBytes >= 0)
                *data++ = (CharType) (0x80 | (0x3f & (c >> (numExtraBytes * 6))));
        }
        else
        {
            *data++ = (CharType) c;
        }
    }

    /** Writes a null character to this string (leaving the pointer's position unchanged). */
    inline void writeNull() const noexcept
    {
        *data = 0;
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    template <typename CharPointer>
    void writeAll (const CharPointer src) noexcept
    {
        CharacterFunctions::copyAll (*this, src);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes. */
    void writeAll (const CharPointer_UTF8 src) noexcept
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
    size_t writeWithDestByteLimit (const CharPointer src, const size_t maxDestBytes) noexcept
    {
        return CharacterFunctions::copyWithDestByteLimit (*this, src, maxDestBytes);
    }

    /** Copies a source string to this pointer, advancing this pointer as it goes.
        The maxChars parameter specifies the maximum number of characters that can be
        written to the destination buffer before stopping (including the terminating null).
    */
    template <typename CharPointer>
    void writeWithCharLimit (const CharPointer src, const int maxChars) noexcept
    {
        CharacterFunctions::copyWithCharLimit (*this, src, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compare (const CharPointer other) const noexcept
    {
        return CharacterFunctions::compare (*this, other);
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareUpTo (const CharPointer other, const int maxChars) const noexcept
    {
        return CharacterFunctions::compareUpTo (*this, other, maxChars);
    }

    /** Compares this string with another one. */
    template <typename CharPointer>
    int compareIgnoreCase (const CharPointer other) const noexcept
    {
        return CharacterFunctions::compareIgnoreCase (*this, other);
    }

    /** Compares this string with another one. */
    int compareIgnoreCase (const CharPointer_UTF8 other) const noexcept
    {
       #if JUCE_WINDOWS
        return stricmp (data, other.data);
       #else
        return strcasecmp (data, other.data);
       #endif
    }

    /** Compares this string with another one, up to a specified number of characters. */
    template <typename CharPointer>
    int compareIgnoreCaseUpTo (const CharPointer other, const int maxChars) const noexcept
    {
        return CharacterFunctions::compareIgnoreCaseUpTo (*this, other, maxChars);
    }

    /** Returns the character index of a substring, or -1 if it isn't found. */
    template <typename CharPointer>
    int indexOf (const CharPointer stringToFind) const noexcept
    {
        return CharacterFunctions::indexOf (*this, stringToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const juce_wchar charToFind) const noexcept
    {
        return CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Returns the character index of a unicode character, or -1 if it isn't found. */
    int indexOf (const juce_wchar charToFind, const bool ignoreCase) const noexcept
    {
        return ignoreCase ? CharacterFunctions::indexOfCharIgnoreCase (*this, charToFind)
                          : CharacterFunctions::indexOfChar (*this, charToFind);
    }

    /** Returns true if the first character of this string is whitespace. */
    bool isWhitespace() const noexcept      { return *data == ' ' || (*data <= 13 && *data >= 9); }
    /** Returns true if the first character of this string is a digit. */
    bool isDigit() const noexcept           { return *data >= '0' && *data <= '9'; }
    /** Returns true if the first character of this string is a letter. */
    bool isLetter() const noexcept          { return CharacterFunctions::isLetter (operator*()) != 0; }
    /** Returns true if the first character of this string is a letter or digit. */
    bool isLetterOrDigit() const noexcept   { return CharacterFunctions::isLetterOrDigit (operator*()) != 0; }
    /** Returns true if the first character of this string is upper-case. */
    bool isUpperCase() const noexcept       { return CharacterFunctions::isUpperCase (operator*()) != 0; }
    /** Returns true if the first character of this string is lower-case. */
    bool isLowerCase() const noexcept       { return CharacterFunctions::isLowerCase (operator*()) != 0; }

    /** Returns an upper-case version of the first character of this string. */
    juce_wchar toUpperCase() const noexcept { return CharacterFunctions::toUpperCase (operator*()); }
    /** Returns a lower-case version of the first character of this string. */
    juce_wchar toLowerCase() const noexcept { return CharacterFunctions::toLowerCase (operator*()); }

    /** Parses this string as a 32-bit integer. */
    int getIntValue32() const noexcept      { return atoi (data); }

    /** Parses this string as a 64-bit integer. */
    int64 getIntValue64() const noexcept
    {
       #if JUCE_LINUX || JUCE_ANDROID
        return atoll (data);
       #elif JUCE_WINDOWS
        return _atoi64 (data);
       #else
        return CharacterFunctions::getIntValue <int64, CharPointer_UTF8> (*this);
       #endif
    }

    /** Parses this string as a floating point double. */
    double getDoubleValue() const noexcept  { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_UTF8 findEndOfWhitespace() const noexcept   { return CharacterFunctions::findEndOfWhitespace (*this); }

    /** Returns true if the given unicode character can be represented in this encoding. */
    static bool canRepresent (juce_wchar character) noexcept
    {
        return ((unsigned int) character) < (unsigned int) 0x10ffff;
    }

    /** Returns true if this data contains a valid string in this encoding. */
    static bool isValidString (const CharType* dataToTest, int maxBytesToRead)
    {
        while (--maxBytesToRead >= 0 && *dataToTest != 0)
        {
            const signed char byte = (signed char) *dataToTest++;

            if (byte < 0)
            {
                uint8 bit = 0x40;
                int numExtraValues = 0;

                while ((byte & bit) != 0)
                {
                    if (bit < 8)
                        return false;

                    ++numExtraValues;
                    bit >>= 1;

                    if (bit == 8 && (numExtraValues > maxBytesToRead
                                       || *CharPointer_UTF8 (dataToTest - 1) > 0x10ffff))
                        return false;
                }

                maxBytesToRead -= numExtraValues;
                if (maxBytesToRead < 0)
                    return false;

                while (--numExtraValues >= 0)
                    if ((*dataToTest++ & 0xc0) != 0x80)
                        return false;
            }
        }

        return true;
    }

    /** Atomically swaps this pointer for a new value, returning the previous value. */
    CharPointer_UTF8 atomicSwap (const CharPointer_UTF8 newValue)
    {
        return CharPointer_UTF8 (reinterpret_cast <Atomic<CharType*>&> (data).exchange (newValue.data));
    }

    /** These values are the byte-order mark (BOM) values for a UTF-8 stream. */
    enum
    {
        byteOrderMark1 = 0xef,
        byteOrderMark2 = 0xbb,
        byteOrderMark3 = 0xbf
    };

    /** Returns true if the first three bytes in this pointer are the UTF8 byte-order mark (BOM).
        The pointer must not be null, and must point to at least 3 valid bytes.
    */
    static bool isByteOrderMark (const void* possibleByteOrder) noexcept
    {
        jassert (possibleByteOrder != nullptr);
        const uint8* const c = static_cast<const uint8*> (possibleByteOrder);

        return c[0] == (uint8) byteOrderMark1
            && c[1] == (uint8) byteOrderMark2
            && c[2] == (uint8) byteOrderMark3;
    }

private:
    CharType* data;
};

#endif   // JUCE_CHARPOINTER_UTF8_H_INCLUDED
