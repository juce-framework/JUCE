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
    Wraps a pointer to a null-terminated UTF-8 character string, and provides
    various methods to operate on the data.
    @see CharPointer_UTF16, CharPointer_UTF32

    @tags{Core}
*/
class CharPointer_UTF8  final
{
public:
    using CharType = char;

    // Standard iterator traits for compatibility with STL algorithms
    using value_type = juce_wchar;
    using pointer = juce_wchar*;
    using reference = juce_wchar;  // Note: returns by value since this is a proxy iterator
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;

    explicit CharPointer_UTF8 (const CharType* rawPointer) noexcept
        : data (const_cast<CharType*> (rawPointer))
    {
    }

    CharPointer_UTF8 (const CharPointer_UTF8& other) = default;

    CharPointer_UTF8& operator= (const CharPointer_UTF8& other) noexcept = default;

    CharPointer_UTF8& operator= (const CharType* text) noexcept
    {
        data = const_cast<CharType*> (text);
        return *this;
    }

    /** This is a pointer comparison, it doesn't compare the actual text. */
    bool operator== (CharPointer_UTF8 other) const noexcept      { return data == other.data; }
    bool operator!= (CharPointer_UTF8 other) const noexcept      { return data != other.data; }
    bool operator<= (CharPointer_UTF8 other) const noexcept      { return data <= other.data; }
    bool operator<  (CharPointer_UTF8 other) const noexcept      { return data <  other.data; }
    bool operator>= (CharPointer_UTF8 other) const noexcept      { return data >= other.data; }
    bool operator>  (CharPointer_UTF8 other) const noexcept      { return data >  other.data; }

    /** Returns the address that this pointer is pointing to. */
    CharType* getAddress() const noexcept        { return data; }

    /** Returns the address that this pointer is pointing to. */
    operator const CharType*() const noexcept    { return data; }

    /** Returns true if this pointer is pointing to a null character. */
    bool isEmpty() const noexcept                { return *data == 0; }

    /** Returns true if this pointer is not pointing to a null character. */
    bool isNotEmpty() const noexcept             { return *data != 0; }

    /** Returns the unicode character that this pointer is pointing to. */
    juce_wchar operator*() const noexcept
    {
        auto byte = (signed char) *data;

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

        for (int i = 1; i <= numExtraValues; ++i)
        {
            auto nextByte = (uint32) (uint8) data[i];

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
        auto n = (signed char) *data++;

        if (n < 0)
        {
            uint8 bit = 0x40;

            while ((static_cast<uint8> (n) & bit) != 0 && bit > 0x8)
            {
                ++data;
                bit = static_cast<uint8> (bit >> 1);
            }
        }

        return *this;
    }

    /** Moves this pointer back to the previous character in the string. */
    CharPointer_UTF8& operator--() noexcept
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
        auto byte = (signed char) *data++;

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
            auto nextByte = (uint32) (uint8) *data;

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
    CharPointer_UTF8& operator+= (int numToSkip) noexcept
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

        return *this;
    }

    /** Moves this pointer backwards by the specified number of characters. */
    CharPointer_UTF8& operator-= (int numToSkip) noexcept
    {
        return operator+= (-numToSkip);
    }

    /** Returns the character at a given character index from the start of the string. */
    juce_wchar operator[] (int characterIndex) const noexcept
    {
        auto p (*this);
        p += characterIndex;
        return *p;
    }

    /** Returns a pointer which is moved forwards from this one by the specified number of characters. */
    CharPointer_UTF8 operator+ (int numToSkip) const noexcept
    {
        return CharPointer_UTF8 (*this) += numToSkip;
    }

    /** Returns a pointer which is moved backwards from this one by the specified number of characters. */
    CharPointer_UTF8 operator- (int numToSkip) const noexcept
    {
        return CharPointer_UTF8 (*this) -= numToSkip;
    }

    /** Returns the number of characters in this string. */
    size_t length() const noexcept
    {
        auto* d = data;
        size_t count = 0;

        for (;;)
        {
            auto n = (uint32) (uint8) *d++;

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
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6387)
        jassert (data != nullptr);
        return strlen (data) + 1;
        JUCE_END_IGNORE_WARNINGS_MSVC
    }

