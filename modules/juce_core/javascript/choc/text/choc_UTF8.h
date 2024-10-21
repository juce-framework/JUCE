//
//    ██████ ██   ██  ██████   ██████
//   ██      ██   ██ ██    ██ ██            ** Classy Header-Only Classes **
//   ██      ███████ ██    ██ ██
//   ██      ██   ██ ██    ██ ██           https://github.com/Tracktion/choc
//    ██████ ██   ██  ██████   ██████
//
//   CHOC is (C)2022 Tracktion Corporation, and is offered under the terms of the ISC license:
//
//   Permission to use, copy, modify, and/or distribute this software for any purpose with or
//   without fee is hereby granted, provided that the above copyright notice and this permission
//   notice appear in all copies. THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
//   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
//   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
//   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
//   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
//   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef CHOC_UTF8_HEADER_INCLUDED
#define CHOC_UTF8_HEADER_INCLUDED

#include <cstddef>
#include "choc_StringUtilities.h"

namespace
{
namespace choc::text
{

/// An integer type to represent a unicode code-point.
using UnicodeChar = uint32_t;

//==============================================================================
/** A non-owning pointer which can iterate over a chunk of null-terminated UTF-8 text
    and read it as wide unicode characters.
*/
struct UTF8Pointer
{
    explicit constexpr UTF8Pointer (const char* utf8Text) noexcept  : text (utf8Text) {}

    UTF8Pointer() = default;
    UTF8Pointer (const UTF8Pointer&) = default;
    UTF8Pointer& operator= (const UTF8Pointer&) = default;

    /// Returns the raw data that this points to.
    const char* data() const noexcept                   { return text; }

    /// Returns true if the pointer is not null.
    operator bool() const noexcept                      { return text != nullptr; }

    /// Returns true if the pointer is either null or points to a null terminator char.
    bool empty() const                                  { return text == nullptr || *text == 0; }

    /// Returns the length by iterating all unicode chars and counting them.
    /// Note that this is slow, and is not a count of the number of bytes in the string!
    size_t length() const;

    //==============================================================================
    /// Returns the first unicode character in the string.
    UnicodeChar operator*() const;

    /// Skips past the first unicode character.
    /// Moving beyond the end of the string is undefined behaviour and will trigger an assertion.
    UTF8Pointer& operator++();

    /// Skips past the first unicode character.
    /// Moving beyond the end of the string is undefined behaviour and will trigger an assertion.
    UTF8Pointer operator++ (int);

    /// Moves backwards to the previous unicode character.
    /// Moving beyond the end of the string is undefined behaviour.
    UTF8Pointer operator--();

    /// Skips past the given number of unicode characters.
    /// Moving beyond the end of the string is undefined behaviour and will trigger an assertion.
    UTF8Pointer& operator+= (size_t numCharsToSkip);

    /// Returns a pointer which points to the n-th unicode character in the text
    /// Reading beyond the end of the string is undefined behaviour and may trigger an assertion.
    UTF8Pointer operator+ (size_t numCharsToSkip) const;

    /// Returns a pointer which points to the n-th unicode character in the text.
    /// Reading beyond the end of the string is undefined behaviour and may trigger an assertion.
    UTF8Pointer operator+ (int numCharsToSkip) const;

    /// Skips past the first unicode character and returns it as a code-point.
    /// Calling this when the current character is the terminator will leave the pointer in an
    /// invalid state.
    UnicodeChar popFirstChar();

    /// Finds the next occurrence of the given string, or return a nullptr if not found.
    UTF8Pointer find (const char* textToFind) const;

    /// Returns true if the text starts with this string
    bool startsWith (const char* textToMatch) const;

    /// If the first character matches the given one, this will advance the pointer and return true.
    bool skipIfStartsWith (char charToMatch);

    /// If the start of the text matches the given string, this will advance this pointer to skip
    /// past it, and return true. If not, it will return false without modifying this pointer.
    bool skipIfStartsWith (const char* textToMatch);

