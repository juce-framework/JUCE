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
  #pragma warning (disable: 4514)
  #pragma warning (push)
#endif
#include <locale>

#include "../core/juce_StandardHeader.h"

#if JUCE_MSVC
  #include <float.h>
#endif

BEGIN_JUCE_NAMESPACE

#include "juce_String.h"
#include "../core/juce_Atomic.h"
#include "../io/streams/juce_OutputStream.h"

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

#if defined (JUCE_STRINGS_ARE_UNICODE) && ! JUCE_STRINGS_ARE_UNICODE
 #error "JUCE_STRINGS_ARE_UNICODE is deprecated! All strings are now unicode by default."
#endif

//==============================================================================
static const char* const emptyCharString                        = "\0\0\0\0JUCE";
static const int safeEmptyStringRefCount                        = 0x3fffffff;
String::InternalRefCountedStringHolder String::emptyString      = { safeEmptyStringRefCount, 0, { 0 } };
static tchar decimalPoint                                       = T('.');

void juce_initialiseStrings()
{
    decimalPoint = String::fromUTF8 (localeconv()->decimal_point) [0];
}

//==============================================================================
void String::createInternal (const int numChars) throw()
{
    jassert (numChars > 0);

    text = (InternalRefCountedStringHolder*) juce_malloc (sizeof (InternalRefCountedStringHolder)
                                                           + numChars * sizeof (tchar));
    text->refCount = 1;
    text->allocatedNumChars = numChars;
    text->text[0] = 0;
}

void String::createInternal (const tchar* const t, const tchar* const textEnd) throw()
{
    jassert (*(textEnd - 1) == 0); // must have a null terminator

    const int numChars = (int) (textEnd - t);
    createInternal (numChars - 1);
    memcpy (text->text, t, numChars * sizeof (tchar));
}

void String::appendInternal (const tchar* const newText,
                             const int numExtraChars) throw()
{
    if (numExtraChars > 0)
    {
        const int oldLen = CharacterFunctions::length (text->text);
        const int newTotalLen = oldLen + numExtraChars;

        if (text->refCount > 1)
        {
            // it's in use by other strings as well, so we need to make a private copy before messing with it..
            InternalRefCountedStringHolder* const newTextHolder
                = (InternalRefCountedStringHolder*) juce_malloc (sizeof (InternalRefCountedStringHolder)
                                                                  + newTotalLen * sizeof (tchar));
            newTextHolder->refCount = 1;
            newTextHolder->allocatedNumChars = newTotalLen;

            memcpy (newTextHolder->text, text->text, oldLen * sizeof (tchar));
            memcpy (newTextHolder->text + oldLen, newText, numExtraChars * sizeof (tchar));

            InternalRefCountedStringHolder* const old = text;
            text = newTextHolder;

            if (Atomic::decrementAndReturn (old->refCount) == 0)
                juce_free (old);
        }
        else
        {
            // no other strings using it, so just expand it if needed..
            if (newTotalLen > text->allocatedNumChars)
            {
                text = (InternalRefCountedStringHolder*)
                            juce_realloc (text, sizeof (InternalRefCountedStringHolder)
                                               + newTotalLen * sizeof (tchar));

                text->allocatedNumChars = newTotalLen;
            }

            memcpy (text->text + oldLen, newText, numExtraChars * sizeof (tchar));
        }

        text->text [newTotalLen] = 0;
    }
}

void String::dupeInternalIfMultiplyReferenced() throw()
{
    if (text->refCount > 1)
    {
        InternalRefCountedStringHolder* const old = text;
        const int len = old->allocatedNumChars;

        InternalRefCountedStringHolder* const newTextHolder
            = (InternalRefCountedStringHolder*) juce_malloc (sizeof (InternalRefCountedStringHolder)
                                                                + len * sizeof (tchar));

        newTextHolder->refCount = 1;
        newTextHolder->allocatedNumChars = len;

        memcpy (newTextHolder->text, old->text, (len + 1) * sizeof (tchar));

        text = newTextHolder;

        if (Atomic::decrementAndReturn (old->refCount) == 0)
            juce_free (old);
    }
}

//==============================================================================
const String String::empty;


//==============================================================================
String::String() throw()
    : text (&emptyString)
{
}

String::String (const String& other) throw()
    : text (other.text)
{
    Atomic::increment (text->refCount);
}

String::String (const int numChars,
                const int /*dummyVariable*/) throw()
{
    createInternal (numChars);
}

String::String (const char* const t) throw()
{
    if (t != 0 && *t != 0)
    {
        const int len = CharacterFunctions::length (t);
        createInternal (len);

        CharacterFunctions::copy (text->text, t, len + 1);
    }
    else
    {
        text = &emptyString;
        emptyString.refCount = safeEmptyStringRefCount;
    }
}

String::String (const juce_wchar* const t) throw()
{
    if (t != 0 && *t != 0)
    {
        const int len = CharacterFunctions::length (t);
        createInternal (len);
        memcpy (text->text, t, (len + 1) * sizeof (tchar));
    }
    else
    {
        text = &emptyString;
        emptyString.refCount = safeEmptyStringRefCount;
    }
}

String::String (const char* const t,
                const size_t maxChars) throw()
{
    int i;
    for (i = 0; (size_t) i < maxChars; ++i)
        if (t[i] == 0)
            break;

    if (i > 0)
    {
        createInternal (i);
        CharacterFunctions::copy (text->text, t, i);
        text->text [i] = 0;
    }
    else
    {
        text = &emptyString;
        emptyString.refCount = safeEmptyStringRefCount;
    }
}

String::String (const juce_wchar* const t,
                const size_t maxChars) throw()
{
    int i;
    for (i = 0; (size_t) i < maxChars; ++i)
        if (t[i] == 0)
            break;

    if (i > 0)
    {
        createInternal (i);
        memcpy (text->text, t, i * sizeof (tchar));
        text->text [i] = 0;
    }
    else
    {
        text = &emptyString;
        emptyString.refCount = safeEmptyStringRefCount;
    }
}

const String String::charToString (const juce_wchar character) throw()
{
    tchar temp[2];
    temp[0] = character;
    temp[1] = 0;

    return String (temp);
}

