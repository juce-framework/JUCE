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
  #pragma warning (push)
  #pragma warning (disable: 4514)
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
    StringHolder()
        : refCount (0x3fffffff), allocatedNumChars (0)
    {
        text[0] = 0;
    }

    //==============================================================================
    static juce_wchar* createUninitialised (const size_t numChars)
    {
        StringHolder* const s = reinterpret_cast <StringHolder*> (new char [sizeof (StringHolder) + numChars * sizeof (juce_wchar)]);
        s->refCount = 0;
        s->allocatedNumChars = numChars;
        return &(s->text[0]);
    }

    static juce_wchar* createCopy (const juce_wchar* const src, const size_t numChars)
    {
        juce_wchar* const dest = createUninitialised (numChars);
        copyChars (dest, src, numChars);
        return dest;
    }

    static juce_wchar* createCopy (const char* const src, const size_t numChars)
    {
        juce_wchar* const dest = createUninitialised (numChars);
        CharacterFunctions::copy (dest, src, numChars);
        dest [numChars] = 0;
        return dest;
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
            delete[] reinterpret_cast <char*> (b);
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

        juce_wchar* const newText = createCopy (text, b->allocatedNumChars);
        release (b);

        return newText;
    }

    static juce_wchar* makeUniqueWithSize (juce_wchar* const text, size_t numChars)
    {
        StringHolder* const b = bufferFromText (text);

        if (b->refCount <= 0 && b->allocatedNumChars >= numChars)
            return text;

        juce_wchar* const newText = createUninitialised (jmax (b->allocatedNumChars, numChars));
        copyChars (newText, text, b->allocatedNumChars);
        release (b);

        return newText;
    }

    static size_t getAllocatedNumChars (juce_wchar* const text) throw()
    {
        return bufferFromText (text)->allocatedNumChars;
    }

    static void copyChars (juce_wchar* const dest, const juce_wchar* const src, const size_t numChars) throw()
    {
        memcpy (dest, src, numChars * sizeof (juce_wchar));
        dest [numChars] = 0;
    }

    //==============================================================================
    int refCount;
    size_t allocatedNumChars;
    juce_wchar text[1];

    static StringHolder empty;

private:
    static inline StringHolder* bufferFromText (void* const text) throw()
    {
        // (Can't use offsetof() here because of warnings about this not being a POD)
        return reinterpret_cast <StringHolder*> (static_cast <char*> (text)
                    - (reinterpret_cast <size_t> (reinterpret_cast <StringHolder*> (1)->text) - 1));
    }
};

StringHolder StringHolder::empty;
const String String::empty;

//==============================================================================
void String::createInternal (const juce_wchar* const t, const size_t numChars)
{
    jassert (t[numChars] == 0); // must have a null terminator

    text = StringHolder::createCopy (t, numChars);
}