    /// Returns a pointer to the first non-whitespace character in the given string (which may
    /// be the terminating null character if it's all whitespace).
    UTF8Pointer findEndOfWhitespace() const;

    /// Iterates backwards from this position to find the first character that follows
    /// a new-line. The pointer provided marks the furthest back that the function should search
    UTF8Pointer findStartOfLine (UTF8Pointer startOfValidText) const;

    /// Searches forwards for the next character that is followed by a new-line or a null-terminator.
    UTF8Pointer findEndOfLine() const;

    //==============================================================================
    struct EndIterator {};

    struct Iterator
    {
        explicit constexpr Iterator (const char* t) : text (t) {}
        Iterator (const Iterator&) = default;
        Iterator& operator= (const Iterator&) = default;

        UnicodeChar operator*() const           { return *UTF8Pointer (text); }
        Iterator& operator++()                  { UTF8Pointer p (text); ++p; text = p.text; return *this; }
        Iterator operator++ (int)               { auto old = *this; ++*this; return old; }
        bool operator== (EndIterator) const     { return *text == 0; }
        bool operator!= (EndIterator) const     { return *text != 0; }

    private:
        const char* text;
    };

    Iterator begin() const;
    EndIterator end() const;

    //==============================================================================
    /// This does a pointer comparison, NOT a comparison of the text itself!
    bool operator== (UTF8Pointer other) const noexcept      { return text == other.text; }
    /// This does a pointer comparison, NOT a comparison of the text itself!
    bool operator!= (UTF8Pointer other) const noexcept      { return text != other.text; }
    /// This does a pointer comparison, NOT a comparison of the text itself!
    bool operator<  (UTF8Pointer other) const noexcept      { return text <  other.text; }
    /// This does a pointer comparison, NOT a comparison of the text itself!
    bool operator>  (UTF8Pointer other) const noexcept      { return text >  other.text; }
    /// This does a pointer comparison, NOT a comparison of the text itself!
    bool operator<= (UTF8Pointer other) const noexcept      { return text <= other.text; }
    /// This does a pointer comparison, NOT a comparison of the text itself!
    bool operator>= (UTF8Pointer other) const noexcept      { return text >= other.text; }

    bool operator== (decltype(nullptr)) const noexcept      { return text == nullptr; }
    bool operator!= (decltype(nullptr)) const noexcept      { return text != nullptr; }

private:
    const char* text = nullptr;
};

//==============================================================================
/// Checks a given chunk of data to see whether it's valid UTF-8.
/// If no errors are found, this returns nullptr. If an error is found, it returns the address
/// of the offending byte. Note that zero bytes in the data are considered to be valid UTF-8.
const char* findInvalidUTF8Data (const void* dataToCheck, size_t numBytesToRead);

/// Writes the bytes for a unicode character, and returns the number of bytes that were needed.
/// The buffer passed in needs to have at least 4 bytes capacity.
uint32_t convertUnicodeCodepointToUTF8 (char* dest, UnicodeChar codepoint);

/// Appends a unicode codepoint to a std::string as a sequence of UTF-8 bytes.
void appendUTF8 (std::string& target, UnicodeChar codepoint);

/// Checks whether a given codepoint is a high-surrogate
bool isUnicodeHighSurrogate (UnicodeChar codepoint);

/// Checks whether a given codepoint is a low-surrogate
bool isUnicodeLowSurrogate (UnicodeChar codepoint);

struct SurrogatePair
{
    UnicodeChar high = 0, low = 0;
};

/// For a codepoint >= 0x10000, this will return a surrogate pair to represent it.
SurrogatePair splitCodePointIntoSurrogatePair (UnicodeChar fullCodePoint);

/// Combines a high and low surrogate into a single codepoint.
UnicodeChar createUnicodeFromHighAndLowSurrogates (SurrogatePair);

/// Checks a UTF-8/CESU-8 string to see if it contains any surrogate pairs.
/// If it does, then to use it as UTF-8 you'll probably need to run it through
/// convertSurrogatePairsToUTF8().
bool containsSurrogatePairs (UTF8Pointer);

/// Returns a string where any surrogate pairs have been converted to UTF-8 codepoints.
std::string convertSurrogatePairsToUTF8 (UTF8Pointer);

/// Returns true if the given UTF-8 string can be used as CESU-8 without conversion. If not,
/// you'll need to run it through convertUTF8ToCESU8() to convert the 32-bit code-points
/// to surrogate pairs.
bool isValidCESU8 (std::string_view utf8);

/// Converts any 32-bit characters in this UTF-8 string to surrogate pairs, which makes
/// the resulting string suitable for use at CESU-8.
std::string convertUTF8ToCESU8 (UTF8Pointer);


//==============================================================================
/// Represents a line and column index within a block of text.
struct LineAndColumn
{
    /// Valid line and column values start at 1.
    /// If either is 0, it means that the LineAndColumn object is uninitialised.
    size_t line = 0, column = 0;