namespace IntToCharConverters
{
    // pass in a pointer to the END of a buffer..
    static tchar* int64ToString (tchar* t, const int64 n) throw()
    {
        *--t = 0;
        int64 v = (n >= 0) ? n : -n;

        do
        {
            *--t = (tchar) (T('0') + (int) (v % 10));
            v /= 10;

        } while (v > 0);

        if (n < 0)
            *--t = T('-');

        return t;
    }

    static tchar* uint64ToString (tchar* t, int64 v) throw()
    {
        *--t = 0;

        do
        {
            *--t = (tchar) (T('0') + (int) (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    static tchar* intToString (tchar* t, const int n) throw()
    {
        if (n == (int) 0x80000000) // (would cause an overflow)
            return int64ToString (t, n);

        *--t = 0;
        int v = abs (n);

        do
        {
            *--t = (tchar) (T('0') + (v % 10));
            v /= 10;

        } while (v > 0);

        if (n < 0)
            *--t = T('-');

        return t;
    }

    static tchar* uintToString (tchar* t, unsigned int v) throw()
    {
        *--t = 0;

        do
        {
            *--t = (tchar) (T('0') + (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }
}

String::String (const int number) throw()
{
    tchar buffer [16];
    tchar* const end = buffer + numElementsInArray (buffer);

    createInternal (IntToCharConverters::intToString (end, number), end);
}

String::String (const unsigned int number) throw()
{
    tchar buffer [16];
    tchar* const end = buffer + numElementsInArray (buffer);

    createInternal (IntToCharConverters::uintToString (end, number), end);
}

String::String (const short number) throw()
{
    tchar buffer [16];
    tchar* const end = buffer + numElementsInArray (buffer);

    createInternal (IntToCharConverters::intToString (end, (int) number), end);
}

String::String (const unsigned short number) throw()
{
    tchar buffer [16];
    tchar* const end = buffer + numElementsInArray (buffer);

    createInternal (IntToCharConverters::uintToString (end, (unsigned int) number), end);
}

String::String (const int64 number) throw()
{
    tchar buffer [32];
    tchar* const end = buffer + numElementsInArray (buffer);

    createInternal (IntToCharConverters::int64ToString (end, number), end);
}

String::String (const uint64 number) throw()
{
    tchar buffer [32];
    tchar* const end = buffer + numElementsInArray (buffer);

    createInternal (IntToCharConverters::uint64ToString (end, number), end);
}

// a double-to-string routine that actually uses the number of dec. places you asked for
// without resorting to exponent notation if the number's too big or small (which is what printf does).
void String::doubleToStringWithDecPlaces (double n, int numDecPlaces) throw()
{
    const int bufSize = 80;
    tchar buffer [bufSize];
    int len;
    tchar* t;

    if (numDecPlaces > 0 && n > -1.0e20 && n < 1.0e20)
    {
        int64 v = (int64) (pow (10.0, numDecPlaces) * fabs (n) + 0.5);

        t = buffer + bufSize;
        *--t = (tchar) 0;

        while (numDecPlaces >= 0 || v > 0)
        {
            if (numDecPlaces == 0)
                *--t = decimalPoint;

            *--t = (tchar) (T('0') + (v % 10));

            v /= 10;
            --numDecPlaces;
        }

        if (n < 0)
            *--t = T('-');

        len = (int) ((buffer + bufSize) - t);
    }
    else
    {
        len = CharacterFunctions::printf (buffer, bufSize, T("%.9g"), n) + 1;
        t = buffer;
    }

    if (len > 1)
    {
        jassert (len < numElementsInArray (buffer));

        createInternal (len - 1);
        memcpy (text->text, t, len * sizeof (tchar));
    }
    else
    {
        jassert (*t == 0);
        text = &emptyString;
        emptyString.refCount = safeEmptyStringRefCount;
    }
}

String::String (const float number,
                const int numberOfDecimalPlaces) throw()
{
    doubleToStringWithDecPlaces ((double) number,
                                 numberOfDecimalPlaces);
}

String::String (const double number,
                const int numberOfDecimalPlaces) throw()
{
    doubleToStringWithDecPlaces (number,
                                 numberOfDecimalPlaces);
}

String::~String() throw()
{
    emptyString.refCount = safeEmptyStringRefCount;

    if (Atomic::decrementAndReturn (text->refCount) == 0)
        juce_free (text);
}

//==============================================================================
void String::preallocateStorage (const size_t numChars) throw()
{
    if (numChars > (size_t) text->allocatedNumChars)
    {
        dupeInternalIfMultiplyReferenced();

        text = (InternalRefCountedStringHolder*) juce_realloc (text, sizeof (InternalRefCountedStringHolder)
                                                                       + numChars * sizeof (tchar));
        text->allocatedNumChars = (int) numChars;
    }
}

//==============================================================================
int String::length() const throw()
{
    return CharacterFunctions::length (text->text);
}

int String::hashCode() const throw()
{
    const tchar* t = text->text;
    int result = 0;

    while (*t != (tchar) 0)
        result = 31 * result + *t++;

    return result;
}

int64 String::hashCode64() const throw()
{
    const tchar* t = text->text;
    int64 result = 0;

    while (*t != (tchar) 0)
        result = 101 * result + *t++;

    return result;
}

//==============================================================================
String& String::operator= (const String& other) throw()
{
    if (this != &other)
    {
        InternalRefCountedStringHolder* newText = other.text;
        Atomic::increment (newText->refCount);

        InternalRefCountedStringHolder* oldText
            = (InternalRefCountedStringHolder*) Atomic::swapPointers ((void* volatile*) &text, newText);

        if (Atomic::decrementAndReturn (oldText->refCount) == 0)
            juce_free (oldText);
    }

    return *this;
}

//==============================================================================
bool operator== (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) == 0;
}

bool operator== (const String& string1, const char* string2) throw()
{
    return string1.compare (string2) == 0;
}

bool operator== (const String& string1, const juce_wchar* string2) throw()
{
    return string1.compare (string2) == 0;
}

bool operator!= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) != 0;
}

bool operator!= (const String& string1, const char* string2) throw()
{
    return string1.compare (string2) != 0;
}

bool operator!= (const String& string1, const juce_wchar* string2) throw()
{
    return string1.compare (string2) != 0;
}

bool operator>  (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) > 0;
}

bool operator<  (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) < 0;
}

bool operator>= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) >= 0;
}

bool operator<= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) <= 0;
}

bool String::equalsIgnoreCase (const tchar* t) const throw()
{
    return t != 0 ? CharacterFunctions::compareIgnoreCase (text->text, t) == 0
                  : isEmpty();
}

