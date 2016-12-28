/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

//==============================================================================
#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4514 4996)
#endif

juce_wchar CharacterFunctions::toUpperCase (const juce_wchar character) noexcept
{
    return (juce_wchar) towupper ((wint_t) character);
}

juce_wchar CharacterFunctions::toLowerCase (const juce_wchar character) noexcept
{
    return (juce_wchar) towlower ((wint_t) character);
}

bool CharacterFunctions::isUpperCase (const juce_wchar character) noexcept
{
   #if JUCE_WINDOWS
    return iswupper ((wint_t) character) != 0;
   #else
    return toLowerCase (character) != character;
   #endif
}

bool CharacterFunctions::isLowerCase (const juce_wchar character) noexcept
{
   #if JUCE_WINDOWS
    return iswlower ((wint_t) character) != 0;
   #else
    return toUpperCase (character) != character;
   #endif
}

#if JUCE_MSVC
 #pragma warning (pop)
#endif

//==============================================================================
bool CharacterFunctions::isWhitespace (const char character) noexcept
{
    return character == ' ' || (character <= 13 && character >= 9);
}

bool CharacterFunctions::isWhitespace (const juce_wchar character) noexcept
{
    return iswspace ((wint_t) character) != 0;
}

bool CharacterFunctions::isDigit (const char character) noexcept
{
    return (character >= '0' && character <= '9');
}

bool CharacterFunctions::isDigit (const juce_wchar character) noexcept
{
    return iswdigit ((wint_t) character) != 0;
}

bool CharacterFunctions::isLetter (const char character) noexcept
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z');
}

bool CharacterFunctions::isLetter (const juce_wchar character) noexcept
{
    return iswalpha ((wint_t) character) != 0;
}

bool CharacterFunctions::isLetterOrDigit (const char character) noexcept
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9');
}

bool CharacterFunctions::isLetterOrDigit (const juce_wchar character) noexcept
{
    return iswalnum ((wint_t) character) != 0;
}

bool CharacterFunctions::isPrintable (const char character) noexcept
{
    return (character >= ' ' && character <= '~');
}

bool CharacterFunctions::isPrintable (const juce_wchar character) noexcept
{
    return iswprint ((wint_t) character) != 0;
}

int CharacterFunctions::getHexDigitValue (const juce_wchar digit) noexcept
{
    unsigned int d = (unsigned int) digit - '0';
    if (d < (unsigned int) 10)
        return (int) d;

    d += (unsigned int) ('0' - 'a');
    if (d < (unsigned int) 6)
        return (int) d + 10;

    d += (unsigned int) ('a' - 'A');
    if (d < (unsigned int) 6)
        return (int) d + 10;

    return -1;
}

double CharacterFunctions::mulexp10 (const double value, int exponent) noexcept
{
    if (exponent == 0)
        return value;

    if (value == 0)
        return 0;

    const bool negative = (exponent < 0);
    if (negative)
        exponent = -exponent;

    double result = 1.0, power = 10.0;
    for (int bit = 1; exponent != 0; bit <<= 1)
    {
        if ((exponent & bit) != 0)
        {
            exponent ^= bit;
            result *= power;
            if (exponent == 0)
                break;
        }
        power *= power;
    }

    return negative ? (value / result) : (value * result);
}

juce_wchar CharacterFunctions::getUnicodeCharFromWindows1252Codepage (const uint8 c) noexcept
{
    if (c < 0x80 || c >= 0xa0)
        return (juce_wchar) c;

    static const uint16 lookup[] = { 0x20AC, 0x0007, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
                                     0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0007, 0x017D, 0x0007,
                                     0x0007, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
                                     0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0007, 0x017E, 0x0178 };

    return (juce_wchar) lookup[c - 0x80];
}
