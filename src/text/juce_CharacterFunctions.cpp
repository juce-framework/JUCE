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

#ifdef _MSC_VER
  #pragma warning (disable: 4514 4996)
  #pragma warning (push)
#endif

#include "../core/juce_StandardHeader.h"
#include <cwctype>
#include <cctype>
#include <ctime>

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

BEGIN_JUCE_NAMESPACE

#include "juce_CharacterFunctions.h"
#include "juce_String.h"

//==============================================================================
int CharacterFunctions::length (const char* const s) throw()
{
    return (int) strlen (s);
}

int CharacterFunctions::length (const juce_wchar* const s) throw()
{
    return (int) wcslen (s);
}

void CharacterFunctions::copy (char* dest, const char* src, const int maxChars) throw()
{
    strncpy (dest, src, maxChars);
}

void CharacterFunctions::copy (juce_wchar* dest, const juce_wchar* src, int maxChars) throw()
{
    wcsncpy (dest, src, maxChars);
}

void CharacterFunctions::copy (juce_wchar* dest, const char* src, const int maxChars) throw()
{
    mbstowcs (dest, src, maxChars);
}

void CharacterFunctions::copy (char* dest, const juce_wchar* src, const int maxChars) throw()
{
    wcstombs (dest, src, maxChars);
}

int CharacterFunctions::bytesRequiredForCopy (const juce_wchar* src) throw()
{
    return (int) wcstombs (0, src, 0);
}

void CharacterFunctions::append (char* dest, const char* src) throw()
{
    strcat (dest, src);
}

void CharacterFunctions::append (juce_wchar* dest, const juce_wchar* src) throw()
{
    wcscat (dest, src);
}

int CharacterFunctions::compare (const char* const s1, const char* const s2) throw()
{
    return strcmp (s1, s2);
}

int CharacterFunctions::compare (const juce_wchar* s1, const juce_wchar* s2) throw()
{
    jassert (s1 != 0 && s2 != 0);
    return wcscmp (s1, s2);
}

int CharacterFunctions::compare (const char* const s1, const char* const s2, const int maxChars) throw()
{
    jassert (s1 != 0 && s2 != 0);
    return strncmp (s1, s2, maxChars);
}

int CharacterFunctions::compare (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw()
{
    jassert (s1 != 0 && s2 != 0);
    return wcsncmp (s1, s2, maxChars);
}

int CharacterFunctions::compareIgnoreCase (const char* const s1, const char* const s2) throw()
{
    jassert (s1 != 0 && s2 != 0);

#if JUCE_WIN32
    return stricmp (s1, s2);
#else
    return strcasecmp (s1, s2);
#endif
}

int CharacterFunctions::compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2) throw()
{
    jassert (s1 != 0 && s2 != 0);

#if JUCE_WIN32
    return _wcsicmp (s1, s2);
#else
    for (;;)
    {
        if (*s1 != *s2)
        {
            const int diff = toUpperCase (*s1) - toUpperCase (*s2);

            if (diff != 0)
                return diff < 0 ? -1 : 1;
        }
        else if (*s1 == 0)
            break;

        ++s1;
        ++s2;
    }

    return 0;
#endif
}

int CharacterFunctions::compareIgnoreCase (const char* const s1, const char* const s2, const int maxChars) throw()
{
    jassert (s1 != 0 && s2 != 0);

#if JUCE_WIN32
    return strnicmp (s1, s2, maxChars);
#else
    return strncasecmp (s1, s2, maxChars);
#endif
}

