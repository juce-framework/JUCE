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
class StringHolder
{
public:
    //==============================================================================
    static juce_wchar* create (const size_t numChars)
    {
        StringHolder* const s = reinterpret_cast <StringHolder*> (juce_malloc (sizeof (StringHolder) + numChars * sizeof (juce_wchar)));
        s->refCount = 0;
        s->allocatedNumChars = numChars;
        return &(s->text[0]);
    }

    static inline juce_wchar* getEmpty() throw()
    {
        return &(empty.text[0]);
    }

    //==============================================================================
    static void retain (juce_wchar* const text) throw()
    {
        Atomic::increment (bufferFromText (text)->refCount);
    }

    static inline void release (StringHolder* const b) throw()
    {
        if (Atomic::decrementAndReturn (b->refCount) == -1 && b != &empty)
            juce_free (b);
    }

    static void release (juce_wchar* const text) throw()
    {
        release (bufferFromText (text));
    }

    //==============================================================================
    static juce_wchar* makeUnique (juce_wchar* const text)
    {
        StringHolder* const b = bufferFromText (text);

        if (b->refCount <= 0)
            return text;

        juce_wchar* const newText = create (b->allocatedNumChars);
        memcpy (newText, text, (b->allocatedNumChars + 1) * sizeof (juce_wchar));
        release (b);

        return newText;
    }

    static juce_wchar* makeUniqueWithSize (juce_wchar* const text, size_t numChars)
    {
        StringHolder* const b = bufferFromText (text);

        if (b->refCount <= 0 && b->allocatedNumChars >= numChars)
            return text;

        juce_wchar* const newText = create (jmax (b->allocatedNumChars, numChars));
        memcpy (newText, text, (b->allocatedNumChars + 1) * sizeof (juce_wchar));
        release (b);

        return newText;
    }

    static size_t getAllocatedNumChars (juce_wchar* const text) throw()
    {
        return bufferFromText (text)->allocatedNumChars;
    }

    //==============================================================================
    int refCount;
    size_t allocatedNumChars;
    juce_wchar text[1];

    static StringHolder empty;

private:
    static inline StringHolder* bufferFromText (juce_wchar* const text) throw()
    {
        return reinterpret_cast <StringHolder*> (reinterpret_cast <char*> (text) - offsetof (StringHolder, StringHolder::text));
    }
};

StringHolder StringHolder::empty = { 0x3fffffff, 0, { 0 } };
const String String::empty;

//==============================================================================
void String::createInternal (const juce_wchar* const t, const size_t numChars)
{
    jassert (t[numChars] == 0); // must have a null terminator

    text = StringHolder::create (numChars);
    memcpy (text, t, (numChars + 1) * sizeof (juce_wchar));
}

void String::appendInternal (const juce_wchar* const newText, const int numExtraChars)
{
    if (numExtraChars > 0)
    {
        const int oldLen = length();
        const int newTotalLen = oldLen + numExtraChars;

        text = StringHolder::makeUniqueWithSize (text, newTotalLen);
        memcpy (text + oldLen, newText, numExtraChars * sizeof (juce_wchar));
        text [newTotalLen] = 0;
    }
}

void String::dupeInternalIfMultiplyReferenced()
{
    text = StringHolder::makeUnique (text);
}

void String::preallocateStorage (const size_t numChars)
{
    text = StringHolder::makeUniqueWithSize (text, numChars);
}

//==============================================================================
String::String() throw()
    : text (StringHolder::getEmpty())
{
}

String::~String() throw()
{
    StringHolder::release (text);
}

String::String (const String& other) throw()
    : text (other.text)
{
    StringHolder::retain (text);
}

String& String::operator= (const String& other) throw()
{
    juce_wchar* const newText = other.text;
    StringHolder::retain (newText);
    StringHolder::release (static_cast <juce_wchar*> (Atomic::swapPointers ((void* volatile*) &text, newText)));
    return *this;
}

String::String (const size_t numChars, const int /*dummyVariable*/)
    : text (StringHolder::create (numChars))
{
}

String::String (const char* const t) throw()
{
    if (t != 0 && *t != 0)
    {
        const int len = CharacterFunctions::length (t);
        text = StringHolder::create (len);
        CharacterFunctions::copy (text, t, len + 1);
    }
    else
    {
        text = StringHolder::getEmpty();
    }
}

