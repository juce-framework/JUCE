/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifdef _MSC_VER
  #pragma warning (disable: 4514 4996)
  #pragma warning (push)
#endif

#include "../basics/juce_StandardHeader.h"
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
#if MACOS_10_2_OR_EARLIER
    int n = 0;
    while (s[n] != 0)
        ++n;

    return n;
#else
    return (int) wcslen (s);
#endif
}

void CharacterFunctions::copy (char* dest, const char* src, const int maxChars) throw()
{
    strncpy (dest, src, maxChars);
}

void CharacterFunctions::copy (juce_wchar* dest, const juce_wchar* src, int maxChars) throw()
{
#if MACOS_10_2_OR_EARLIER
    while (--maxChars >= 0 && *src != 0)
        *dest++ = *src++;

    *dest = 0;
#else
    wcsncpy (dest, src, maxChars);
#endif
}

void CharacterFunctions::copy (juce_wchar* dest, const char* src, const int maxChars) throw()
{
    mbstowcs (dest, src, maxChars);
}

void CharacterFunctions::copy (char* dest, const juce_wchar* src, const int maxChars) throw()
{
    wcstombs (dest, src, maxChars);
}

void CharacterFunctions::append (char* dest, const char* src) throw()
{
    strcat (dest, src);
}

void CharacterFunctions::append (juce_wchar* dest, const juce_wchar* src) throw()
{
#if MACOS_10_2_OR_EARLIER
    while (*dest != 0)
        ++dest;

    while (*src != 0)
        *dest++ = *src++;

    *dest = 0;
#else
    wcscat (dest, src);
#endif
}

int CharacterFunctions::compare (const char* const s1, const char* const s2) throw()
{
    return strcmp (s1, s2);
}

int CharacterFunctions::compare (const juce_wchar* s1, const juce_wchar* s2) throw()
{
    jassert (s1 != 0 && s2 != 0);

#if MACOS_10_2_OR_EARLIER
    for (;;)
    {
        if (*s1 != *s2)
        {
            const int diff = *s1 - *s2;

            if (diff != 0)
                return diff < 0 ? -1 : 1;
        }
        else if (*s1 == 0)
            break;

        ++s1;
        ++s2;
    }

    return 0;
#else
    return wcscmp (s1, s2);
#endif
}

int CharacterFunctions::compare (const char* const s1, const char* const s2, const int maxChars) throw()
{
    jassert (s1 != 0 && s2 != 0);

    return strncmp (s1, s2, maxChars);
}

int CharacterFunctions::compare (const juce_wchar* s1, const juce_wchar* s2, int maxChars) throw()
{
    jassert (s1 != 0 && s2 != 0);

#if MACOS_10_2_OR_EARLIER
    while (--maxChars >= 0)
    {
        if (*s1 != *s2)
            return (*s1 < *s2) ? -1 : 1;
        else if (*s1 == 0)
            break;

        ++s1;
        ++s2;
    }

    return 0;
#else
    return wcsncmp (s1, s2, maxChars);
#endif
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
#if MACOS_10_2_OR_EARLIER
    while (*haystack != 0)
    {
        const juce_wchar* s1 = haystack;
        const juce_wchar* s2 = needle;

        for (;;)
        {
            if (*s2 == 0)
                return haystack;

            if (*s1 != *s2 || *s2 == 0)
                break;

            ++s1;
            ++s2;
        }

        ++haystack;
    }

    return 0;
#else
    return wcsstr (haystack, needle);
#endif
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
#if MACOS_10_2_OR_EARLIER
    const String formatTemp (format);
    size_t num = strftime ((char*) dest, maxChars, (const char*) formatTemp, tm);
    String temp ((char*) dest);
    temp.copyToBuffer (dest, num);
    dest [num] = 0;
    return (int) num;
#else
    return (int) wcsftime (dest, maxChars, format, tm);
#endif
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

double CharacterFunctions::getDoubleValue (const char* const s) throw()
{
    return atof (s);
}

double CharacterFunctions::getDoubleValue (const juce_wchar* const s) throw()
{
#if MACOS_10_2_OR_EARLIER
    String temp (s);
    return atof ((const char*) temp);
#else
    wchar_t* endChar;
    return wcstod (s, &endChar);
#endif
}

//==============================================================================
char CharacterFunctions::toUpperCase (const char character) throw()
{
    return (char) toupper (character);
}

juce_wchar CharacterFunctions::toUpperCase (const juce_wchar character) throw()
{
#if MACOS_10_2_OR_EARLIER
    return toupper ((char) character);
#else
    return towupper (character);
#endif
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
#if MACOS_10_2_OR_EARLIER
    return tolower ((char) character);
#else
    return towlower (character);
#endif
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
#if MACOS_10_2_OR_EARLIER
    return isWhitespace ((char) character);
#else
    return iswspace (character) != 0;
#endif
}

bool CharacterFunctions::isDigit (const char character) throw()
{
    return (character >= '0' && character <= '9');
}

bool CharacterFunctions::isDigit (const juce_wchar character) throw()
{
    return isdigit (character) != 0;
}

bool CharacterFunctions::isLetter (const char character) throw()
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z');
}

bool CharacterFunctions::isLetter (const juce_wchar character) throw()
{
#if MACOS_10_2_OR_EARLIER
    return isLetter ((char) character);
#else
    return iswalpha (character) != 0;
#endif
}

bool CharacterFunctions::isLetterOrDigit (const char character) throw()
{
    return (character >= 'a' && character <= 'z')
        || (character >= 'A' && character <= 'Z')
        || (character >= '0' && character <= '9');
}

bool CharacterFunctions::isLetterOrDigit (const juce_wchar character) throw()
{
#if MACOS_10_2_OR_EARLIER
    return isLetterOrDigit ((char) character);
#else
    return iswalnum (character) != 0;
#endif
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
#if MACOS_10_3_OR_EARLIER
    const String formatTemp (format);
    size_t num = vprintf ((char*) dest, maxLength, formatTemp, args);
    String temp ((char*) dest);
    temp.copyToBuffer (dest, num);
    dest [num] = 0;
    return (int) num;
#elif defined (JUCE_WIN32)
    return (int) _vsnwprintf (dest, maxLength, format, args);
#else
    return (int) vswprintf (dest, maxLength, format, args);
#endif
}


END_JUCE_NAMESPACE