bool String::equalsIgnoreCase (const String& other) const throw()
{
    return text == other.text
            || CharacterFunctions::compareIgnoreCase (text->text, other.text->text) == 0;
}

int String::compare (const String& other) const throw()
{
    return (text == other.text) ? 0 : CharacterFunctions::compare (text->text, other.text->text);
}

int String::compare (const char* other) const throw()
{
    return other == 0 ? isEmpty() : CharacterFunctions::compare (text->text, other);
}

int String::compare (const juce_wchar* other) const throw()
{
    return other == 0 ? isEmpty() : CharacterFunctions::compare (text->text, other);
}

int String::compareIgnoreCase (const String& other) const throw()
{
    return (text == other.text) ? 0 : CharacterFunctions::compareIgnoreCase (text->text, other.text->text);
}

int String::compareLexicographically (const String& other) const throw()
{
    const tchar* s1 = text->text;
    while (*s1 != 0 && ! CharacterFunctions::isLetterOrDigit (*s1))
        ++s1;

    const tchar* s2 = other.text->text;
    while (*s2 != 0 && ! CharacterFunctions::isLetterOrDigit (*s2))
        ++s2;

    return CharacterFunctions::compareIgnoreCase (s1, s2);
}

//==============================================================================
String& String::operator+= (const tchar* const t)
{
    if (t != 0)
        appendInternal (t, CharacterFunctions::length (t));

    return *this;
}

String& String::operator+= (const String& other)
{
    if (isEmpty())
        operator= (other);
    else
        appendInternal (other.text->text, other.length());

    return *this;
}

String& String::operator+= (const char ch)
{
    const tchar asString[] = { (tchar) ch, 0 };
    return operator+= ((const tchar*) asString);
}

String& String::operator+= (const juce_wchar ch)
{
    const tchar asString[] = { (tchar) ch, 0 };
    return operator+= ((const tchar*) asString);
}

String& String::operator+= (const int number)
{
    tchar buffer [16];
    tchar* const end = buffer + numElementsInArray (buffer);
    tchar* const start = IntToCharConverters::intToString (end, number);
    appendInternal (start, end - start);
    return *this;
}

String& String::operator+= (const unsigned int number)
{
    tchar buffer [16];
    tchar* const end = buffer + numElementsInArray (buffer);
    tchar* const start = IntToCharConverters::uintToString (end, number);
    appendInternal (start, end - start);
    return *this;
}

void String::append (const tchar* const other, const int howMany)
{
    if (howMany > 0)
    {
        int i;
        for (i = 0; i < howMany; ++i)
            if (other[i] == 0)
                break;

        appendInternal (other, i);
    }
}

//==============================================================================
const String operator+ (const char* const string1, const String& string2)
{
    String s (string1);
    return s += string2;
}

const String operator+ (const juce_wchar* const string1, const String& string2)
{
    String s (string1);
    return s += string2;
}

const String operator+ (const char string1, const String& string2)
{
    return String::charToString (string1) + string2;
}

const String operator+ (const juce_wchar string1, const String& string2)
{
    return String::charToString (string1) + string2;
}

const String operator+ (String string1, const String& string2)
{
    return string1 += string2;
}

const String operator+ (String string1, const char* const string2)
{
    return string1 += string2;
}

const String operator+ (String string1, const juce_wchar* const string2)
{
    return string1 += string2;
}

const String operator+ (String string1, const char string2)
{
    return string1 += string2;
}

const String operator+ (String string1, const juce_wchar string2)
{
    return string1 += string2;
}

String& operator<< (String& string1, const char characterToAppend)
{
    return string1 += characterToAppend;
}

String& operator<< (String& string1, const juce_wchar characterToAppend)
{
    return string1 += characterToAppend;
}

String& operator<< (String& string1, const char* const string2)
{
    return string1 += string2;
}

String& operator<< (String& string1, const juce_wchar* const string2)
{
    return string1 += string2;
}

String& operator<< (String& string1, const String& string2)
{
    return string1 += string2;
}

String& operator<< (String& string1, const short number)
{
    return string1 += (int) number;
}

String& operator<< (String& string1, const int number)
{
    return string1 += number;
}

String& operator<< (String& string1, const unsigned int number)
{
    return string1 += number;
}

String& operator<< (String& string1, const long number)
{
    return string1 += (int) number;
}

String& operator<< (String& string1, const unsigned long number)
{
    return string1 += (unsigned int) number;
}

String& operator<< (String& string1, const float number)
{
    return string1 += String (number);
}

String& operator<< (String& string1, const double number)
{
    return string1 += String (number);
}

OutputStream& operator<< (OutputStream& stream, const String& text)
{
    // (This avoids using toUTF8() to prevent the memory bloat that it would leave behind
    // if lots of large, persistent strings were to be written to streams).
    const int numBytes = text.getNumBytesAsUTF8();
    HeapBlock<char> temp (numBytes + 1);
    text.copyToUTF8 (temp, numBytes + 1);
    stream.write (temp, numBytes);
    return stream;
}

//==============================================================================
int String::indexOfChar (const tchar character) const throw()
{
    const tchar* t = text->text;

    for (;;)
    {
        if (*t == character)
            return (int) (t - text->text);

        if (*t++ == 0)
            return -1;
    }
}

int String::lastIndexOfChar (const tchar character) const throw()
{
    for (int i = CharacterFunctions::length (text->text); --i >= 0;)
        if (text->text[i] == character)
            return i;

    return -1;
}

int String::indexOf (const tchar* const t) const throw()
{
    const tchar* const r = CharacterFunctions::find (text->text, t);
    return (r == 0) ? -1
                    : (int) (r - text->text);
}

int String::indexOfChar (const int startIndex,
                         const tchar character) const throw()
{
    if (startIndex >= 0 && startIndex >= CharacterFunctions::length (text->text))
        return -1;

    const tchar* t = text->text + jmax (0, startIndex);

    for (;;)
    {
        if (*t == character)
            return (int) (t - text->text);

        if (*t++ == 0)
            return -1;
    }
}

int String::indexOfAnyOf (const tchar* const charactersToLookFor,
                          const int startIndex,
                          const bool ignoreCase) const throw()
{
    if (charactersToLookFor == 0
         || (startIndex >= 0 && startIndex >= CharacterFunctions::length (text->text)))
        return -1;

    const tchar* t = text->text + jmax (0, startIndex);

    while (*t != 0)
    {
        if (CharacterFunctions::indexOfChar (charactersToLookFor, *t, ignoreCase) >= 0)
            return (int) (t - text->text);

        ++t;
    }

    return -1;
}