    /// Returns true if neither the line nor column is zero.
    bool isValid() const noexcept          { return line != 0 && column != 0; }

    /// Turns this location into a [line]:[col] string suitable for use in a
    /// standard compiler error message format.
    std::string toString() const;
};

/// Given a block of text and a position within it, this will work out the
/// line and column of that position.
LineAndColumn findLineAndColumn (UTF8Pointer fullText,
                                 UTF8Pointer targetPosition);


//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

inline size_t UTF8Pointer::length() const
{
    size_t count = 0;

    if (text != nullptr)
        for (auto p = *this; *p.text != 0; ++p)
            ++count;

    return count;
}

inline const char* findInvalidUTF8Data (const void* dataToCheck, size_t numBytes)
{
    CHOC_ASSERT (dataToCheck != nullptr);
    auto source = static_cast<const char*> (dataToCheck);
    const auto end = source + numBytes;

    for (;;)
    {
        if (source >= end)
            return nullptr;

        auto byte = static_cast<signed char> (*source);

        if (byte >= 0)
        {
            ++source;
            continue;
        }

        int testBit = 0x40, numExtraBytes = 0;

        while ((byte & testBit) != 0)
        {
            testBit >>= 1;
            ++numExtraBytes;

            if (numExtraBytes > 3
                || source + static_cast<size_t> (numExtraBytes) >= end
                || (numExtraBytes == 3 && *UTF8Pointer (source) > 0x10ffff))
            {
                return source;
            }
        }

        if (numExtraBytes == 0)
            return source;

        ++source;

        for (int i = 0; i < numExtraBytes; ++i)
        {
            if ((*source & 0xc0) != 0x80)
                return source;

            ++source;
        }
    }
}

inline UnicodeChar UTF8Pointer::operator*() const
{
    return UTF8Pointer (*this).popFirstChar();
}

inline UTF8Pointer& UTF8Pointer::operator++()
{
    CHOC_ASSERT (! empty());  // can't advance past the zero-terminator
    auto firstByte = static_cast<signed char> (*text++);

    if (firstByte >= 0)
        return *this;

    uint32_t testBit = 0x40, unicodeChar = static_cast<unsigned char> (firstByte);

    while ((unicodeChar & testBit) != 0 && testBit > 8)
    {
        ++text;
        testBit >>= 1;
    }

    return *this;
}

inline UTF8Pointer UTF8Pointer::operator++ (int)
{
    auto prev = *this;
    operator++();
    return prev;
}

inline UTF8Pointer UTF8Pointer::operator--()
{
    CHOC_ASSERT (text != nullptr); // mustn't use this on nullptrs
    uint32_t bytesSkipped = 0;

    while ((*--text & 0xc0) == 0x80)
    {
        if (bytesSkipped > 2)
        {
            CHOC_ASSERT (bytesSkipped <= 2);
            break;
        }

        ++bytesSkipped;
    }

    return *this;
}

inline UTF8Pointer& UTF8Pointer::operator+= (size_t numCharsToSkip)
{
    while (numCharsToSkip != 0)
    {
        --numCharsToSkip;
        operator++();
    }

    return *this;
}

inline UTF8Pointer UTF8Pointer::operator+ (size_t numCharsToSkip) const
{
    auto p = *this;
    p += numCharsToSkip;
    return p;
}

inline UTF8Pointer UTF8Pointer::operator+ (int numCharsToSkip) const
{
    CHOC_ASSERT (numCharsToSkip >= 0);
    return operator+ (static_cast<size_t> (numCharsToSkip));
}

inline UnicodeChar UTF8Pointer::popFirstChar()
{
    CHOC_ASSERT (text != nullptr); // mustn't use this on nullptrs
    auto firstByte = static_cast<signed char> (*text++);
    UnicodeChar unicodeChar = static_cast<unsigned char> (firstByte);

    if (firstByte < 0)
    {
        uint32_t bitMask = 0x7f, numExtraBytes = 0;

        for (uint32_t testBit = 0x40; (unicodeChar & testBit) != 0 && testBit > 8; ++numExtraBytes)
        {
            bitMask >>= 1;
            testBit >>= 1;
        }

        unicodeChar &= bitMask;

        for (uint32_t i = 0; i < numExtraBytes; ++i)
        {
            uint32_t nextByte = static_cast<unsigned char> (*text);

            CHOC_ASSERT ((nextByte & 0xc0) == 0x80); // error in the data - you should always make sure the source
                                                        // gets validated before iterating a UTF8Pointer over it

            unicodeChar = (unicodeChar << 6) | (nextByte & 0x3f);
            ++text;
        }
    }

    return unicodeChar;
}

inline bool UTF8Pointer::startsWith (const char* textToMatch) const
{
    CHOC_ASSERT (textToMatch != nullptr);

    if (auto p = text)
    {
        while (*textToMatch != 0)
            if (*textToMatch++ != *p++)
                return false;

        return true;
    }

    return false;
}

inline UTF8Pointer UTF8Pointer::find (const char* textToFind) const
{
    CHOC_ASSERT (textToFind != nullptr);

    for (auto t = *this;; ++t)
        if (t.startsWith (textToFind) || t.empty())
            return t;
}

inline bool UTF8Pointer::skipIfStartsWith (char charToMatch)
{
    if (text != nullptr && *text == charToMatch && charToMatch != 0)
    {
        ++text;
        return true;
    }

    return false;
}

inline bool UTF8Pointer::skipIfStartsWith (const char* textToMatch)
{
    CHOC_ASSERT (textToMatch != nullptr);

    if (auto p = text)
    {
        while (*textToMatch != 0)
            if (*textToMatch++ != *p++)
                return false;

        text = p;
        return true;
    }

    return false;
}

inline UTF8Pointer UTF8Pointer::findEndOfWhitespace() const
{
    auto p = *this;

    if (p.text != nullptr)
        while (choc::text::isWhitespace (*p.text))
            ++p;

    return p;
}

inline UTF8Pointer UTF8Pointer::findStartOfLine (UTF8Pointer start) const
{
    if (text == nullptr)
        return {};

    auto l = *this;
    CHOC_ASSERT (l.text >= start.text && start.text != nullptr);

    while (l.text > start.text)
    {
        auto prev = l;
        auto c = *--prev;

        if (c == '\r' || c == '\n')
            break;

        l = prev;
    }

    return l;
}

inline UTF8Pointer UTF8Pointer::findEndOfLine() const
{
    if (text == nullptr)
        return {};

    auto l = *this;

    while (! l.empty())
    {
        auto c = l.popFirstChar();

        if (c == '\r' || c == '\n')
            break;
    }

    return l;
}

inline UTF8Pointer::Iterator UTF8Pointer::begin() const      { CHOC_ASSERT (text != nullptr); return Iterator (text); }
inline UTF8Pointer::EndIterator UTF8Pointer::end() const     { return EndIterator(); }

inline LineAndColumn findLineAndColumn (UTF8Pointer start, UTF8Pointer targetPosition)
{
    if (start == nullptr || targetPosition == nullptr)
        return {};

    CHOC_ASSERT (start <= targetPosition);
    LineAndColumn lc { 1, 1 };

    while (start < targetPosition && ! start.empty())
    {
        ++lc.column;
        if (*start++ == '\n')  { lc.line++; lc.column = 1; }
    }

    return lc;
}

inline std::string LineAndColumn::toString() const   { return std::to_string (line) + ':' + std::to_string (column); }

//==============================================================================
inline uint32_t convertUnicodeCodepointToUTF8 (char* dest, UnicodeChar unicodeChar)
{
    if (unicodeChar < 0x80)
    {
        *dest = static_cast<char> (unicodeChar);
        return 1;
    }

    uint32_t extraBytes = 1;

    if (unicodeChar >= 0x800)
    {
        ++extraBytes;

        if (unicodeChar >= 0x10000)
            ++extraBytes;
    }

    dest[0] = static_cast<char> ((0xffu << (7 - extraBytes)) | (unicodeChar >> (extraBytes * 6)));

    for (uint32_t i = 1; i <= extraBytes; ++i)
        dest[i] = static_cast<char> (0x80u | (0x3fu & (unicodeChar >> ((extraBytes - i) * 6))));

    return extraBytes + 1;
}

inline void appendUTF8 (std::string& target, UnicodeChar unicodeChar)
{
    char bytes[4];
    auto num = convertUnicodeCodepointToUTF8 (bytes, unicodeChar);
    target.append (bytes, num);
}

inline bool isUnicodeHighSurrogate (UnicodeChar codepoint)   { return codepoint >= 0xd800 && codepoint <= 0xdbff; }
inline bool isUnicodeLowSurrogate  (UnicodeChar codepoint)   { return codepoint >= 0xdc00 && codepoint <= 0xdfff; }

inline UnicodeChar createUnicodeFromHighAndLowSurrogates (SurrogatePair pair)
{
    if (! isUnicodeHighSurrogate (pair.high))   return pair.high;
    if (! isUnicodeLowSurrogate (pair.low))     return 0;

    return (pair.high << 10) + pair.low - 0x35fdc00u;
}

inline bool containsSurrogatePairs (UTF8Pointer text)
{
    for (;;)
    {
        auto c = text.popFirstChar();

        if (c == 0)
            return false;

        if (isUnicodeHighSurrogate (c))
            return true;
    }
}

inline std::string convertSurrogatePairsToUTF8 (UTF8Pointer text)
{
    std::string result;

    for (;;)
    {
        auto c = text.popFirstChar();

        if (choc::text::isUnicodeHighSurrogate (c))
            c = createUnicodeFromHighAndLowSurrogates ({ c, text.popFirstChar() });

        if (c == 0)
            return result;

        appendUTF8 (result, c);
    }
}

inline SurrogatePair splitCodePointIntoSurrogatePair (UnicodeChar fullCodePoint)
{
    CHOC_ASSERT (fullCodePoint >= 0x10000);

    return { static_cast<UnicodeChar> (0xd800u + ((fullCodePoint - 0x10000u) >> 10)),
             static_cast<UnicodeChar> (0xdc00u + (fullCodePoint & 0x3ffu)) };
}

inline bool isValidCESU8 (std::string_view utf8)
{
    for (auto c : utf8)
        if (static_cast<uint8_t> (c) >= 0xe8)
            return false;

    return true;
}

inline std::string convertUTF8ToCESU8 (UTF8Pointer utf8)
{
    std::string result;

    for (;;)
    {
        auto c = utf8.popFirstChar();

        if (c == 0)
            return result;

        if (c < 128)
        {
            result += (char) c;
        }
        else if (c >= 0x10000)
        {
            auto pair = splitCodePointIntoSurrogatePair (c);
            appendUTF8 (result, pair.high);
            appendUTF8 (result, pair.low);
        }
        else
        {
            appendUTF8 (result, c);
        }
    }
}


} // namespace choc::text
} // anonymous namespace

#endif