void String::appendInternal (const juce_wchar* const newText, const int numExtraChars)
{
    if (numExtraChars > 0)
    {
        const int oldLen = length();
        const int newTotalLen = oldLen + numExtraChars;

        text = StringHolder::makeUniqueWithSize (text, newTotalLen);
        StringHolder::copyChars (text + oldLen, newText, numExtraChars);
    }
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

void String::swapWith (String& other) throw()
{
    swapVariables (text, other.text);
}

String& String::operator= (const String& other) throw()
{
    juce_wchar* const newText = other.text;
    StringHolder::retain (newText);
    StringHolder::release (static_cast <juce_wchar*> (Atomic::swapPointers ((void* volatile*) &text, newText)));
    return *this;
}

String::String (const size_t numChars, const int /*dummyVariable*/)
    : text (StringHolder::createUninitialised (numChars))
{
}

String::String (const String& stringToCopy, const size_t charsToAllocate)
{
    const size_t otherSize = StringHolder::getAllocatedNumChars (stringToCopy.text);
    text = StringHolder::createUninitialised (jmax (charsToAllocate, otherSize));
    StringHolder::copyChars (text, stringToCopy.text, otherSize);
}

String::String (const char* const t)
{
    if (t != 0 && *t != 0)
        text = StringHolder::createCopy (t, CharacterFunctions::length (t));
    else
        text = StringHolder::getEmpty();
}

String::String (const juce_wchar* const t)
{
    if (t != 0 && *t != 0)
        text = StringHolder::createCopy (t, CharacterFunctions::length (t));
    else
        text = StringHolder::getEmpty();
}

String::String (const char* const t, const size_t maxChars)
{
    int i;
    for (i = 0; (size_t) i < maxChars; ++i)
        if (t[i] == 0)
            break;

    if (i > 0)
        text = StringHolder::createCopy (t, i);
    else
        text = StringHolder::getEmpty();
}

String::String (const juce_wchar* const t, const size_t maxChars)
{
    int i;
    for (i = 0; (size_t) i < maxChars; ++i)
        if (t[i] == 0)
            break;

    if (i > 0)
        text = StringHolder::createCopy (t, i);
    else
        text = StringHolder::getEmpty();
}

const String String::charToString (const juce_wchar character)
{
    String result ((size_t) 1, (int) 0);
    result.text[0] = character;
    result.text[1] = 0;
    return result;
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
            *--t = (juce_wchar) ('0' + (int) (v % 10));
            v /= 10;

        } while (v > 0);

        if (n < 0)
            *--t = '-';

        return t;
    }

    static juce_wchar* uint64ToString (juce_wchar* t, int64 v) throw()
    {
        *--t = 0;

        do
        {
            *--t = (juce_wchar) ('0' + (int) (v % 10));
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
            *--t = (juce_wchar) ('0' + (v % 10));
            v /= 10;

        } while (v > 0);

        if (n < 0)
            *--t = '-';

        return t;
    }

    static juce_wchar* uintToString (juce_wchar* t, unsigned int v) throw()
    {
        *--t = 0;

        do
        {
            *--t = (juce_wchar) ('0' + (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    static juce_wchar getDecimalPoint()
    {
#if JUCE_WINDOWS && _MSC_VER < 1400
        static juce_wchar dp = std::_USE (std::locale(), std::numpunct <wchar_t>).decimal_point();
#else
        static juce_wchar dp = std::use_facet <std::numpunct <wchar_t> > (std::locale()).decimal_point();
#endif
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

                *--t = (juce_wchar) ('0' + (v % 10));

                v /= 10;
                --numDecPlaces;
            }

            if (n < 0)
                *--t = '-';

            len = end - t - 1;
            return t;
        }
        else
        {
#if JUCE_WIN32
  #if _MSC_VER <= 1400
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
String::String (const int number)
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::intToString (end, number);
    createInternal (start, end - start - 1);
}

String::String (const unsigned int number)
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::uintToString (end, number);
    createInternal (start, end - start - 1);
}

String::String (const short number)
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::intToString (end, (int) number);
    createInternal (start, end - start - 1);
}

String::String (const unsigned short number)
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::uintToString (end, (unsigned int) number);
    createInternal (start, end - start - 1);
}

String::String (const int64 number)
{
    juce_wchar buffer [32];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::int64ToString (end, number);
    createInternal (start, end - start - 1);
}

String::String (const uint64 number)
{
    juce_wchar buffer [32];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::uint64ToString (end, number);
    createInternal (start, end - start - 1);
}

String::String (const float number, const int numberOfDecimalPlaces)
{
    juce_wchar buffer [48];
    size_t len;
    juce_wchar* start = NumberToStringConverters::doubleToString (buffer, numElementsInArray (buffer), (double) number, numberOfDecimalPlaces, len);
    createInternal (start, len);
}

String::String (const double number, const int numberOfDecimalPlaces)
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

bool String::equalsIgnoreCase (const char* t) const throw()
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
    return operator+= (static_cast <const juce_wchar*> (asString));
}