int String::indexOf (const int startIndex,
                     const tchar* const other) const throw()
{
    if (other == 0 || startIndex >= CharacterFunctions::length (text->text))
        return -1;

    const tchar* const found = CharacterFunctions::find (text->text + jmax (0, startIndex),
                                                         other);

    return (found == 0) ? -1
                        : (int) (found - text->text);
}

int String::indexOfIgnoreCase (const tchar* const other) const throw()
{
    if (other != 0 && *other != 0)
    {
        const int len = CharacterFunctions::length (other);
        const int end = CharacterFunctions::length (text->text) - len;

        for (int i = 0; i <= end; ++i)
            if (CharacterFunctions::compareIgnoreCase (text->text + i, other, len) == 0)
                return i;
    }

    return -1;
}

int String::indexOfIgnoreCase (const int startIndex,
                               const tchar* const other) const throw()
{
    if (other != 0 && *other != 0)
    {
        const int len = CharacterFunctions::length (other);
        const int end = length() - len;

        for (int i = jmax (0, startIndex); i <= end; ++i)
            if (CharacterFunctions::compareIgnoreCase (text->text + i, other, len) == 0)
                return i;
    }

    return -1;
}

int String::lastIndexOf (const tchar* const other) const throw()
{
    if (other != 0 && *other != 0)
    {
        const int len = CharacterFunctions::length (other);
        int i = length() - len;

        if (i >= 0)
        {
            const tchar* n = text->text + i;

            while (i >= 0)
            {
                if (CharacterFunctions::compare (n--, other, len) == 0)
                    return i;

                --i;
            }
        }
    }

    return -1;
}

int String::lastIndexOfIgnoreCase (const tchar* const other) const throw()
{
    if (other != 0 && *other != 0)
    {
        const int len = CharacterFunctions::length (other);
        int i = length() - len;

        if (i >= 0)
        {
            const tchar* n = text->text + i;

            while (i >= 0)
            {
                if (CharacterFunctions::compareIgnoreCase (n--, other, len) == 0)
                    return i;

                --i;
            }
        }
    }

    return -1;
}

int String::lastIndexOfAnyOf (const tchar* const charactersToLookFor,
                              const bool ignoreCase) const throw()
{
    for (int i = CharacterFunctions::length (text->text); --i >= 0;)
        if (CharacterFunctions::indexOfChar (charactersToLookFor, text->text [i], ignoreCase) >= 0)
            return i;

    return -1;
}

bool String::contains (const tchar* const other) const throw()
{
    return indexOf (other) >= 0;
}

bool String::containsChar (const tchar character) const throw()
{
    return indexOfChar (character) >= 0;
}

bool String::containsIgnoreCase (const tchar* const t) const throw()
{
    return indexOfIgnoreCase (t) >= 0;
}

int String::indexOfWholeWord (const tchar* const word) const throw()
{
    if (word != 0 && *word != 0)
    {
        const int wordLen = CharacterFunctions::length (word);
        const int end = length() - wordLen;
        const tchar* t = text->text;

        for (int i = 0; i <= end; ++i)
        {
            if (CharacterFunctions::compare (t, word, wordLen) == 0
                  && (i == 0 || ! CharacterFunctions::isLetterOrDigit (* (t - 1)))
                  && ! CharacterFunctions::isLetterOrDigit (t [wordLen]))
            {
                return i;
            }

            ++t;
        }
    }

    return -1;
}

int String::indexOfWholeWordIgnoreCase (const tchar* const word) const throw()
{
    if (word != 0 && *word != 0)
    {
        const int wordLen = CharacterFunctions::length (word);
        const int end = length() - wordLen;
        const tchar* t = text->text;

        for (int i = 0; i <= end; ++i)
        {
            if (CharacterFunctions::compareIgnoreCase (t, word, wordLen) == 0
                  && (i == 0 || ! CharacterFunctions::isLetterOrDigit (* (t - 1)))
                  && ! CharacterFunctions::isLetterOrDigit (t [wordLen]))
            {
                return i;
            }

            ++t;
        }
    }

    return -1;
}

bool String::containsWholeWord (const tchar* const wordToLookFor) const throw()
{
    return indexOfWholeWord (wordToLookFor) >= 0;
}

bool String::containsWholeWordIgnoreCase (const tchar* const wordToLookFor) const throw()
{
    return indexOfWholeWordIgnoreCase (wordToLookFor) >= 0;
}

//==============================================================================
static int indexOfMatch (const tchar* const wildcard,
                         const tchar* const test,
                         const bool ignoreCase) throw()
{
    int start = 0;

    while (test [start] != 0)
    {
        int i = 0;

        for (;;)
        {
            const tchar wc = wildcard [i];
            const tchar c = test [i + start];

            if (wc == c
                 || (ignoreCase && CharacterFunctions::toLowerCase (wc) == CharacterFunctions::toLowerCase (c))
                 || (wc == T('?') && c != 0))
            {
                if (wc == 0)
                    return start;

                ++i;
            }
            else
            {
                if (wc == T('*') && (wildcard [i + 1] == 0
                                      || indexOfMatch (wildcard + i + 1,
                                                       test + start + i,
                                                       ignoreCase) >= 0))
                {
                    return start;
                }

                break;
            }
        }

        ++start;
    }

    return -1;
}

bool String::matchesWildcard (const tchar* wildcard, const bool ignoreCase) const throw()
{
    int i = 0;

    for (;;)
    {
        const tchar wc = wildcard [i];
        const tchar c = text->text [i];

        if (wc == c
             || (ignoreCase && CharacterFunctions::toLowerCase (wc) == CharacterFunctions::toLowerCase (c))
             || (wc == T('?') && c != 0))
        {
            if (wc == 0)
                return true;

            ++i;
        }
        else
        {
            return wc == T('*') && (wildcard [i + 1] == 0
                                     || indexOfMatch (wildcard + i + 1,
                                                      text->text + i,
                                                      ignoreCase) >= 0);
        }
    }
}

//==============================================================================
void String::printf (const tchar* const pf, ...) throw()
{
    va_list list;
    va_start (list, pf);

    vprintf (pf, list);
}

