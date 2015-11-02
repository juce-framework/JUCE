/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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