String::String (const juce_wchar* const t) throw()
{
    if (t != 0 && *t != 0)
    {
        const int len = CharacterFunctions::length (t);
        text = StringHolder::create (len);
        memcpy (text, t, (len + 1) * sizeof (juce_wchar));
    }
    else
    {
        text = StringHolder::getEmpty();
    }
}

String::String (const char* const t, const size_t maxChars) throw()
{
    int i;
    for (i = 0; (size_t) i < maxChars; ++i)
        if (t[i] == 0)
            break;

    if (i > 0)
    {
        text = StringHolder::create (i);
        CharacterFunctions::copy (text, t, i);
        text[i] = 0;
    }
    else
    {
        text = StringHolder::getEmpty();
    }
}

String::String (const juce_wchar* const t, const size_t maxChars) throw()
{
    int i;
    for (i = 0; (size_t) i < maxChars; ++i)
        if (t[i] == 0)
            break;

    if (i > 0)
    {
        text = StringHolder::create (i);
        memcpy (text, t, i * sizeof (juce_wchar));
        text[i] = 0;
    }
    else
    {
        text = StringHolder::getEmpty();
    }
}

const String String::charToString (const juce_wchar character) throw()
{
    juce_wchar temp[] = { character, 0 };
    return String (temp);
}

//==============================================================================
namespace NumberToStringConverters
{
    // pass in a pointer to the END of a buffer..
    static juce_wchar* int64ToString (juce_wchar* t, const int64 n) throw()
    {
        *--t = 0;
        int64 v = (n >= 0) ? n : -n;

        do
        {
            *--t = (juce_wchar) (T('0') + (int) (v % 10));
            v /= 10;

        } while (v > 0);

        if (n < 0)
            *--t = T('-');

        return t;
    }

    static juce_wchar* uint64ToString (juce_wchar* t, int64 v) throw()
    {
        *--t = 0;

        do
        {
            *--t = (juce_wchar) (T('0') + (int) (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    static juce_wchar* intToString (juce_wchar* t, const int n) throw()
    {
        if (n == (int) 0x80000000) // (would cause an overflow)
            return int64ToString (t, n);

        *--t = 0;
        int v = abs (n);

        do
        {
            *--t = (juce_wchar) (T('0') + (v % 10));
            v /= 10;

        } while (v > 0);

        if (n < 0)
            *--t = T('-');

        return t;
    }

    static juce_wchar* uintToString (juce_wchar* t, unsigned int v) throw()
    {
        *--t = 0;

        do
        {
            *--t = (juce_wchar) (T('0') + (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    static juce_wchar getDecimalPoint()
    {
        static juce_wchar dp = std::use_facet <std::numpunct <wchar_t> > (std::locale()).decimal_point();
        return dp;
    }

    static juce_wchar* doubleToString (juce_wchar* buffer, int numChars, double n, int numDecPlaces, size_t& len) throw()
    {
        if (numDecPlaces > 0 && n > -1.0e20 && n < 1.0e20)
        {
            juce_wchar* const end = buffer + numChars;
            juce_wchar* t = end;
            int64 v = (int64) (pow (10.0, numDecPlaces) * fabs (n) + 0.5);
            *--t = (juce_wchar) 0;

            while (numDecPlaces >= 0 || v > 0)
            {
                if (numDecPlaces == 0)
                    *--t = getDecimalPoint();

                *--t = (juce_wchar) (T('0') + (v % 10));

                v /= 10;
                --numDecPlaces;
            }

            if (n < 0)
                *--t = T('-');

            len = end - t - 1;
            return t;
        }
        else
        {
#if JUCE_WIN32
  #if _MSC_VER <= 1200
            len = _snwprintf (buffer, numChars, L"%.9g", n);
  #else
            len = _snwprintf_s (buffer, numChars, _TRUNCATE, L"%.9g", n);
  #endif
#else
            len = swprintf (buffer, numChars, L"%.9g", n);
#endif
            return buffer;
        }
    }
}

//==============================================================================
String::String (const int number) throw()
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::intToString (end, number);
    createInternal (start, end - start - 1);
}

String::String (const unsigned int number) throw()
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::uintToString (end, number);
    createInternal (start, end - start - 1);
}

String::String (const short number) throw()
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::intToString (end, (int) number);
    createInternal (start, end - start - 1);
}

String::String (const unsigned short number) throw()
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::uintToString (end, (unsigned int) number);
    createInternal (start, end - start - 1);
}

String::String (const int64 number) throw()
{
    juce_wchar buffer [32];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::int64ToString (end, number);
    createInternal (start, end - start - 1);
}

String::String (const uint64 number) throw()
{
    juce_wchar buffer [32];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::uint64ToString (end, number);
    createInternal (start, end - start - 1);
}

String::String (const float number, const int numberOfDecimalPlaces) throw()
{
    juce_wchar buffer [48];
    size_t len;
    juce_wchar* start = NumberToStringConverters::doubleToString (buffer, numElementsInArray (buffer), (double) number, numberOfDecimalPlaces, len);
    createInternal (start, len);
}

String::String (const double number, const int numberOfDecimalPlaces) throw()
{
    juce_wchar buffer [48];
    size_t len;
    juce_wchar* start = NumberToStringConverters::doubleToString (buffer, numElementsInArray (buffer), number, numberOfDecimalPlaces, len);
    createInternal (start, len);
}

//==============================================================================
int String::length() const throw()
{
    return CharacterFunctions::length (text);
}

int String::hashCode() const throw()
{
    const juce_wchar* t = text;
    int result = 0;

    while (*t != (juce_wchar) 0)
        result = 31 * result + *t++;

    return result;
}

int64 String::hashCode64() const throw()
{
    const juce_wchar* t = text;
    int64 result = 0;

    while (*t != (juce_wchar) 0)
        result = 101 * result + *t++;

    return result;
}

//==============================================================================
bool JUCE_CALLTYPE operator== (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) == 0;
}

bool JUCE_CALLTYPE operator== (const String& string1, const char* string2) throw()
{
    return string1.compare (string2) == 0;
}

bool JUCE_CALLTYPE operator== (const String& string1, const juce_wchar* string2) throw()
{
    return string1.compare (string2) == 0;
}

bool JUCE_CALLTYPE operator!= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) != 0;
}

bool JUCE_CALLTYPE operator!= (const String& string1, const char* string2) throw()
{
    return string1.compare (string2) != 0;
}

bool JUCE_CALLTYPE operator!= (const String& string1, const juce_wchar* string2) throw()
{
    return string1.compare (string2) != 0;
}

bool JUCE_CALLTYPE operator>  (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) > 0;
}