const String String::formatted (const tchar* const pf, ...) throw()
{
    va_list list;
    va_start (list, pf);

    String result;
    result.vprintf (pf, list);
    return result;
}

//==============================================================================
void String::vprintf (const tchar* const pf, va_list& args) throw()
{
    int bufferSize = 256;
    String result (bufferSize, 0);

    do
    {
#if JUCE_LINUX && JUCE_64BIT
        va_list tempArgs;
        va_copy (tempArgs, args);
        const int num = CharacterFunctions::vprintf (result.text->text, bufferSize - 1, pf, tempArgs);
        va_end (tempArgs);
#else
        const int num = CharacterFunctions::vprintf (result.text->text, bufferSize - 1, pf, args);
#endif

        if (num > 0)
        {
            *this = result;
            break;
        }
        else if (num == 0)
        {
            *this = String::empty;
            break;
        }

        bufferSize += 256;
        result.preallocateStorage (bufferSize);
    }
    while (bufferSize < 65536);  // this is a sanity check to avoid situations where vprintf repeatedly
                                 // returns -1 because of an error rather than because it needs more space.
}

//==============================================================================
const String String::repeatedString (const tchar* const stringToRepeat,
                                     int numberOfTimesToRepeat) throw()
{
    const int len = CharacterFunctions::length (stringToRepeat);
    String result ((int) (len * numberOfTimesToRepeat + 1), (int) 0);

    tchar* n = result.text->text;
    n[0] = 0;

    while (--numberOfTimesToRepeat >= 0)
    {
        CharacterFunctions::append (n, stringToRepeat);
        n += len;
    }

    return result;
}

//==============================================================================
const String String::replaceSection (int index,
                                     int numCharsToReplace,
                                     const tchar* const stringToInsert) const throw()
{
    if (index < 0)
    {
        // a negative index to replace from?
        jassertfalse
        index = 0;
    }

    if (numCharsToReplace < 0)
    {
        // replacing a negative number of characters?
        numCharsToReplace = 0;
        jassertfalse;
    }

    const int len = length();

    if (index + numCharsToReplace > len)
    {
        if (index > len)
        {
            // replacing beyond the end of the string?
            index = len;
            jassertfalse
        }

        numCharsToReplace = len - index;
    }

    const int newStringLen = (stringToInsert != 0) ? CharacterFunctions::length (stringToInsert) : 0;
    const int newTotalLen = len + newStringLen - numCharsToReplace;

    if (newTotalLen <= 0)
        return String::empty;

    String result (newTotalLen, (int) 0);

    memcpy (result.text->text,
            text->text,
            index * sizeof (tchar));

    if (newStringLen > 0)
        memcpy (result.text->text + index,
                stringToInsert,
                newStringLen * sizeof (tchar));

    const int endStringLen = newTotalLen - (index + newStringLen);

    if (endStringLen > 0)
        memcpy (result.text->text + (index + newStringLen),
                text->text + (index + numCharsToReplace),
                endStringLen * sizeof (tchar));

    result.text->text [newTotalLen] = 0;

    return result;
}

const String String::replace (const tchar* const stringToReplace,
                              const tchar* const stringToInsert,
                              const bool ignoreCase) const throw()
{
    const int stringToReplaceLen = CharacterFunctions::length (stringToReplace);
    const int stringToInsertLen = CharacterFunctions::length (stringToInsert);

    int i = 0;
    String result (*this);

    while ((i = (ignoreCase ? result.indexOfIgnoreCase (i, stringToReplace)
                            : result.indexOf (i, stringToReplace))) >= 0)
    {
        result = result.replaceSection (i, stringToReplaceLen, stringToInsert);
        i += stringToInsertLen;
    }

    return result;
}

const String String::replaceCharacter (const tchar charToReplace,
                                       const tchar charToInsert) const throw()
{
    const int index = indexOfChar (charToReplace);

    if (index < 0)
        return *this;

    String result (*this);
    result.dupeInternalIfMultiplyReferenced();

    tchar* t = result.text->text + index;

    while (*t != 0)
    {
        if (*t == charToReplace)
            *t = charToInsert;

        ++t;
    }

    return result;
}

const String String::replaceCharacters (const String& charactersToReplace,
                                        const tchar* const charactersToInsertInstead) const throw()
{
    String result (*this);
    result.dupeInternalIfMultiplyReferenced();

    tchar* t = result.text->text;
    const int len2 = CharacterFunctions::length (charactersToInsertInstead);

    // the two strings passed in are supposed to be the same length!
    jassert (len2 == charactersToReplace.length());

    while (*t != 0)
    {
        const int index = charactersToReplace.indexOfChar (*t);

        if (((unsigned int) index) < (unsigned int) len2)
            *t = charactersToInsertInstead [index];

        ++t;
    }

    return result;
}

//==============================================================================
bool String::startsWith (const tchar* const other) const throw()
{
    return other != 0
            && CharacterFunctions::compare (text->text, other, CharacterFunctions::length (other)) == 0;
}

bool String::startsWithIgnoreCase (const tchar* const other) const throw()
{
    return other != 0
            && CharacterFunctions::compareIgnoreCase (text->text, other, CharacterFunctions::length (other)) == 0;
}

bool String::startsWithChar (const tchar character) const throw()
{
    jassert (character != 0); // strings can't contain a null character!

    return text->text[0] == character;
}

bool String::endsWithChar (const tchar character) const throw()
{
    jassert (character != 0); // strings can't contain a null character!

    return text->text[0] != 0
            && text->text [length() - 1] == character;
}

bool String::endsWith (const tchar* const other) const throw()
{
    if (other == 0)
        return false;

    const int thisLen = length();
    const int otherLen = CharacterFunctions::length (other);

    return thisLen >= otherLen
            && CharacterFunctions::compare (text->text + thisLen - otherLen, other) == 0;
}

bool String::endsWithIgnoreCase (const tchar* const other) const throw()
{
    if (other == 0)
        return false;

    const int thisLen = length();
    const int otherLen = CharacterFunctions::length (other);

    return thisLen >= otherLen
            && CharacterFunctions::compareIgnoreCase (text->text + thisLen - otherLen, other) == 0;
}

//==============================================================================
const String String::toUpperCase() const throw()
{
    String result (*this);
    result.dupeInternalIfMultiplyReferenced();
    CharacterFunctions::toUpperCase (result.text->text);
    return result;
}