int CharacterFunctions::compareIgnoreCase (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw()
{
    jassert (s1 != 0 && s2 != 0);

#if JUCE_WIN32
    return _wcsnicmp (s1, s2, maxChars);
#else
    while (--maxChars >= 0)
    {
        if (*s1 != *s2)
        {
            const int diff = toUpperCase (*s1) - toUpperCase (*s2);

            if (diff != 0)
                return diff < 0 ? -1 : 1;
        }
        else if (*s1 == 0)
            break;

        ++s1;
        ++s2;
    }

    return 0;
#endif
}

const char* CharacterFunctions::find (const char* const haystack, const char* const needle) throw()
{
    return strstr (haystack, needle);
}

const juce_wchar* CharacterFunctions::find (const juce_wchar* haystack, const juce_wchar* const needle) throw()
{
    return wcsstr (haystack, needle);
}

int CharacterFunctions::indexOfChar (const char* const haystack, const char needle, const bool ignoreCase) throw()
{
    if (haystack != 0)
    {
        int i = 0;

        if (ignoreCase)
        {
            const char n1 = toLowerCase (needle);
            const char n2 = toUpperCase (needle);

            if (n1 != n2) // if the char is the same in upper/lower case, fall through to the normal search
            {
                while (haystack[i] != 0)
                {
                    if (haystack[i] == n1 || haystack[i] == n2)
                        return i;

                    ++i;
                }

                return -1;
            }

            jassert (n1 == needle);
        }

        while (haystack[i] != 0)
        {
            if (haystack[i] == needle)
                return i;

            ++i;
        }
    }

    return -1;
}

int CharacterFunctions::indexOfChar (const juce_wchar* const haystack, const juce_wchar needle, const bool ignoreCase) throw()
{
    if (haystack != 0)
    {
        int i = 0;

        if (ignoreCase)
        {
            const juce_wchar n1 = toLowerCase (needle);
            const juce_wchar n2 = toUpperCase (needle);

            if (n1 != n2) // if the char is the same in upper/lower case, fall through to the normal search
            {
                while (haystack[i] != 0)
                {
                    if (haystack[i] == n1 || haystack[i] == n2)
                        return i;

                    ++i;
                }

                return -1;
            }

            jassert (n1 == needle);
        }

        while (haystack[i] != 0)
        {
            if (haystack[i] == needle)
                return i;

            ++i;
        }
    }

    return -1;
}

int CharacterFunctions::indexOfCharFast (const char* const haystack, const char needle) throw()
{
    jassert (haystack != 0);

    int i = 0;
    while (haystack[i] != 0)
    {
        if (haystack[i] == needle)
            return i;

        ++i;
    }

    return -1;
}

int CharacterFunctions::indexOfCharFast (const juce_wchar* const haystack, const juce_wchar needle) throw()
{
    jassert (haystack != 0);

    int i = 0;
    while (haystack[i] != 0)
    {
        if (haystack[i] == needle)
            return i;

        ++i;
    }

    return -1;
}

int CharacterFunctions::getIntialSectionContainingOnly (const char* const text, const char* const allowedChars) throw()
{
    return allowedChars == 0 ? 0 : (int) strspn (text, allowedChars);
}

int CharacterFunctions::getIntialSectionContainingOnly (const juce_wchar* const text, const juce_wchar* const allowedChars) throw()
{
    if (allowedChars == 0)
        return 0;

    int i = 0;

    for (;;)
    {
        if (indexOfCharFast (allowedChars, text[i]) < 0)
            break;

        ++i;
    }

    return i;
}

int CharacterFunctions::ftime (char* const dest, const int maxChars, const char* const format, const struct tm* const tm) throw()
{
    return (int) strftime (dest, maxChars, format, tm);
}

int CharacterFunctions::ftime (juce_wchar* const dest, const int maxChars, const juce_wchar* const format, const struct tm* const tm) throw()
{
    return (int) wcsftime (dest, maxChars, format, tm);
}

int CharacterFunctions::getIntValue (const char* const s) throw()
{
    return atoi (s);
}

int CharacterFunctions::getIntValue (const juce_wchar* s) throw()
{
#if JUCE_WIN32
    return _wtoi (s);
#else
    int v = 0;

    while (isWhitespace (*s))
        ++s;

    const bool isNeg = *s == T('-');
    if (isNeg)
        ++s;

    for (;;)
    {
        const wchar_t c = *s++;

        if (c >= T('0') && c <= T('9'))
            v = v * 10 + (int) (c - T('0'));
        else
            break;
    }

    return isNeg ? -v : v;
#endif
}

int64 CharacterFunctions::getInt64Value (const char* s) throw()
{
#if JUCE_LINUX
    return atoll (s);
#elif defined (JUCE_WIN32)
    return _atoi64 (s);
#else
    int64 v = 0;

    while (isWhitespace (*s))
        ++s;

    const bool isNeg = *s == T('-');
    if (isNeg)
        ++s;

    for (;;)
    {
        const char c = *s++;

        if (c >= '0' && c <= '9')
            v = v * 10 + (int64) (c - '0');
        else
            break;
    }

    return isNeg ? -v : v;
#endif
}

int64 CharacterFunctions::getInt64Value (const juce_wchar* s) throw()
{
#if JUCE_WIN32
    return _wtoi64 (s);
#else
    int64 v = 0;

    while (isWhitespace (*s))
        ++s;

    const bool isNeg = *s == T('-');
    if (isNeg)
        ++s;

    for (;;)
    {
        const juce_wchar c = *s++;

        if (c >= T('0') && c <= T('9'))
            v = v * 10 + (int64) (c - T('0'));
        else
            break;
    }

    return isNeg ? -v : v;
#endif
}

//==============================================================================
static double juce_mulexp10 (const double value, int exponent) throw()
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

template <class CharType>
double juce_atof (const CharType* const original) throw()
{
    double result[3] = { 0, 0, 0 }, accumulator[2] = { 0, 0 };
    int exponentAdjustment[2] = { 0, 0 }, exponentAccumulator[2] = { -1, -1 };
    int exponent = 0, decPointIndex = 0, digit = 0;
    int lastDigit = 0, numSignificantDigits = 0;
    bool isNegative = false, digitsFound = false;
    const int maxSignificantDigits = 15 + 2;

    const CharType* s = original;
    while (CharacterFunctions::isWhitespace (*s))
        ++s;

    switch (*s)
    {
        case '-':   isNegative = true; // fall-through..
        case '+':   ++s;
    }

    if (*s == 'n' || *s == 'N' || *s == 'i' || *s == 'I')
        return atof (String (original)); // Let the c library deal with NAN and INF

    for (;;)
    {
        if (CharacterFunctions::isDigit (*s))
        {
            lastDigit = digit;
            digit = *s++ - '0';
            digitsFound = true;

            if (decPointIndex != 0)
                exponentAdjustment[1]++;

            if (numSignificantDigits == 0 && digit == 0)
                continue;

            if (++numSignificantDigits > maxSignificantDigits)
            {
                if (digit > 5)
                    ++accumulator [decPointIndex];
                else if (digit == 5 && (lastDigit & 1) != 0)
                    ++accumulator [decPointIndex];

                if (decPointIndex > 0)
                    exponentAdjustment[1]--;
                else
                    exponentAdjustment[0]++;

                while (CharacterFunctions::isDigit (*s))
                {
                    ++s;
                    if (decPointIndex == 0)
                        exponentAdjustment[0]++;
                }
            }
            else
            {
                const double maxAccumulatorValue = (double) ((UINT_MAX - 9) / 10);
                if (accumulator [decPointIndex] > maxAccumulatorValue)
                {
                    result [decPointIndex] = juce_mulexp10 (result [decPointIndex], exponentAccumulator [decPointIndex])
                                                + accumulator [decPointIndex];
                    accumulator [decPointIndex] = 0;
                    exponentAccumulator [decPointIndex] = 0;
                }

                accumulator [decPointIndex] = accumulator[decPointIndex] * 10 + digit;
                exponentAccumulator [decPointIndex]++;
            }
        }
        else if (decPointIndex == 0 && *s == '.')
        {
            ++s;
            decPointIndex = 1;

            if (numSignificantDigits > maxSignificantDigits)
            {
                while (CharacterFunctions::isDigit (*s))
                    ++s;
                break;
            }
        }
        else
        {
            break;
        }
    }

    result[0] = juce_mulexp10 (result[0], exponentAccumulator[0]) + accumulator[0];

    if (decPointIndex != 0)
        result[1] = juce_mulexp10 (result[1], exponentAccumulator[1]) + accumulator[1];

    if ((*s == 'e' || *s == 'E') && digitsFound)
    {
        bool negativeExponent = false;

        switch (*++s)
        {
            case '-':   negativeExponent = true; // fall-through..
            case '+':   ++s;
        }

        while (CharacterFunctions::isDigit (*s))
            exponent = (exponent * 10) + (*s++ - '0');

        if (negativeExponent)
            exponent = -exponent;
    }

    double r = juce_mulexp10 (result[0], exponent + exponentAdjustment[0]);
    if (decPointIndex != 0)
        r += juce_mulexp10 (result[1], exponent - exponentAdjustment[1]);

    return isNegative ? -r : r;
}

double CharacterFunctions::getDoubleValue (const char* const s) throw()
{
    return juce_atof <char> (s);
}

double CharacterFunctions::getDoubleValue (const juce_wchar* const s) throw()
{
    return juce_atof <juce_wchar> (s);
}

//==============================================================================
char CharacterFunctions::toUpperCase (const char character) throw()
{
    return (char) toupper (character);
}

juce_wchar CharacterFunctions::toUpperCase (const juce_wchar character) throw()
{
    return towupper (character);
}

void CharacterFunctions::toUpperCase (char* s) throw()
{
#if JUCE_WIN32
    strupr (s);
#else
    while (*s != 0)
    {
        *s = toUpperCase (*s);
        ++s;
    }
#endif
}

void CharacterFunctions::toUpperCase (juce_wchar* s) throw()
{
#if JUCE_WIN32
    _wcsupr (s);
#else
    while (*s != 0)
    {
        *s = toUpperCase (*s);
        ++s;
    }
#endif
}

bool CharacterFunctions::isUpperCase (const char character) throw()
{
    return isupper (character) != 0;
}

bool CharacterFunctions::isUpperCase (const juce_wchar character) throw()
{
#if JUCE_WIN32
    return iswupper (character) != 0;
#else
    return toLowerCase (character) != character;
#endif
}

//==============================================================================
char CharacterFunctions::toLowerCase (const char character) throw()
{
    return (char) tolower (character);
}

juce_wchar CharacterFunctions::toLowerCase (const juce_wchar character) throw()
{
    return towlower (character);
}

void CharacterFunctions::toLowerCase (char* s) throw()
{
#if JUCE_WIN32
    strlwr (s);
#else
    while (*s != 0)
    {
        *s = toLowerCase (*s);
        ++s;
    }
#endif
}

void CharacterFunctions::toLowerCase (juce_wchar* s) throw()
{
#if JUCE_WIN32
    _wcslwr (s);
#else
    while (*s != 0)
    {
        *s = toLowerCase (*s);
        ++s;
    }
#endif
}

bool CharacterFunctions::isLowerCase (const char character) throw()
{
    return islower (character) != 0;
}

bool CharacterFunctions::isLowerCase (const juce_wchar character) throw()
{
#if JUCE_WIN32
    return iswlower (character) != 0;
#else
    return toUpperCase (character) != character;
#endif
}

//==============================================================================
bool CharacterFunctions::isWhitespace (const char character) throw()
{
    return character == T(' ') || (character <= 13 && character >= 9);
}

bool CharacterFunctions::isWhitespace (const juce_wchar character) throw()
{
    return iswspace (character) != 0;
}

bool CharacterFunctions::isDigit (const char character) throw()
{
    return (character >= '0' && character <= '9');
}

bool CharacterFunctions::isDigit (const juce_wchar character) throw()
{
    return iswdigit (character) != 0;
}

bool CharacterFunctions::isLetter (const char character) throw()
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z');
}