bool JUCE_CALLTYPE operator<  (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) < 0;
}

bool JUCE_CALLTYPE operator>= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) >= 0;
}

bool JUCE_CALLTYPE operator<= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) <= 0;
}

bool String::equalsIgnoreCase (const juce_wchar* t) const throw()
{
    return t != 0 ? CharacterFunctions::compareIgnoreCase (text, t) == 0
                  : isEmpty();
}

bool String::equalsIgnoreCase (const String& other) const throw()
{
    return text == other.text
            || CharacterFunctions::compareIgnoreCase (text, other.text) == 0;
}

int String::compare (const String& other) const throw()
{
    return (text == other.text) ? 0 : CharacterFunctions::compare (text, other.text);
}

int String::compare (const char* other) const throw()
{
    return other == 0 ? isEmpty() : CharacterFunctions::compare (text, other);
}

int String::compare (const juce_wchar* other) const throw()
{
    return other == 0 ? isEmpty() : CharacterFunctions::compare (text, other);
}

int String::compareIgnoreCase (const String& other) const throw()
{
    return (text == other.text) ? 0 : CharacterFunctions::compareIgnoreCase (text, other.text);
}

int String::compareLexicographically (const String& other) const throw()
{
    const juce_wchar* s1 = text;
    while (*s1 != 0 && ! CharacterFunctions::isLetterOrDigit (*s1))
        ++s1;

    const juce_wchar* s2 = other.text;
    while (*s2 != 0 && ! CharacterFunctions::isLetterOrDigit (*s2))
        ++s2;

    return CharacterFunctions::compareIgnoreCase (s1, s2);
}

//==============================================================================
String& String::operator+= (const juce_wchar* const t)
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
        appendInternal (other.text, other.length());

    return *this;
}

String& String::operator+= (const char ch)
{
    const juce_wchar asString[] = { (juce_wchar) ch, 0 };
    return operator+= ((const juce_wchar*) asString);
}

String& String::operator+= (const juce_wchar ch)
{
    const juce_wchar asString[] = { (juce_wchar) ch, 0 };
    return operator+= ((const juce_wchar*) asString);
}