const String String::toLowerCase() const throw()
{
    String result (*this);
    result.dupeInternalIfMultiplyReferenced();
    CharacterFunctions::toLowerCase (result.text->text);
    return result;
}

//==============================================================================
juce_wchar& String::operator[] (const int index) throw()
{
    jassert (((unsigned int) index) <= (unsigned int) length());

    dupeInternalIfMultiplyReferenced();

    return text->text [index];
}

juce_wchar String::getLastCharacter() const throw()
{
    return (isEmpty()) ? ((juce_wchar) 0)
                       : text->text [CharacterFunctions::length (text->text) - 1];
}

const String String::substring (int start, int end) const throw()
{
    if (start < 0)
        start = 0;
    else if (end <= start)
        return empty;

    int len = 0;
    const tchar* const t = text->text;

    while (len <= end && t [len] != 0)
        ++len;

    if (end >= len)
    {
        if (start == 0)
            return *this;

        end = len;
    }

    return String (text->text + start,
                   end - start);
}

const String String::substring (const int start) const throw()
{
    if (start <= 0)
        return *this;

    const int len = CharacterFunctions::length (text->text);

    if (start >= len)
        return empty;
    else
        return String (text->text + start,
                       len - start);
}

const String String::dropLastCharacters (const int numberToDrop) const throw()
{
    return String (text->text,
                   jmax (0, CharacterFunctions::length (text->text) - numberToDrop));
}

const String String::getLastCharacters (const int numCharacters) const throw()
{
    return String (text->text + jmax (0, CharacterFunctions::length (text->text) - jmax (0, numCharacters)));
}

const String String::fromFirstOccurrenceOf (const tchar* const sub,
                                            const bool includeSubString,
                                            const bool ignoreCase) const throw()
{
    const int i = ignoreCase ? indexOfIgnoreCase (sub)
                             : indexOf (sub);

    if (i < 0)
        return empty;
    else
        return substring (includeSubString ? i : i + CharacterFunctions::length (sub));
}


const String String::fromLastOccurrenceOf (const tchar* const sub,
                                           const bool includeSubString,
                                           const bool ignoreCase) const throw()
{
    const int i = ignoreCase ? lastIndexOfIgnoreCase (sub)
                             : lastIndexOf (sub);

    if (i < 0)
        return *this;
    else
        return substring (includeSubString ? i : i + CharacterFunctions::length (sub));
}

const String String::upToFirstOccurrenceOf (const tchar* const sub,
                                            const bool includeSubString,
                                            const bool ignoreCase) const throw()
{
    const int i = ignoreCase ? indexOfIgnoreCase (sub)
                             : indexOf (sub);

    if (i < 0)
        return *this;
    else
        return substring (0, includeSubString ? i + CharacterFunctions::length (sub) : i);
}

const String String::upToLastOccurrenceOf (const tchar* const sub,
                                           const bool includeSubString,
                                           const bool ignoreCase) const throw()
{
    const int i = ignoreCase ? lastIndexOfIgnoreCase (sub)
                             : lastIndexOf (sub);
    if (i < 0)
        return *this;

    return substring (0, includeSubString ? i + CharacterFunctions::length (sub) : i);
}

bool String::isQuotedString() const throw()
{
    const String trimmed (trimStart());

    return trimmed[0] == T('"')
        || trimmed[0] == T('\'');
}

const String String::unquoted() const throw()
{
    String s (*this);

    if (s[0] == T('"') || s[0] == T('\''))
        s = s.substring (1);

    const int lastCharIndex = s.length() - 1;

    if (lastCharIndex >= 0
         && (s [lastCharIndex] == T('"') || s[lastCharIndex] == T('\'')))
        s [lastCharIndex] = 0;

    return s;
}

const String String::quoted (const tchar quoteCharacter) const throw()
{
    if (isEmpty())
        return charToString (quoteCharacter) + quoteCharacter;

    String t (*this);

    if (! t.startsWithChar (quoteCharacter))
        t = charToString (quoteCharacter) + t;

    if (! t.endsWithChar (quoteCharacter))
        t += quoteCharacter;

    return t;
}

//==============================================================================
const String String::trim() const throw()
{
    if (isEmpty())
        return empty;

    int start = 0;

    while (CharacterFunctions::isWhitespace (text->text [start]))
        ++start;

    const int len = CharacterFunctions::length (text->text);
    int end = len - 1;

    while ((end >= start) && CharacterFunctions::isWhitespace (text->text [end]))
        --end;

    ++end;

    if (end <= start)
        return empty;
    else if (start > 0 || end < len)
        return String (text->text + start, end - start);
    else
        return *this;
}

const String String::trimStart() const throw()
{
    if (isEmpty())
        return empty;

    const tchar* t = text->text;

    while (CharacterFunctions::isWhitespace (*t))
        ++t;

    if (t == text->text)
        return *this;
    else
        return String (t);
}

const String String::trimEnd() const throw()
{
    if (isEmpty())
        return empty;

    const tchar* endT = text->text + (CharacterFunctions::length (text->text) - 1);

    while ((endT >= text->text) && CharacterFunctions::isWhitespace (*endT))
        --endT;

    return String (text->text, (int) (++endT - text->text));
}

const String String::trimCharactersAtStart (const tchar* charactersToTrim) const throw()
{
    jassert (charactersToTrim != 0);

    if (isEmpty())
        return empty;

    const tchar* t = text->text;

    while (CharacterFunctions::indexOfCharFast (charactersToTrim, *t) >= 0)
        ++t;

    if (t == text->text)
        return *this;
    else
        return String (t);
}

const String String::trimCharactersAtEnd (const tchar* charactersToTrim) const throw()
{
    jassert (charactersToTrim != 0);

    if (isEmpty())
        return empty;

    const tchar* endT = text->text + (CharacterFunctions::length (text->text) - 1);

    while ((endT >= text->text) && CharacterFunctions::indexOfCharFast (charactersToTrim, *endT) >= 0)
        --endT;

    return String (text->text, (int) (++endT - text->text));
}

//==============================================================================
const String String::retainCharacters (const tchar* const charactersToRetain) const throw()
{
    jassert (charactersToRetain != 0);

    if (isEmpty())
        return empty;

    String result (text->allocatedNumChars, (int) 0);
    tchar* dst = result.text->text;
    const tchar* src = text->text;

    while (*src != 0)
    {
        if (CharacterFunctions::indexOfCharFast (charactersToRetain, *src) >= 0)
            *dst++ = *src;

        ++src;
    }

    *dst = 0;
    return result;
}