    /** Returns the number of bytes that would be needed to represent the given
        unicode character in this encoding format.
    */
    static size_t getBytesRequiredFor (const juce_wchar charToWrite) noexcept
    {
        size_t num = 1;
        auto c = (uint32) charToWrite;

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

        while (auto n = text.getAndAdvance())
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
        auto c = (uint32) charToWrite;

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
    void writeNull() const noexcept
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
        auto* s = src.data;

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
        return CharacterFunctions::compareIgnoreCase (*this, other);
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
    bool isWhitespace() const noexcept          { return CharacterFunctions::isWhitespace ((juce_wchar) *(*this)); }
    /** Returns true if the first character of this string is a digit. */
    bool isDigit() const noexcept               { const CharType c = *data; return c >= '0' && c <= '9'; }
    /** Returns true if the first character of this string is a letter. */
    bool isLetter() const noexcept              { return CharacterFunctions::isLetter (operator*()) != 0; }
    /** Returns true if the first character of this string is a letter or digit. */
    bool isLetterOrDigit() const noexcept       { return CharacterFunctions::isLetterOrDigit (operator*()) != 0; }
    /** Returns true if the first character of this string is upper-case. */
    bool isUpperCase() const noexcept           { return CharacterFunctions::isUpperCase (operator*()) != 0; }
    /** Returns true if the first character of this string is lower-case. */
    bool isLowerCase() const noexcept           { return CharacterFunctions::isLowerCase (operator*()) != 0; }

    /** Returns an upper-case version of the first character of this string. */
    juce_wchar toUpperCase() const noexcept     { return CharacterFunctions::toUpperCase (operator*()); }
    /** Returns a lower-case version of the first character of this string. */
    juce_wchar toLowerCase() const noexcept     { return CharacterFunctions::toLowerCase (operator*()); }

    /** Parses this string as a 32-bit integer. */
    int getIntValue32() const noexcept          { return atoi (data); }

    /** Parses this string as a 64-bit integer. */
    int64 getIntValue64() const noexcept
    {
       #if JUCE_WINDOWS
        return _atoi64 (data);
       #else
        return atoll (data);
       #endif
    }

    /** Parses this string as a floating point double. */
    double getDoubleValue() const noexcept                      { return CharacterFunctions::getDoubleValue (*this); }

    /** Returns the first non-whitespace character in the string. */
    CharPointer_UTF8 findEndOfWhitespace() const noexcept       { return CharacterFunctions::findEndOfWhitespace (*this); }

    /** Move this pointer to the first non-whitespace character in the string. */
    void incrementToEndOfWhitespace() noexcept                  { CharacterFunctions::incrementToEndOfWhitespace (*this); }

    /** Returns true if the given unicode character can be represented in this encoding. */
    static bool canRepresent (juce_wchar character) noexcept
    {
        return CharacterFunctions::isNonSurrogateCodePoint (character);
    }

    /** Returns true if this data contains a valid string in this encoding. */
    static bool isValidString (const CharType* codeUnits, int maxBytesToRead)
    {
        const auto maxCodeUnitsToRead = (size_t) maxBytesToRead / sizeof (CharType);

        for (size_t codeUnitIndex = 0; codeUnitIndex < maxCodeUnitsToRead; ++codeUnitIndex)
        {
            const auto firstByte = (uint8_t) codeUnits[codeUnitIndex];

            if (firstByte == 0)
                return true;

            if (CharacterFunctions::isAscii ((juce_wchar) firstByte))
                continue;

            auto numExtraBytes = [&]
            {
                if (firstByte < 0xc0)
                    return 0;

                if (firstByte < 0xe0)
                    return 1;

                if (firstByte <  0xf0)
                    return 2;

                if (firstByte <= 0xf4)
                    return 3;

                return 0;
            }();

            if (numExtraBytes == 0)
                return false;

            auto bytes = (uint32_t) firstByte;

            while (numExtraBytes--)
            {
                if (++codeUnitIndex >= maxCodeUnitsToRead)
                    return false;

                bytes <<= 8;
                bytes |= (uint32_t) (uint8_t) codeUnits[codeUnitIndex];
            }

            if (constexpr uint32_t firstTwoByteCodePoint = 0xc280; bytes < firstTwoByteCodePoint)
                return false;

            if (constexpr uint32_t lastTwoByteCodePoint = 0xdfbf; bytes <= lastTwoByteCodePoint)
                continue;

            if (constexpr uint32_t firstThreeByteCodePoint = 0xe0a080; bytes < firstThreeByteCodePoint)
                return false;

            if (constexpr uint32_t firstSurrogateCodePoint = 0xeda080; bytes < firstSurrogateCodePoint)
                continue;

            if (constexpr uint32_t lastSurrogateCodePoint = 0xedbfbf; bytes <= lastSurrogateCodePoint)
                return false;

            if (constexpr uint32_t lastThreeByteCodePoint = 0xefbfbf; bytes <= lastThreeByteCodePoint)
                continue;

            if (constexpr uint32_t firstFourByteCodePoint = 0xf0908080; bytes < firstFourByteCodePoint)
                return false;

            if (constexpr uint32_t lastFourByteCodePoint = 0xf48fbfbf; bytes <= lastFourByteCodePoint)
                continue;

            return false;
        }

        return true;
    }

    /** Atomically swaps this pointer for a new value, returning the previous value. */
    CharPointer_UTF8 atomicSwap (const CharPointer_UTF8 newValue)
    {
        return CharPointer_UTF8 (reinterpret_cast<Atomic<CharType*>&> (data).exchange (newValue.data));
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
        JUCE_BEGIN_IGNORE_WARNINGS_MSVC (28182)
        jassert (possibleByteOrder != nullptr);
        auto c = static_cast<const uint8*> (possibleByteOrder);

        return c[0] == (uint8) byteOrderMark1
            && c[1] == (uint8) byteOrderMark2
            && c[2] == (uint8) byteOrderMark3;
        JUCE_END_IGNORE_WARNINGS_MSVC
    }

private:
    CharType* data;
};

} // namespace juce