String& String::operator+= (const int number)
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::intToString (end, number);
    appendInternal (start, (int) (end - start));
    return *this;
}

String& String::operator+= (const unsigned int number)
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::uintToString (end, number);
    appendInternal (start, (int) (end - start));
    return *this;
}

void String::append (const juce_wchar* const other, const int howMany)
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
const String JUCE_CALLTYPE operator+ (const char* const string1, const String& string2)
{
    String s (string1);
    return s += string2;
}

const String JUCE_CALLTYPE operator+ (const juce_wchar* const string1, const String& string2)
{
    String s (string1);
    return s += string2;
}

const String JUCE_CALLTYPE operator+ (const char string1, const String& string2)
{
    return String::charToString (string1) + string2;
}

const String JUCE_CALLTYPE operator+ (const juce_wchar string1, const String& string2)
{
    return String::charToString (string1) + string2;
}

const String JUCE_CALLTYPE operator+ (String string1, const String& string2)
{
    return string1 += string2;
}

const String JUCE_CALLTYPE operator+ (String string1, const char* const string2)
{
    return string1 += string2;
}

const String JUCE_CALLTYPE operator+ (String string1, const juce_wchar* const string2)
{
    return string1 += string2;
}

const String JUCE_CALLTYPE operator+ (String string1, const char string2)
{
    return string1 += string2;
}

const String JUCE_CALLTYPE operator+ (String string1, const juce_wchar string2)
{
    return string1 += string2;
}

String& JUCE_CALLTYPE operator<< (String& string1, const char characterToAppend)
{
    return string1 += characterToAppend;
}

String& JUCE_CALLTYPE operator<< (String& string1, const juce_wchar characterToAppend)
{
    return string1 += characterToAppend;
}

String& JUCE_CALLTYPE operator<< (String& string1, const char* const string2)
{
    return string1 += string2;
}

String& JUCE_CALLTYPE operator<< (String& string1, const juce_wchar* const string2)
{
    return string1 += string2;
}

String& JUCE_CALLTYPE operator<< (String& string1, const String& string2)
{
    return string1 += string2;
}

String& JUCE_CALLTYPE operator<< (String& string1, const short number)
{
    return string1 += (int) number;
}

String& JUCE_CALLTYPE operator<< (String& string1, const int number)
{
    return string1 += number;
}

String& JUCE_CALLTYPE operator<< (String& string1, const unsigned int number)
{
    return string1 += number;
}

String& JUCE_CALLTYPE operator<< (String& string1, const long number)
{
    return string1 += (int) number;
}

String& JUCE_CALLTYPE operator<< (String& string1, const unsigned long number)
{
    return string1 += (unsigned int) number;
}

String& JUCE_CALLTYPE operator<< (String& string1, const float number)
{
    return string1 += String (number);
}

String& JUCE_CALLTYPE operator<< (String& string1, const double number)
{
    return string1 += String (number);
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const String& text)
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
int String::indexOfChar (const juce_wchar character) const throw()
{
    const juce_wchar* t = text;

    for (;;)
    {
        if (*t == character)
            return (int) (t - text);

        if (*t++ == 0)
            return -1;
    }
}

int String::lastIndexOfChar (const juce_wchar character) const throw()
{
    for (int i = length(); --i >= 0;)
        if (text[i] == character)
            return i;

    return -1;
}

int String::indexOf (const juce_wchar* const t) const throw()
{
    const juce_wchar* const r = CharacterFunctions::find (text, t);
    return (r == 0) ? -1
                    : (int) (r - text);
}

int String::indexOfChar (const int startIndex,
                         const juce_wchar character) const throw()
{
    if (startIndex >= 0 && startIndex >= length())
        return -1;

    const juce_wchar* t = text + jmax (0, startIndex);

    for (;;)
    {
        if (*t == character)
            return (int) (t - text);

        if (*t++ == 0)
            return -1;
    }
}

int String::indexOfAnyOf (const juce_wchar* const charactersToLookFor,
                          const int startIndex,
                          const bool ignoreCase) const throw()
{
    if (charactersToLookFor == 0
         || (startIndex >= 0 && startIndex >= length()))
        return -1;

    const juce_wchar* t = text + jmax (0, startIndex);

    while (*t != 0)
    {
        if (CharacterFunctions::indexOfChar (charactersToLookFor, *t, ignoreCase) >= 0)
            return (int) (t - text);

        ++t;
    }

    return -1;
}