const String String::removeCharacters (const tchar* const charactersToRemove) const throw()
{
    jassert (charactersToRemove != 0);

    if (isEmpty())
        return empty;

    String result (text->allocatedNumChars, (int) 0);
    tchar* dst = result.text->text;
    const tchar* src = text->text;

    while (*src != 0)
    {
        if (CharacterFunctions::indexOfCharFast (charactersToRemove, *src) < 0)
            *dst++ = *src;

        ++src;
    }

    *dst = 0;
    return result;
}

const String String::initialSectionContainingOnly (const tchar* const permittedCharacters) const throw()
{
    return substring (0, CharacterFunctions::getIntialSectionContainingOnly (text->text, permittedCharacters));
}

const String String::initialSectionNotContaining (const tchar* const charactersToStopAt) const throw()
{
    jassert (charactersToStopAt != 0);

    const tchar* const t = text->text;
    int i = 0;

    while (t[i] != 0)
    {
        if (CharacterFunctions::indexOfCharFast (charactersToStopAt, t[i]) >= 0)
            return String (text->text, i);

        ++i;
    }

    return empty;
}

bool String::containsOnly (const tchar* const chars) const throw()
{
    jassert (chars != 0);

    const tchar* t = text->text;

    while (*t != 0)
        if (CharacterFunctions::indexOfCharFast (chars, *t++) < 0)
            return false;

    return true;
}

bool String::containsAnyOf (const tchar* const chars) const throw()
{
    jassert (chars != 0);

    const tchar* t = text->text;

    while (*t != 0)
        if (CharacterFunctions::indexOfCharFast (chars, *t++) >= 0)
            return true;

    return false;
}

bool String::containsNonWhitespaceChars() const throw()
{
    const tchar* t = text->text;

    while (*t != 0)
        if (! CharacterFunctions::isWhitespace (*t++))
            return true;

    return false;
}


//==============================================================================
int String::getIntValue() const throw()
{
    return CharacterFunctions::getIntValue (text->text);
}

int String::getTrailingIntValue() const throw()
{
    int n = 0;
    int mult = 1;
    const tchar* t = text->text + length();

    while (--t >= text->text)
    {
        const tchar c = *t;

        if (! CharacterFunctions::isDigit (c))
        {
            if (c == T('-'))
                n = -n;

            break;
        }

        n += mult * (c - T('0'));
        mult *= 10;
    }

    return n;
}

int64 String::getLargeIntValue() const throw()
{
    return CharacterFunctions::getInt64Value (text->text);
}

float String::getFloatValue() const throw()
{
    return (float) CharacterFunctions::getDoubleValue (text->text);
}

double String::getDoubleValue() const throw()
{
    return CharacterFunctions::getDoubleValue (text->text);
}

static const tchar* const hexDigits = T("0123456789abcdef");

const String String::toHexString (const int number) throw()
{
    tchar buffer[32];
    tchar* const end = buffer + 32;
    tchar* t = end;
    *--t = 0;
    unsigned int v = (unsigned int) number;

    do
    {
        *--t = hexDigits [v & 15];
        v >>= 4;

    } while (v != 0);

    return String (t, (int) (((char*) end) - (char*) t) - 1);
}

const String String::toHexString (const int64 number) throw()
{
    tchar buffer[32];
    tchar* const end = buffer + 32;
    tchar* t = end;
    *--t = 0;
    uint64 v = (uint64) number;

    do
    {
        *--t = hexDigits [(int) (v & 15)];
        v >>= 4;

    } while (v != 0);

    return String (t, (int) (((char*) end) - (char*) t));
}

const String String::toHexString (const short number) throw()
{
    return toHexString ((int) (unsigned short) number);
}

const String String::toHexString (const unsigned char* data,
                                  const int size,
                                  const int groupSize) throw()
{
    if (size <= 0)
        return empty;

    int numChars = (size * 2) + 2;
    if (groupSize > 0)
        numChars += size / groupSize;

    String s (numChars, (int) 0);

    tchar* d = s.text->text;

    for (int i = 0; i < size; ++i)
    {
        *d++ = hexDigits [(*data) >> 4];
        *d++ = hexDigits [(*data) & 0xf];
        ++data;

        if (groupSize > 0 && (i % groupSize) == (groupSize - 1) && i < (size - 1))
            *d++ = T(' ');
    }

    *d = 0;
    return s;
}

int String::getHexValue32() const throw()
{
    int result = 0;
    const tchar* c = text->text;

    for (;;)
    {
        const int hexValue = CharacterFunctions::getHexDigitValue (*c);

        if (hexValue >= 0)
            result = (result << 4) | hexValue;
        else if (*c == 0)
            break;

        ++c;
    }

    return result;
}

int64 String::getHexValue64() const throw()
{
    int64 result = 0;
    const tchar* c = text->text;

    for (;;)
    {
        const int hexValue = CharacterFunctions::getHexDigitValue (*c);

        if (hexValue >= 0)
            result = (result << 4) | hexValue;
        else if (*c == 0)
            break;

        ++c;
    }

    return result;
}

//==============================================================================
const String String::createStringFromData (const void* const data_,
                                           const int size) throw()
{
    const char* const data = (const char*) data_;

    if (size <= 0 || data == 0)
    {
        return empty;
    }
    else if (size < 2)
    {
        return charToString (data[0]);
    }
    else if ((data[0] == (char)-2 && data[1] == (char)-1)
             || (data[0] == (char)-1 && data[1] == (char)-2))
    {
        // assume it's 16-bit unicode
        const bool bigEndian = (data[0] == (char)-2);
        const int numChars = size / 2 - 1;

        String result;
        result.preallocateStorage (numChars + 2);

        const uint16* const src = (const uint16*) (data + 2);
        tchar* const dst = const_cast <tchar*> ((const tchar*) result);

        if (bigEndian)
        {
            for (int i = 0; i < numChars; ++i)
                dst[i] = (tchar) ByteOrder::swapIfLittleEndian (src[i]);
        }
        else
        {
            for (int i = 0; i < numChars; ++i)
                dst[i] = (tchar) ByteOrder::swapIfBigEndian (src[i]);
        }

        dst [numChars] = 0;
        return result;
    }
    else
    {
        return String::fromUTF8 (data, size);
    }
}