bool CharacterFunctions::isLetter (const juce_wchar character) throw()
{
    return iswalpha (character) != 0;
}

bool CharacterFunctions::isLetterOrDigit (const char character) throw()
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9');
}

bool CharacterFunctions::isLetterOrDigit (const juce_wchar character) throw()
{
    return iswalnum (character) != 0;
}

int CharacterFunctions::getHexDigitValue (const tchar digit) throw()
{
    if (digit >= T('0') && digit <= T('9'))
        return digit - T('0');
    else if (digit >= T('a') && digit <= T('f'))
        return digit - (T('a') - 10);
    else if (digit >= T('A') && digit <= T('F'))
        return digit - (T('A') - 10);

    return -1;
}

//==============================================================================
int CharacterFunctions::printf (char* const dest, const int maxLength, const char* const format, ...) throw()
{
    va_list list;
    va_start (list, format);
    return vprintf (dest, maxLength, format, list);
}

int CharacterFunctions::printf (juce_wchar* const dest, const int maxLength, const juce_wchar* const format, ...) throw()
{
    va_list list;
    va_start (list, format);
    return vprintf (dest, maxLength, format, list);
}

int CharacterFunctions::vprintf (char* const dest, const int maxLength, const char* const format, va_list& args) throw()
{
#if JUCE_WIN32
    return (int) _vsnprintf (dest, maxLength, format, args);
#else
    return (int) vsnprintf (dest, maxLength, format, args);
#endif
}

int CharacterFunctions::vprintf (juce_wchar* const dest, const int maxLength, const juce_wchar* const format, va_list& args) throw()
{
#if defined (JUCE_WIN32)
    return (int) _vsnwprintf (dest, maxLength, format, args);
#else
    return (int) vswprintf (dest, maxLength, format, args);
#endif
}


END_JUCE_NAMESPACE