String& String::operator+= (const juce_wchar ch)
{
    const juce_wchar asString[] = { (juce_wchar) ch, 0 };
    return operator+= (static_cast <const juce_wchar*> (asString));
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

int String::indexOf (const String& t) const throw()
{
    const juce_wchar* const r = CharacterFunctions::find (text, t.text);
    return r == 0 ? -1 : (int) (r - text);
}

int String::indexOfChar (const int startIndex,
                         const juce_wchar character) const throw()
{
    if (startIndex > 0 && startIndex >= length())
        return -1;

    const juce_wchar* t = text + jmax (0, startIndex);

    for (;;)
    {
        if (*t == character)
            return (int) (t - text);

        if (*t == 0)
            return -1;

        ++t;
    }
}

int String::indexOfAnyOf (const String& charactersToLookFor,
                          const int startIndex,
                          const bool ignoreCase) const throw()
{
    if (startIndex > 0 && startIndex >= length())
        return -1;

    const juce_wchar* t = text + jmax (0, startIndex);

    while (*t != 0)
    {
        if (CharacterFunctions::indexOfChar (charactersToLookFor.text, *t, ignoreCase) >= 0)
            return (int) (t - text);

        ++t;
    }

    return -1;
}

int String::indexOf (const int startIndex, const String& other) const throw()
{
    if (startIndex > 0 && startIndex >= length())
        return -1;

    const juce_wchar* const found = CharacterFunctions::find (text + jmax (0, startIndex), other.text);

    return found == 0 ? -1 : (int) (found - text);
}

int String::indexOfIgnoreCase (const String& other) const throw()
{
    if (other.isNotEmpty())
    {
        const int len = other.length();
        const int end = length() - len;

        for (int i = 0; i <= end; ++i)
            if (CharacterFunctions::compareIgnoreCase (text + i, other.text, len) == 0)
                return i;
    }

    return -1;
}

int String::indexOfIgnoreCase (const int startIndex, const String& other) const throw()
{
    if (other.isNotEmpty())
    {
        const int len = other.length();
        const int end = length() - len;

        for (int i = jmax (0, startIndex); i <= end; ++i)
            if (CharacterFunctions::compareIgnoreCase (text + i, other.text, len) == 0)
                return i;
    }

    return -1;
}

int String::lastIndexOf (const String& other) const throw()
{
    if (other.isNotEmpty())
    {
        const int len = other.length();
        int i = length() - len;

        if (i >= 0)
        {
            const juce_wchar* n = text + i;

            while (i >= 0)
            {
                if (CharacterFunctions::compare (n--, other.text, len) == 0)
                    return i;

                --i;
            }
        }
    }

    return -1;
}

int String::lastIndexOfIgnoreCase (const String& other) const throw()
{
    if (other.isNotEmpty())
    {
        const int len = other.length();
        int i = length() - len;

        if (i >= 0)
        {
            const juce_wchar* n = text + i;

            while (i >= 0)
            {
                if (CharacterFunctions::compareIgnoreCase (n--, other.text, len) == 0)
                    return i;

                --i;
            }
        }
    }

    return -1;
}

int String::lastIndexOfAnyOf (const String& charactersToLookFor, const bool ignoreCase) const throw()
{
    for (int i = length(); --i >= 0;)
        if (CharacterFunctions::indexOfChar (charactersToLookFor.text, text[i], ignoreCase) >= 0)
            return i;

    return -1;
}

bool String::contains (const String& other) const throw()
{
    return indexOf (other) >= 0;
}

bool String::containsChar (const juce_wchar character) const throw()
{
    const juce_wchar* t = text;

    for (;;)
    {
        if (*t == 0)
            return false;

        if (*t == character)
            return true;

        ++t;
    }
}

bool String::containsIgnoreCase (const String& t) const throw()
{
    return indexOfIgnoreCase (t) >= 0;
}

int String::indexOfWholeWord (const String& word) const throw()
{
    if (word.isNotEmpty())
    {
        const int wordLen = word.length();
        const int end = length() - wordLen;
        const juce_wchar* t = text;

        for (int i = 0; i <= end; ++i)
        {
            if (CharacterFunctions::compare (t, word.text, wordLen) == 0
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

int String::indexOfWholeWordIgnoreCase (const String& word) const throw()
{
    if (word.isNotEmpty())
    {
        const int wordLen = word.length();
        const int end = length() - wordLen;
        const juce_wchar* t = text;

        for (int i = 0; i <= end; ++i)
        {
            if (CharacterFunctions::compareIgnoreCase (t, word.text, wordLen) == 0
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

bool String::containsWholeWord (const String& wordToLookFor) const throw()
{
    return indexOfWholeWord (wordToLookFor) >= 0;
}

bool String::containsWholeWordIgnoreCase (const String& wordToLookFor) const throw()
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
                 || (wc == '?' && c != 0))
            {
                if (wc == 0)
                    return start;

                ++i;
            }
            else
            {
                if (wc == '*' && (wildcard [i + 1] == 0
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

bool String::matchesWildcard (const String& wildcard, const bool ignoreCase) const throw()
{
    int i = 0;

    for (;;)
    {
        const juce_wchar wc = wildcard.text [i];
        const juce_wchar c = text [i];

        if (wc == c
             || (ignoreCase && CharacterFunctions::toLowerCase (wc) == CharacterFunctions::toLowerCase (c))
             || (wc == '?' && c != 0))
        {
            if (wc == 0)
                return true;

            ++i;
        }
        else
        {
            return wc == '*' && (wildcard [i + 1] == 0
                                  || indexOfMatch (wildcard.text + i + 1,
                                                   text + i,
                                                   ignoreCase) >= 0);
        }
    }
}

//==============================================================================
const String String::repeatedString (const String& stringToRepeat, int numberOfTimesToRepeat)
{
    const int len = stringToRepeat.length();
    String result ((size_t) (len * numberOfTimesToRepeat + 1), (int) 0);
    juce_wchar* n = result.text;
    *n = 0;

    while (--numberOfTimesToRepeat >= 0)
    {
        StringHolder::copyChars (n, stringToRepeat.text, len);
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

    StringHolder::copyChars (n, text, len);
    return result;
}

const String String::paddedRight (const juce_wchar padCharacter, int minimumLength) const
{
    jassert (padCharacter != 0);

    const int len = length();
    if (len >= minimumLength || padCharacter == 0)
        return *this;

    String result (*this, (size_t) minimumLength);
    juce_wchar* n = result.text + len;

    minimumLength -= len;
    while (--minimumLength >= 0)
        *n++ = padCharacter;

    *n = 0;
    return result;
}

//==============================================================================
const String String::replaceSection (int index, int numCharsToReplace, const String& stringToInsert) const
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

    const int newStringLen = stringToInsert.length();
    const int newTotalLen = len + newStringLen - numCharsToReplace;

    if (newTotalLen <= 0)
        return String::empty;

    String result ((size_t) newTotalLen, (int) 0);

    StringHolder::copyChars (result.text, text, index);

    if (newStringLen > 0)
        StringHolder::copyChars (result.text + index, stringToInsert.text, newStringLen);

    const int endStringLen = newTotalLen - (index + newStringLen);

    if (endStringLen > 0)
        StringHolder::copyChars (result.text + (index + newStringLen),
                                 text + (index + numCharsToReplace),
                                 endStringLen);

    return result;
}

const String String::replace (const String& stringToReplace, const String& stringToInsert, const bool ignoreCase) const
{
    const int stringToReplaceLen = stringToReplace.length();
    const int stringToInsertLen = stringToInsert.length();

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

const String String::replaceCharacter (const juce_wchar charToReplace, const juce_wchar charToInsert) const
{
    const int index = indexOfChar (charToReplace);

    if (index < 0)
        return *this;

    String result (*this, size_t());
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
                                        const String& charactersToInsertInstead) const
{
    String result (*this, size_t());
    juce_wchar* t = result.text;
    const int len2 = charactersToInsertInstead.length();

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
bool String::startsWith (const String& other) const throw()
{
    return CharacterFunctions::compare (text, other.text, other.length()) == 0;
}

bool String::startsWithIgnoreCase (const String& other) const throw()
{
    return CharacterFunctions::compareIgnoreCase (text, other.text, other.length()) == 0;
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

bool String::endsWith (const String& other) const throw()
{
    const int thisLen = length();
    const int otherLen = other.length();

    return thisLen >= otherLen
            && CharacterFunctions::compare (text + thisLen - otherLen, other.text) == 0;
}

bool String::endsWithIgnoreCase (const String& other) const throw()
{
    const int thisLen = length();
    const int otherLen = other.length();

    return thisLen >= otherLen
            && CharacterFunctions::compareIgnoreCase (text + thisLen - otherLen, other.text) == 0;
}

//==============================================================================
const String String::toUpperCase() const
{
    String result (*this, size_t());
    CharacterFunctions::toUpperCase (result.text);
    return result;
}

const String String::toLowerCase() const
{
    String result (*this, size_t());
    CharacterFunctions::toLowerCase (result.text);
    return result;
}

//==============================================================================
juce_wchar& String::operator[] (const int index)
{
    jassert (((unsigned int) index) <= (unsigned int) length());
    text = StringHolder::makeUnique (text);
    return text [index];
}

juce_wchar String::getLastCharacter() const throw()
{
    return isEmpty() ? juce_wchar() : text [length() - 1];
}

const String String::substring (int start, int end) const
{
    if (start < 0)
        start = 0;
    else if (end <= start)
        return empty;

    int len = 0;
    while (len <= end && text [len] != 0)
        ++len;

    if (end >= len)
    {
        if (start == 0)
            return *this;

        end = len;
    }

    return String (text + start, end - start);
}

const String String::substring (const int start) const
{
    if (start <= 0)
        return *this;

    const int len = length();

    if (start >= len)
        return empty;

    return String (text + start, len - start);
}

const String String::dropLastCharacters (const int numberToDrop) const
{
    return String (text, jmax (0, length() - numberToDrop));
}

const String String::getLastCharacters (const int numCharacters) const
{
    return String (text + jmax (0, length() - jmax (0, numCharacters)));
}

const String String::fromFirstOccurrenceOf (const String& sub,
                                            const bool includeSubString,
                                            const bool ignoreCase) const
{
    const int i = ignoreCase ? indexOfIgnoreCase (sub)
                             : indexOf (sub);
    if (i < 0)
        return empty;

    return substring (includeSubString ? i : i + sub.length());
}

const String String::fromLastOccurrenceOf (const String& sub,
                                           const bool includeSubString,
                                           const bool ignoreCase) const
{
    const int i = ignoreCase ? lastIndexOfIgnoreCase (sub)
                             : lastIndexOf (sub);
    if (i < 0)
        return *this;

    return substring (includeSubString ? i : i + sub.length());
}

const String String::upToFirstOccurrenceOf (const String& sub,
                                            const bool includeSubString,
                                            const bool ignoreCase) const
{
    const int i = ignoreCase ? indexOfIgnoreCase (sub)
                             : indexOf (sub);
    if (i < 0)
        return *this;

    return substring (0, includeSubString ? i + sub.length() : i);
}

const String String::upToLastOccurrenceOf (const String& sub,
                                           const bool includeSubString,
                                           const bool ignoreCase) const
{
    const int i = ignoreCase ? lastIndexOfIgnoreCase (sub)
                             : lastIndexOf (sub);
    if (i < 0)
        return *this;

    return substring (0, includeSubString ? i + sub.length() : i);
}

bool String::isQuotedString() const
{
    const String trimmed (trimStart());

    return trimmed[0] == '"'
        || trimmed[0] == '\'';
}

const String String::unquoted() const
{
    String s (*this);

    if (s.text[0] == '"' || s.text[0] == '\'')
        s = s.substring (1);

    const int lastCharIndex = s.length() - 1;

    if (lastCharIndex >= 0
         && (s [lastCharIndex] == '"' || s[lastCharIndex] == '\''))
        s [lastCharIndex] = 0;

    return s;
}

const String String::quoted (const juce_wchar quoteCharacter) const
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
const String String::trim() const
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

    return *this;
}

const String String::trimStart() const
{
    if (isEmpty())
        return empty;

    const juce_wchar* t = text;

    while (CharacterFunctions::isWhitespace (*t))
        ++t;

    if (t == text)
        return *this;

    return String (t);
}

const String String::trimEnd() const
{
    if (isEmpty())
        return empty;

    const juce_wchar* endT = text + (length() - 1);

    while ((endT >= text) && CharacterFunctions::isWhitespace (*endT))
        --endT;

    return String (text, (int) (++endT - text));
}

const String String::trimCharactersAtStart (const String& charactersToTrim) const
{
    const juce_wchar* t = text;

    while (charactersToTrim.containsChar (*t))
        ++t;

    if (t == text)
        return *this;

    return String (t);
}

const String String::trimCharactersAtEnd (const String& charactersToTrim) const
{
    if (isEmpty())
        return empty;

    const juce_wchar* endT = text + (length() - 1);

    while (endT >= text && charactersToTrim.containsChar (*endT))
        --endT;

    return String (text, (int) (++endT - text));
}

//==============================================================================
const String String::retainCharacters (const String& charactersToRetain) const
{
    if (isEmpty())
        return empty;

    String result (StringHolder::getAllocatedNumChars (text), (int) 0);
    juce_wchar* dst = result.text;
    const juce_wchar* src = text;

    while (*src != 0)
    {
        if (charactersToRetain.containsChar (*src))
            *dst++ = *src;

        ++src;
    }

    *dst = 0;
    return result;
}

const String String::removeCharacters (const String& charactersToRemove) const
{
    if (isEmpty())
        return empty;

    String result (StringHolder::getAllocatedNumChars (text), (int) 0);
    juce_wchar* dst = result.text;
    const juce_wchar* src = text;

    while (*src != 0)
    {
        if (! charactersToRemove.containsChar (*src))
            *dst++ = *src;

        ++src;
    }

    *dst = 0;
    return result;
}

const String String::initialSectionContainingOnly (const String& permittedCharacters) const
{
    int i = 0;

    for (;;)
    {
        if (! permittedCharacters.containsChar (text[i]))
            break;

        ++i;
    }

    return substring (0, i);
}

const String String::initialSectionNotContaining (const String& charactersToStopAt) const
{
    const juce_wchar* const t = text;
    int i = 0;

    while (t[i] != 0)
    {
        if (charactersToStopAt.containsChar (t[i]))
            return String (text, i);

        ++i;
    }

    return empty;
}

bool String::containsOnly (const String& chars) const throw()
{
    const juce_wchar* t = text;

    while (*t != 0)
        if (! chars.containsChar (*t++))
            return false;

    return true;
}

bool String::containsAnyOf (const String& chars) const throw()
{
    const juce_wchar* t = text;

    while (*t != 0)
        if (chars.containsChar (*t++))
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

const String String::formatted (const juce_wchar* const pf, ... )
{
    jassert (pf != 0);

    va_list args;
    va_start (args, pf);

    size_t bufferSize = 256;
    String result (bufferSize, (int) 0);
    result.text[0] = 0;

    for (;;)
    {
#if JUCE_LINUX && JUCE_64BIT
        va_list tempArgs;
        va_copy (tempArgs, args);
        const int num = (int) vswprintf (result.text, bufferSize - 1, pf, tempArgs);
        va_end (tempArgs);
#elif JUCE_WINDOWS
        #ifdef _MSC_VER
          #pragma warning (push)
          #pragma warning (disable: 4996)
        #endif
        const int num = (int) _vsnwprintf (result.text, bufferSize - 1, pf, args);
        #ifdef _MSC_VER
          #pragma warning (pop)
        #endif
#else
        const int num = (int) vswprintf (result.text, bufferSize - 1, pf, args);
#endif

        if (num > 0)
            return result;

        bufferSize += 256;

        if (num == 0 || bufferSize > 65536) // the upper limit is a sanity check to avoid situations where vprintf repeatedly
            break;                          // returns -1 because of an error rather than because it needs more space.

        result.preallocateStorage (bufferSize);
    }

    return empty;
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
            if (c == '-')
                n = -n;

            break;
        }

        n += mult * (c - '0');
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

const String String::toHexString (const int number)
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

const String String::toHexString (const int64 number)
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

const String String::toHexString (const short number)
{
    return toHexString ((int) (unsigned short) number);
}

const String String::toHexString (const unsigned char* data,
                                  const int size,
                                  const int groupSize)
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
            *d++ = ' ';
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
const String String::createStringFromData (const void* const data_, const int size)
{
    const char* const data = static_cast <const char*> (data_);

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
        juce_wchar* const dst = const_cast <juce_wchar*> (static_cast <const juce_wchar*> (result));

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

        char* const otherCopy = reinterpret_cast <char*> (mutableThis->text + currentLen);
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
        const int len = length();
        String* const mutableThis = const_cast <String*> (this);
        mutableThis->text = StringHolder::makeUniqueWithSize (mutableThis->text, (len + 1) * 2);

        char* otherCopy = reinterpret_cast <char*> (mutableThis->text + len + 1);
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
    StringHolder::copyChars (destBuffer, text, jmin (maxCharsToCopy, length()));
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
        s.copyToUnicode (static_cast <juce_wchar*> (result) + nextIndex, len);
        nextIndex += len;
    }
}


END_JUCE_NAMESPACE