int String::indexOf (const int startIndex,
                     const juce_wchar* const other) const throw()
{
    if (other == 0 || startIndex >= length())
        return -1;

    const juce_wchar* const found = CharacterFunctions::find (text + jmax (0, startIndex), other);

    return (found == 0) ? -1
                        : (int) (found - text);
}

int String::indexOfIgnoreCase (const juce_wchar* const other) const throw()
{
    if (other != 0 && *other != 0)
    {
        const int len = CharacterFunctions::length (other);
        const int end = length() - len;

        for (int i = 0; i <= end; ++i)
            if (CharacterFunctions::compareIgnoreCase (text + i, other, len) == 0)
                return i;
    }

    return -1;
}

int String::indexOfIgnoreCase (const int startIndex,
                               const juce_wchar* const other) const throw()
{
    if (other != 0 && *other != 0)
    {
        const int len = CharacterFunctions::length (other);
        const int end = length() - len;

        for (int i = jmax (0, startIndex); i <= end; ++i)
            if (CharacterFunctions::compareIgnoreCase (text + i, other, len) == 0)
                return i;
    }

    return -1;
}

int String::lastIndexOf (const juce_wchar* const other) const throw()
{
    if (other != 0 && *other != 0)
    {
        const int len = CharacterFunctions::length (other);
        int i = length() - len;

        if (i >= 0)
        {
            const juce_wchar* n = text + i;

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

int String::lastIndexOfIgnoreCase (const juce_wchar* const other) const throw()
{
    if (other != 0 && *other != 0)
    {
        const int len = CharacterFunctions::length (other);
        int i = length() - len;

        if (i >= 0)
        {
            const juce_wchar* n = text + i;

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

int String::lastIndexOfAnyOf (const juce_wchar* const charactersToLookFor,
                              const bool ignoreCase) const throw()
{
    for (int i = length(); --i >= 0;)
        if (CharacterFunctions::indexOfChar (charactersToLookFor, text[i], ignoreCase) >= 0)
            return i;

    return -1;
}

bool String::contains (const juce_wchar* const other) const throw()
{
    return indexOf (other) >= 0;
}

bool String::containsChar (const juce_wchar character) const throw()
{
    return indexOfChar (character) >= 0;
}

bool String::containsIgnoreCase (const juce_wchar* const t) const throw()
{
    return indexOfIgnoreCase (t) >= 0;
}

int String::indexOfWholeWord (const juce_wchar* const word) const throw()
{
    if (word != 0 && *word != 0)
    {
        const int wordLen = CharacterFunctions::length (word);
        const int end = length() - wordLen;
        const juce_wchar* t = text;

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

int String::indexOfWholeWordIgnoreCase (const juce_wchar* const word) const throw()
{
    if (word != 0 && *word != 0)
    {
        const int wordLen = CharacterFunctions::length (word);
        const int end = length() - wordLen;
        const juce_wchar* t = text;

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

bool String::containsWholeWord (const juce_wchar* const wordToLookFor) const throw()
{
    return indexOfWholeWord (wordToLookFor) >= 0;
}

bool String::containsWholeWordIgnoreCase (const juce_wchar* const wordToLookFor) const throw()
{
    return indexOfWholeWordIgnoreCase (wordToLookFor) >= 0;
}

//==============================================================================
static int indexOfMatch (const juce_wchar* const wildcard,
                         const juce_wchar* const test,
                         const bool ignoreCase) throw()
{
    int start = 0;

    while (test [start] != 0)
    {
        int i = 0;

        for (;;)
        {
            const juce_wchar wc = wildcard [i];
            const juce_wchar c = test [i + start];

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

bool String::matchesWildcard (const juce_wchar* wildcard, const bool ignoreCase) const throw()
{
    int i = 0;

    for (;;)
    {
        const juce_wchar wc = wildcard [i];
        const juce_wchar c = text [i];

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
                                                      text + i,
                                                      ignoreCase) >= 0);
        }
    }
}

//==============================================================================
const String String::repeatedString (const juce_wchar* const stringToRepeat, int numberOfTimesToRepeat)
{
    const int len = CharacterFunctions::length (stringToRepeat);
    String result ((size_t) (len * numberOfTimesToRepeat + 1), (int) 0);

    juce_wchar* n = result.text;
    n[0] = 0;

    while (--numberOfTimesToRepeat >= 0)
    {
        CharacterFunctions::append (n, stringToRepeat);
        n += len;
    }

    return result;
}

const String String::paddedLeft (const juce_wchar padCharacter, int minimumLength) const
{
    jassert (padCharacter != 0);

    const int len = length();

    if (len >= minimumLength || padCharacter == 0)
        return *this;

    String result ((size_t) minimumLength + 1, (int) 0);

    juce_wchar* n = result.text;

    minimumLength -= len;
    while (--minimumLength >= 0)
        *n++ = padCharacter;

    *n = 0;
    CharacterFunctions::append (n, text);

    return result;
}

const String String::paddedRight (const juce_wchar padCharacter, int minimumLength) const
{
    jassert (padCharacter != 0);

    const int paddingNeeded = minimumLength - length();
    if (paddingNeeded <= 0 || padCharacter == 0)
        return *this;

    return *this + String::empty.paddedLeft (padCharacter, paddingNeeded);
}

//==============================================================================
const String String::replaceSection (int index,
                                     int numCharsToReplace,
                                     const juce_wchar* const stringToInsert) const throw()
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

    String result ((size_t) newTotalLen, (int) 0);

    memcpy (result.text, text, index * sizeof (juce_wchar));

    if (newStringLen > 0)
        memcpy (result.text + index,
                stringToInsert,
                newStringLen * sizeof (juce_wchar));

    const int endStringLen = newTotalLen - (index + newStringLen);

    if (endStringLen > 0)
        memcpy (result.text + (index + newStringLen),
                text + (index + numCharsToReplace),
                endStringLen * sizeof (juce_wchar));

    result.text [newTotalLen] = 0;

    return result;
}

const String String::replace (const juce_wchar* const stringToReplace,
                              const juce_wchar* const stringToInsert,
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

const String String::replaceCharacter (const juce_wchar charToReplace,
                                       const juce_wchar charToInsert) const throw()
{
    const int index = indexOfChar (charToReplace);

    if (index < 0)
        return *this;

    String result (*this);
    result.dupeInternalIfMultiplyReferenced();

    juce_wchar* t = result.text + index;

    while (*t != 0)
    {
        if (*t == charToReplace)
            *t = charToInsert;

        ++t;
    }

    return result;
}

const String String::replaceCharacters (const String& charactersToReplace,
                                        const juce_wchar* const charactersToInsertInstead) const throw()
{
    String result (*this);
    result.dupeInternalIfMultiplyReferenced();

    juce_wchar* t = result.text;
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
bool String::startsWith (const juce_wchar* const other) const throw()
{
    return other != 0
            && CharacterFunctions::compare (text, other, CharacterFunctions::length (other)) == 0;
}

bool String::startsWithIgnoreCase (const juce_wchar* const other) const throw()
{
    return other != 0
            && CharacterFunctions::compareIgnoreCase (text, other, CharacterFunctions::length (other)) == 0;
}

bool String::startsWithChar (const juce_wchar character) const throw()
{
    jassert (character != 0); // strings can't contain a null character!

    return text[0] == character;
}

bool String::endsWithChar (const juce_wchar character) const throw()
{
    jassert (character != 0); // strings can't contain a null character!

    return text[0] != 0
            && text [length() - 1] == character;
}

bool String::endsWith (const juce_wchar* const other) const throw()
{
    if (other == 0)
        return false;

    const int thisLen = length();
    const int otherLen = CharacterFunctions::length (other);

    return thisLen >= otherLen
            && CharacterFunctions::compare (text + thisLen - otherLen, other) == 0;
}

bool String::endsWithIgnoreCase (const juce_wchar* const other) const throw()
{
    if (other == 0)
        return false;

    const int thisLen = length();
    const int otherLen = CharacterFunctions::length (other);

    return thisLen >= otherLen
            && CharacterFunctions::compareIgnoreCase (text + thisLen - otherLen, other) == 0;
}

//==============================================================================
const String String::toUpperCase() const throw()
{
    String result (*this);
    result.dupeInternalIfMultiplyReferenced();
    CharacterFunctions::toUpperCase (result.text);
    return result;
}

const String String::toLowerCase() const throw()
{
    String result (*this);
    result.dupeInternalIfMultiplyReferenced();
    CharacterFunctions::toLowerCase (result.text);
    return result;
}

//==============================================================================
juce_wchar& String::operator[] (const int index) throw()
{
    jassert (((unsigned int) index) <= (unsigned int) length());

    dupeInternalIfMultiplyReferenced();

    return text [index];
}

juce_wchar String::getLastCharacter() const throw()
{
    return (isEmpty()) ? ((juce_wchar) 0)
                       : text [length() - 1];
}

const String String::substring (int start, int end) const throw()
{
    if (start < 0)
        start = 0;
    else if (end <= start)
        return empty;

    int len = 0;
    const juce_wchar* const t = text;

    while (len <= end && t [len] != 0)
        ++len;

    if (end >= len)
    {
        if (start == 0)
            return *this;

        end = len;
    }

    return String (text + start,
                   end - start);
}

const String String::substring (const int start) const throw()
{
    if (start <= 0)
        return *this;

    const int len = length();

    if (start >= len)
        return empty;
    else
        return String (text + start, len - start);
}

const String String::dropLastCharacters (const int numberToDrop) const throw()
{
    return String (text, jmax (0, length() - numberToDrop));
}

const String String::getLastCharacters (const int numCharacters) const throw()
{
    return String (text + jmax (0, length() - jmax (0, numCharacters)));
}

const String String::fromFirstOccurrenceOf (const juce_wchar* const sub,
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


const String String::fromLastOccurrenceOf (const juce_wchar* const sub,
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

const String String::upToFirstOccurrenceOf (const juce_wchar* const sub,
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

const String String::upToLastOccurrenceOf (const juce_wchar* const sub,
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

const String String::quoted (const juce_wchar quoteCharacter) const throw()
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

    while (CharacterFunctions::isWhitespace (text [start]))
        ++start;

    const int len = length();
    int end = len - 1;

    while ((end >= start) && CharacterFunctions::isWhitespace (text [end]))
        --end;

    ++end;

    if (end <= start)
        return empty;
    else if (start > 0 || end < len)
        return String (text + start, end - start);
    else
        return *this;
}

const String String::trimStart() const throw()
{
    if (isEmpty())
        return empty;

    const juce_wchar* t = text;

    while (CharacterFunctions::isWhitespace (*t))
        ++t;

    if (t == text)
        return *this;
    else
        return String (t);
}

const String String::trimEnd() const throw()
{
    if (isEmpty())
        return empty;

    const juce_wchar* endT = text + (length() - 1);

    while ((endT >= text) && CharacterFunctions::isWhitespace (*endT))
        --endT;

    return String (text, (int) (++endT - text));
}

const String String::trimCharactersAtStart (const juce_wchar* charactersToTrim) const throw()
{
    jassert (charactersToTrim != 0);

    if (isEmpty())
        return empty;

    const juce_wchar* t = text;

    while (CharacterFunctions::indexOfCharFast (charactersToTrim, *t) >= 0)
        ++t;

    if (t == text)
        return *this;
    else
        return String (t);
}

const String String::trimCharactersAtEnd (const juce_wchar* charactersToTrim) const throw()
{
    jassert (charactersToTrim != 0);

    if (isEmpty())
        return empty;

    const juce_wchar* endT = text + (length() - 1);

    while ((endT >= text) && CharacterFunctions::indexOfCharFast (charactersToTrim, *endT) >= 0)
        --endT;

    return String (text, (int) (++endT - text));
}

//==============================================================================
const String String::retainCharacters (const juce_wchar* const charactersToRetain) const throw()
{
    jassert (charactersToRetain != 0);

    if (isEmpty())
        return empty;

    String result (StringHolder::getAllocatedNumChars (text), (int) 0);
    juce_wchar* dst = result.text;
    const juce_wchar* src = text;

    while (*src != 0)
    {
        if (CharacterFunctions::indexOfCharFast (charactersToRetain, *src) >= 0)
            *dst++ = *src;

        ++src;
    }

    *dst = 0;
    return result;
}

const String String::removeCharacters (const juce_wchar* const charactersToRemove) const throw()
{
    jassert (charactersToRemove != 0);

    if (isEmpty())
        return empty;

    String result (StringHolder::getAllocatedNumChars (text), (int) 0);
    juce_wchar* dst = result.text;
    const juce_wchar* src = text;

    while (*src != 0)
    {
        if (CharacterFunctions::indexOfCharFast (charactersToRemove, *src) < 0)
            *dst++ = *src;

        ++src;
    }

    *dst = 0;
    return result;
}

const String String::initialSectionContainingOnly (const juce_wchar* const permittedCharacters) const throw()
{
    return substring (0, CharacterFunctions::getIntialSectionContainingOnly (text, permittedCharacters));
}

const String String::initialSectionNotContaining (const juce_wchar* const charactersToStopAt) const throw()
{
    jassert (charactersToStopAt != 0);

    const juce_wchar* const t = text;
    int i = 0;

    while (t[i] != 0)
    {
        if (CharacterFunctions::indexOfCharFast (charactersToStopAt, t[i]) >= 0)
            return String (text, i);

        ++i;
    }

    return empty;
}

bool String::containsOnly (const juce_wchar* const chars) const throw()
{
    jassert (chars != 0);

    const juce_wchar* t = text;

    while (*t != 0)
        if (CharacterFunctions::indexOfCharFast (chars, *t++) < 0)
            return false;

    return true;
}

bool String::containsAnyOf (const juce_wchar* const chars) const throw()
{
    jassert (chars != 0);

    const juce_wchar* t = text;

    while (*t != 0)
        if (CharacterFunctions::indexOfCharFast (chars, *t++) >= 0)
            return true;

    return false;
}

bool String::containsNonWhitespaceChars() const throw()
{
    const juce_wchar* t = text;

    while (*t != 0)
        if (! CharacterFunctions::isWhitespace (*t++))
            return true;

    return false;
}


//==============================================================================
int String::getIntValue() const throw()
{
    return CharacterFunctions::getIntValue (text);
}

int String::getTrailingIntValue() const throw()
{
    int n = 0;
    int mult = 1;
    const juce_wchar* t = text + length();

    while (--t >= text)
    {
        const juce_wchar c = *t;

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
    return CharacterFunctions::getInt64Value (text);
}

float String::getFloatValue() const throw()
{
    return (float) CharacterFunctions::getDoubleValue (text);
}

double String::getDoubleValue() const throw()
{
    return CharacterFunctions::getDoubleValue (text);
}

static const juce_wchar* const hexDigits = T("0123456789abcdef");

const String String::toHexString (const int number) throw()
{
    juce_wchar buffer[32];
    juce_wchar* const end = buffer + 32;
    juce_wchar* t = end;
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
    juce_wchar buffer[32];
    juce_wchar* const end = buffer + 32;
    juce_wchar* t = end;
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

    String s ((size_t) numChars, (int) 0);

    juce_wchar* d = s.text;

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
    const juce_wchar* c = text;

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
    const juce_wchar* c = text;

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
        juce_wchar* const dst = const_cast <juce_wchar*> ((const juce_wchar*) result);

        if (bigEndian)
        {
            for (int i = 0; i < numChars; ++i)
                dst[i] = (juce_wchar) ByteOrder::swapIfLittleEndian (src[i]);
        }
        else
        {
            for (int i = 0; i < numChars; ++i)
                dst[i] = (juce_wchar) ByteOrder::swapIfBigEndian (src[i]);
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
        return reinterpret_cast <const char*> (text);
    }
    else
    {
        const int currentLen = length() + 1;
        const int utf8BytesNeeded = getNumBytesAsUTF8();

        String* const mutableThis = const_cast <String*> (this);
        mutableThis->text = StringHolder::makeUniqueWithSize (mutableThis->text, currentLen + 1 + utf8BytesNeeded / sizeof (juce_wchar));

        char* const otherCopy = (char*) (text + currentLen);
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
        const uint32 c = (uint32) text [index++];

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
    const juce_wchar* t = text;

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

    String result ((size_t) numBytes + 1, (int) 0);
    juce_wchar* dest = result.text;

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
        return reinterpret_cast <const char*> (text);
    }
    else
    {
        int len = length();

        String* const mutableThis = const_cast <String*> (this);
        mutableThis->text = StringHolder::makeUniqueWithSize (mutableThis->text, (len + 1) * 2);

        char* otherCopy = (char*) (text + len + 1);
        CharacterFunctions::copy (otherCopy, text, len);
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
    return (int) wcstombs (0, text, 0);
}

int String::copyToCString (char* destBuffer, const int maxBufferSizeBytes) const throw()
{
    const int numBytes = (int) wcstombs (destBuffer, text, maxBufferSizeBytes);

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
    memcpy (destBuffer, text, len * sizeof (juce_wchar));
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
