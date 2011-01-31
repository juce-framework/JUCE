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

#include "../core/juce_StandardHeader.h"

#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4514 4996)
#endif

#if ! JUCE_ANDROID
 #include <cwctype>
#endif

#include <cctype>
#include <ctime>

BEGIN_JUCE_NAMESPACE

#include "juce_String.h"
#include "../memory/juce_HeapBlock.h"

//==============================================================================
juce_wchar CharacterFunctions::toUpperCase (const juce_wchar character) throw()
{
    return towupper ((wchar_t) character);
}

juce_wchar CharacterFunctions::toLowerCase (const juce_wchar character) throw()
{
    return towlower ((wchar_t) character);
}

bool CharacterFunctions::isUpperCase (const juce_wchar character) throw()
{
   #if JUCE_WINDOWS
    return iswupper ((wchar_t) character) != 0;
   #else
    return toLowerCase (character) != character;
   #endif
}

bool CharacterFunctions::isLowerCase (const juce_wchar character) throw()
{
   #if JUCE_WINDOWS
    return iswlower ((wchar_t) character) != 0;
   #else
    return toUpperCase (character) != character;
   #endif
}

//==============================================================================
bool CharacterFunctions::isWhitespace (const char character) throw()
{
    return character == ' ' || (character <= 13 && character >= 9);
}

bool CharacterFunctions::isWhitespace (const juce_wchar character) throw()
{
    return iswspace ((wchar_t) character) != 0;
}

bool CharacterFunctions::isDigit (const char character) throw()
{
    return (character >= '0' && character <= '9');
}

bool CharacterFunctions::isDigit (const juce_wchar character) throw()
{
    return iswdigit ((wchar_t) character) != 0;
}

bool CharacterFunctions::isLetter (const char character) throw()
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z');
}

bool CharacterFunctions::isLetter (const juce_wchar character) throw()
{
    return iswalpha ((wchar_t) character) != 0;
}

bool CharacterFunctions::isLetterOrDigit (const char character) throw()
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9');
}

bool CharacterFunctions::isLetterOrDigit (const juce_wchar character) throw()
{
    return iswalnum ((wchar_t) character) != 0;
}

int CharacterFunctions::getHexDigitValue (const juce_wchar digit) throw()
{
    unsigned int d = digit - '0';
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

int CharacterFunctions::ftime (char* const dest, const int maxChars, const char* const format, const struct tm* const tm) throw()
{
    return (int) strftime (dest, maxChars, format, tm);
}

int CharacterFunctions::ftime (juce_wchar* const dest, const int maxChars, const juce_wchar* const format, const struct tm* const tm) throw()
{
   #if JUCE_NATIVE_WCHAR_IS_NOT_UTF32
    HeapBlock <char> tempDest;
    tempDest.calloc (maxChars + 2);
    int result = ftime (tempDest.getData(), maxChars, String (format).toUTF8(), tm);
    CharPointer_UTF32 (dest).writeAll (CharPointer_UTF8 (tempDest.getData()));
    return result;
   #else
    return (int) wcsftime (dest, maxChars, format, tm);
   #endif
}

#if JUCE_MSVC
  #pragma warning (pop)
#endif

double CharacterFunctions::mulexp10 (const double value, int exponent) throw()
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


END_JUCE_NAMESPACE