//==============================================================================
const char* String::toUTF8() const
{
    if (isEmpty())
    {
        return (const char*) emptyCharString;
    }
    else
    {
        String* const mutableThis = const_cast <String*> (this);

        mutableThis->dupeInternalIfMultiplyReferenced();

        const int currentLen = CharacterFunctions::length (text->text) + 1;
        const int utf8BytesNeeded = getNumBytesAsUTF8() + 1;

        mutableThis->text = (InternalRefCountedStringHolder*)
                                juce_realloc (text, sizeof (InternalRefCountedStringHolder)
                                                  + (currentLen * sizeof (juce_wchar) + utf8BytesNeeded));

        char* const otherCopy = (char*) (text->text + currentLen);
        copyToUTF8 (otherCopy, std::numeric_limits<int>::max());

        return otherCopy;
    }
}

int String::copyToUTF8 (char* const buffer, const int maxBufferSizeBytes) const throw()
{
    jassert (maxBufferSizeBytes >= 0); // keep this value positive, or no characters will be copied!

    int num = 0, index = 0;

    for (;;)
    {
        const uint32 c = (uint32) text->text [index++];

        if (c >= 0x80)
        {
            int numExtraBytes = 1;

            if (c >= 0x800)
            {
                ++numExtraBytes;

                if (c >= 0x10000)
                {
                    ++numExtraBytes;

                    if (c >= 0x200000)
                    {
                        ++numExtraBytes;

                        if (c >= 0x4000000)
                            ++numExtraBytes;
                    }
                }
            }

            if (buffer != 0)
            {
                if (num + numExtraBytes >= maxBufferSizeBytes)
                {
                    buffer [num++] = 0;
                    break;
                }
                else
                {
                    buffer [num++] = (uint8) ((0xff << (7 - numExtraBytes)) | (c >> (numExtraBytes * 6)));

                    while (--numExtraBytes >= 0)
                        buffer [num++] = (uint8) (0x80 | (0x3f & (c >> (numExtraBytes * 6))));
                }
            }
            else
            {
                num += numExtraBytes + 1;
            }
        }
        else
        {
            if (buffer != 0)
            {
                if (num + 1 >= maxBufferSizeBytes)
                {
                    buffer [num++] = 0;
                    break;
                }

                buffer [num] = (uint8) c;
            }

            ++num;
        }

        if (c == 0)
            break;
    }

    return num;
}

int String::getNumBytesAsUTF8() const throw()
{
    int num = 0;
    const juce_wchar* t = text->text;

    for (;;)
    {
        const uint32 c = (uint32) *t;

        if (c >= 0x80)
        {
            ++num;
            if (c >= 0x800)
            {
                ++num;
                if (c >= 0x10000)
                {
                    ++num;
                    if (c >= 0x200000)
                    {
                        ++num;
                        if (c >= 0x4000000)
                            ++num;
                    }
                }
            }
        }
        else if (c == 0)
            break;

        ++num;
        ++t;
    }

    return num;
}

const String String::fromUTF8 (const char* const buffer, int bufferSizeBytes)
{
    if (buffer == 0)
        return empty;

    if (bufferSizeBytes < 0)
        bufferSizeBytes = std::numeric_limits<int>::max();

    size_t numBytes;
    for (numBytes = 0; numBytes < (size_t) bufferSizeBytes; ++numBytes)
        if (buffer [numBytes] == 0)
            break;

    String result ((int) numBytes + 1, 0);
    juce_wchar* dest = result.text->text;

    size_t i = 0;
    while (i < numBytes)
    {
        const char c = buffer [i++];

        if (c < 0)
        {
            unsigned int mask = 0x7f;
            int bit = 0x40;
            int numExtraValues = 0;

            while (bit != 0 && (c & bit) != 0)
            {
                bit >>= 1;
                mask >>= 1;
                ++numExtraValues;
            }

            int n = (mask & (unsigned char) c);

            while (--numExtraValues >= 0 && i < (size_t) bufferSizeBytes)
            {
                const char nextByte = buffer[i];

                if ((nextByte & 0xc0) != 0x80)
                    break;

                n <<= 6;
                n |= (nextByte & 0x3f);
                ++i;
            }

            *dest++ = (juce_wchar) n;
        }
        else
        {
            *dest++ = (juce_wchar) c;
        }
    }

    *dest = 0;
    return result;
}

//==============================================================================
const char* String::toCString() const
{
    if (isEmpty())
    {
        return (const char*) emptyCharString;
    }
    else
    {
        String* const mutableThis = const_cast <String*> (this);

        mutableThis->dupeInternalIfMultiplyReferenced();
        int len = length() + 1;
        mutableThis->text = (InternalRefCountedStringHolder*)
                                juce_realloc (text, sizeof (InternalRefCountedStringHolder)
                                                      + (len * sizeof (juce_wchar) + len));
        char* otherCopy = (char*) (text->text + len);
        --len;

        CharacterFunctions::copy (otherCopy, text->text, len);
        otherCopy [len] = 0;
        return otherCopy;
    }
}

#ifdef _MSC_VER
  #pragma warning (disable: 4514 4996)
  #pragma warning (push)
#endif

int String::getNumBytesAsCString() const throw()
{
    return (int) wcstombs (0, text->text, 0);
}

int String::copyToCString (char* destBuffer, const int maxBufferSizeBytes) const throw()
{
    const int numBytes = (int) wcstombs (destBuffer, text->text, maxBufferSizeBytes);

    if (destBuffer != 0 && numBytes >= 0)
        destBuffer [numBytes] = 0;

    return numBytes;
}

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

//==============================================================================
void String::copyToUnicode (juce_wchar* const destBuffer, const int maxCharsToCopy) const throw()
{
    const int len = jmin (maxCharsToCopy, length());
    memcpy (destBuffer, text->text, len * sizeof (juce_wchar));
    destBuffer [len] = 0;
}


//==============================================================================
String::Concatenator::Concatenator (String& stringToAppendTo)
    : result (stringToAppendTo),
      nextIndex (stringToAppendTo.length())
{
}

String::Concatenator::~Concatenator()
{
}

void String::Concatenator::append (const String& s)
{
    const int len = s.length();

    if (len > 0)
    {
        result.preallocateStorage (nextIndex + len);
        s.copyToUnicode (((juce_wchar*) result) + nextIndex, len);
        nextIndex += len;
    }
}


END_JUCE_NAMESPACE
