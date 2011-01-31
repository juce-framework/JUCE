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

#include <locale>

BEGIN_JUCE_NAMESPACE

#include "juce_String.h"
#include "../memory/juce_Atomic.h"
#include "../io/streams/juce_OutputStream.h"

#if defined (JUCE_STRINGS_ARE_UNICODE) && ! JUCE_STRINGS_ARE_UNICODE
 #error "JUCE_STRINGS_ARE_UNICODE is deprecated! All strings are now unicode by default."
#endif

NewLine newLine;

//==============================================================================
class StringHolder
{
public:
    StringHolder()
        : refCount (0x3fffffff), allocatedNumChars (0)
    {
        text[0] = 0;
    }

    typedef String::CharPointerType CharPointerType;

    //==============================================================================
    static const CharPointerType createUninitialised (const size_t numChars)
    {
        StringHolder* const s = reinterpret_cast <StringHolder*> (new char [sizeof (StringHolder) + numChars * sizeof (juce_wchar)]);
        s->refCount.value = 0;
        s->allocatedNumChars = numChars;
        return CharPointerType (&(s->text[0]));
    }

    template <class CharPointer>
    static const CharPointerType createFromCharPointer (const CharPointer& text)
    {
        if (text.getAddress() == 0 || text.isEmpty())
            return getEmpty();

        const size_t numChars = text.length();
        const CharPointerType dest (createUninitialised (numChars));
        CharPointerType (dest).writeAll (text);
        return dest;
    }

    template <class CharPointer>
    static const CharPointerType createFromCharPointer (const CharPointer& text, size_t maxChars)
    {
        if (text.getAddress() == 0 || text.isEmpty())
            return getEmpty();

        size_t numChars = text.lengthUpTo (maxChars);
        if (numChars == 0)
            return getEmpty();

        const CharPointerType dest (createUninitialised (numChars));
        CharPointerType (dest).writeWithCharLimit (text, (int) (numChars + 1));
        return dest;
    }

    static CharPointerType createFromFixedLength (const juce_wchar* const src, const size_t numChars)
    {
        CharPointerType dest (createUninitialised (numChars));
        copyChars (dest, CharPointerType (src), (int) numChars);
        return dest;
    }

    static const CharPointerType createFromFixedLength (const char* const src, const size_t numChars)
    {
        const CharPointerType dest (createUninitialised (numChars));
        CharPointerType (dest).writeWithCharLimit (CharPointer_UTF8 (src), (int) (numChars + 1));
        return dest;
    }

    static inline const CharPointerType getEmpty() throw()
    {
        return CharPointerType (&(empty.text[0]));
    }

    //==============================================================================
    static void retain (const CharPointerType& text) throw()
    {
        ++(bufferFromText (text)->refCount);
    }

    static inline void release (StringHolder* const b) throw()
    {
        if (--(b->refCount) == -1 && b != &empty)
            delete[] reinterpret_cast <char*> (b);
    }

    static void release (const CharPointerType& text) throw()
    {
        release (bufferFromText (text));
    }

    //==============================================================================
    static CharPointerType makeUnique (const CharPointerType& text)
    {
        StringHolder* const b = bufferFromText (text);

        if (b->refCount.get() <= 0)
            return text;

        CharPointerType newText (createFromFixedLength (text, b->allocatedNumChars));
        release (b);

        return newText;
    }

    static CharPointerType makeUniqueWithSize (const CharPointerType& text, size_t numChars)
    {
        StringHolder* const b = bufferFromText (text);

        if (b->refCount.get() <= 0 && b->allocatedNumChars >= numChars)
            return text;

        CharPointerType newText (createUninitialised (jmax (b->allocatedNumChars, numChars)));
        copyChars (newText, text, b->allocatedNumChars);
        release (b);

        return newText;
    }

    static size_t getAllocatedNumChars (const CharPointerType& text) throw()
    {
        return bufferFromText (text)->allocatedNumChars;
    }

    static void copyChars (CharPointerType dest, const CharPointerType& src, const size_t numChars) throw()
    {
        jassert (src.getAddress() != 0 && dest.getAddress() != 0);
        memcpy (dest.getAddress(), src.getAddress(), numChars * sizeof (juce_wchar));
        CharPointerType (dest + (int) numChars).writeNull();
    }

    //==============================================================================
    Atomic<int> refCount;
    size_t allocatedNumChars;
    juce_wchar text[1];

    static StringHolder empty;

private:
    static inline StringHolder* bufferFromText (const CharPointerType& text) throw()
    {
        // (Can't use offsetof() here because of warnings about this not being a POD)
        return reinterpret_cast <StringHolder*> (reinterpret_cast <char*> (text.getAddress())
                    - (reinterpret_cast <size_t> (reinterpret_cast <StringHolder*> (1)->text) - 1));
    }
};

StringHolder StringHolder::empty;
const String String::empty;

//==============================================================================
void String::appendFixedLength (const juce_wchar* const newText, const int numExtraChars)
{
    if (numExtraChars > 0)
    {
        const int oldLen = length();
        const int newTotalLen = oldLen + numExtraChars;

        text = StringHolder::makeUniqueWithSize (text, newTotalLen);
        StringHolder::copyChars (text + oldLen, CharPointer_UTF32 (newText), numExtraChars);
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
    StringHolder::retain (other.text);
    StringHolder::release (text.atomicSwap (other.text));
    return *this;
}

inline String::Preallocation::Preallocation (const size_t numChars_) : numChars (numChars_) {}

String::String (const Preallocation& preallocationSize)
    : text (StringHolder::createUninitialised (preallocationSize.numChars))
{
}

String::String (const String& stringToCopy, const size_t charsToAllocate)
    : text (0)
{
    const size_t otherSize = StringHolder::getAllocatedNumChars (stringToCopy.text);
    text = StringHolder::createUninitialised (jmax (charsToAllocate, otherSize));
    StringHolder::copyChars (text, stringToCopy.text, otherSize);
}

String::String (const char* const t)
    : text (StringHolder::createFromCharPointer (CharPointer_ASCII (t)))
{
    /*  If you get an assertion here, then you're trying to create a string from 8-bit data
        that contains values greater than 127. These can NOT be correctly converted to unicode
        because there's no way for the String class to know what encoding was used to
        create them. The source data could be UTF-8, ASCII or one of many local code-pages.

        To get around this problem, you must be more explicit when you pass an ambiguous 8-bit
        string to the String class - so for example if your source data is actually UTF-8,
        you'd call String (CharPointer_UTF8 ("my utf8 string..")), and it would be able to
        correctly convert the multi-byte characters to unicode. It's *highly* recommended that
        you use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent these strings in a way that isn't dependent on
        the compiler, source code editor and platform.
    */
    jassert (CharPointer_ASCII::isValidString (t, std::numeric_limits<int>::max()));
}

String::String (const char* const t, const size_t maxChars)
    : text (StringHolder::createFromCharPointer (CharPointer_ASCII (t), maxChars))
{
    /*  If you get an assertion here, then you're trying to create a string from 8-bit data
        that contains values greater than 127. These can NOT be correctly converted to unicode
        because there's no way for the String class to know what encoding was used to
        create them. The source data could be UTF-8, ASCII or one of many local code-pages.

        To get around this problem, you must be more explicit when you pass an ambiguous 8-bit
        string to the String class - so for example if your source data is actually UTF-8,
        you'd call String (CharPointer_UTF8 ("my utf8 string..")), and it would be able to
        correctly convert the multi-byte characters to unicode. It's *highly* recommended that
        you use UTF-8 with escape characters in your source code to represent extended characters,
        because there's no other way to represent these strings in a way that isn't dependent on
        the compiler, source code editor and platform.
    */
    jassert (CharPointer_ASCII::isValidString (t, (int) maxChars));
}

String::String (const juce_wchar* const t)
    : text (StringHolder::createFromCharPointer (CharPointer_UTF32 (t)))
{
}

String::String (const juce_wchar* const t, const size_t maxChars)
    : text (StringHolder::createFromCharPointer (CharPointer_UTF32 (t), maxChars))
{
}

String::String (const CharPointer_UTF8& t)
    : text (StringHolder::createFromCharPointer (t))
{
}

String::String (const CharPointer_UTF16& t)
    : text (StringHolder::createFromCharPointer (t))
{
}

String::String (const CharPointer_UTF32& t)
    : text (StringHolder::createFromCharPointer (t))
{
}

String::String (const CharPointer_UTF32& t, const size_t maxChars)
    : text (StringHolder::createFromCharPointer (t, maxChars))
{
}

String::String (const CharPointer_ASCII& t)
    : text (StringHolder::createFromCharPointer (t))
{
}

#if JUCE_WINDOWS
String::String (const wchar_t* const t)
    : text (StringHolder::createFromCharPointer (CharPointer_UTF16 (t)))
{
}

String::String (const wchar_t* const t, size_t maxChars)
    : text (StringHolder::createFromCharPointer (CharPointer_UTF16 (t), maxChars))
{
}
#endif

const String String::charToString (const juce_wchar character)
{
    String result (Preallocation (1));
    result.text[0] = character;
    result.text[1] = 0;
    return result;
}

//==============================================================================
namespace NumberToStringConverters
{
    // pass in a pointer to the END of a buffer..
    juce_wchar* numberToString (juce_wchar* t, const int64 n) throw()
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

    juce_wchar* numberToString (juce_wchar* t, uint64 v) throw()
    {
        *--t = 0;

        do
        {
            *--t = (juce_wchar) ('0' + (int) (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    juce_wchar* numberToString (juce_wchar* t, const int n) throw()
    {
        if (n == (int) 0x80000000) // (would cause an overflow)
            return numberToString (t, (int64) n);

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

    juce_wchar* numberToString (juce_wchar* t, unsigned int v) throw()
    {
        *--t = 0;

        do
        {
            *--t = (juce_wchar) ('0' + (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    juce_wchar getDecimalPoint()
    {
      #if JUCE_VC7_OR_EARLIER
        static juce_wchar dp = std::_USE (std::locale(), std::numpunct <wchar_t>).decimal_point();
      #else
        static juce_wchar dp = std::use_facet <std::numpunct <wchar_t> > (std::locale()).decimal_point();
      #endif
        return dp;
    }

    char* doubleToString (char* buffer, int numChars, double n, int numDecPlaces, size_t& len) throw()
    {
        if (numDecPlaces > 0 && n > -1.0e20 && n < 1.0e20)
        {
            char* const end = buffer + numChars;
            char* t = end;
            int64 v = (int64) (pow (10.0, numDecPlaces) * std::abs (n) + 0.5);
            *--t = (char) 0;

            while (numDecPlaces >= 0 || v > 0)
            {
                if (numDecPlaces == 0)
                    *--t = (char) getDecimalPoint();

                *--t = (char) ('0' + (v % 10));

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
            len = sprintf (buffer, "%.9g", n);
            return buffer;
        }
    }

    template <typename IntegerType>
    const String::CharPointerType createFromInteger (const IntegerType number)
    {
        juce_wchar buffer [32];
        juce_wchar* const end = buffer + numElementsInArray (buffer);
        juce_wchar* const start = numberToString (end, number);

        return StringHolder::createFromFixedLength (start, end - start - 1);
    }

    const String::CharPointerType createFromDouble (const double number, const int numberOfDecimalPlaces)
    {
        char buffer [48];
        size_t len;
        char* const start = doubleToString (buffer, numElementsInArray (buffer), (double) number, numberOfDecimalPlaces, len);
        return StringHolder::createFromFixedLength (start, len);
    }
}

//==============================================================================
String::String (const int number)
    : text (NumberToStringConverters::createFromInteger (number))
{
}

String::String (const unsigned int number)
    : text (NumberToStringConverters::createFromInteger (number))
{
}

String::String (const short number)
    : text (NumberToStringConverters::createFromInteger ((int) number))
{
}

String::String (const unsigned short number)
    : text (NumberToStringConverters::createFromInteger ((unsigned int) number))
{
}

String::String (const int64 number)
    : text (NumberToStringConverters::createFromInteger (number))
{
}

String::String (const uint64 number)
    : text (NumberToStringConverters::createFromInteger (number))
{
}

String::String (const float number, const int numberOfDecimalPlaces)
    : text (NumberToStringConverters::createFromDouble ((double) number, numberOfDecimalPlaces))
{
}

String::String (const double number, const int numberOfDecimalPlaces)
    : text (NumberToStringConverters::createFromDouble (number, numberOfDecimalPlaces))
{
}

//==============================================================================
int String::length() const throw()
{
    return (int) text.length();
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
JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) == 0;
}

JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const char* string2) throw()
{
    return string1.compare (string2) == 0;
}

JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const juce_wchar* string2) throw()
{
    return string1.compare (string2) == 0;
}

JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const CharPointer_UTF8& string2) throw()
{
    return string1.getCharPointer().compare (string2) == 0;
}

JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const CharPointer_UTF16& string2) throw()
{
    return string1.getCharPointer().compare (string2) == 0;
}

JUCE_API bool JUCE_CALLTYPE operator== (const String& string1, const CharPointer_UTF32& string2) throw()
{
    return string1.getCharPointer().compare (string2) == 0;
}

JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) != 0;
}

JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const char* string2) throw()
{
    return string1.compare (string2) != 0;
}

JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const juce_wchar* string2) throw()
{
    return string1.compare (string2) != 0;
}

JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const CharPointer_UTF8& string2) throw()
{
    return string1.getCharPointer().compare (string2) != 0;
}

JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const CharPointer_UTF16& string2) throw()
{
    return string1.getCharPointer().compare (string2) != 0;
}

JUCE_API bool JUCE_CALLTYPE operator!= (const String& string1, const CharPointer_UTF32& string2) throw()
{
    return string1.getCharPointer().compare (string2) != 0;
}

JUCE_API bool JUCE_CALLTYPE operator>  (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) > 0;
}

JUCE_API bool JUCE_CALLTYPE operator<  (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) < 0;
}

JUCE_API bool JUCE_CALLTYPE operator>= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) >= 0;
}

JUCE_API bool JUCE_CALLTYPE operator<= (const String& string1, const String& string2) throw()
{
    return string1.compare (string2) <= 0;
}

bool String::equalsIgnoreCase (const juce_wchar* t) const throw()
{
    return t != 0 ? text.compareIgnoreCase (CharPointer_UTF32 (t)) == 0
                  : isEmpty();
}

bool String::equalsIgnoreCase (const char* t) const throw()
{
    return t != 0 ? text.compareIgnoreCase (CharPointer_UTF8 (t)) == 0
                  : isEmpty();
}

bool String::equalsIgnoreCase (const String& other) const throw()
{
    return text == other.text
            || text.compareIgnoreCase (other.text) == 0;
}

int String::compare (const String& other) const throw()
{
    return (text == other.text) ? 0 : text.compare (other.text);
}

int String::compare (const char* other) const throw()
{
    return text.compare (CharPointer_UTF8 (other));
}

int String::compare (const juce_wchar* other) const throw()
{
    return text.compare (CharPointer_UTF32 (other));
}

int String::compareIgnoreCase (const String& other) const throw()
{
    return (text == other.text) ? 0 : text.compareIgnoreCase (other.text);
}

int String::compareLexicographically (const String& other) const throw()
{
    CharPointerType s1 (text);

    while (! (s1.isEmpty() || s1.isLetterOrDigit()))
        ++s1;

    CharPointerType s2 (other.text);

    while (! (s2.isEmpty() || s2.isLetterOrDigit()))
        ++s2;

    return s1.compareIgnoreCase (s2);
}

//==============================================================================
void String::append (const String& textToAppend, size_t maxCharsToTake)
{
    appendCharPointer (textToAppend.text, maxCharsToTake);
}

String& String::operator+= (const juce_wchar* const t)
{
    appendCharPointer (CharPointer_UTF32 (t));

    return *this;
}

String& String::operator+= (const String& other)
{
    if (isEmpty())
        return operator= (other);

    appendCharPointer (other.text);
    return *this;
}

String& String::operator+= (const char ch)
{
    return operator+= ((juce_wchar) ch);
}

String& String::operator+= (const juce_wchar ch)
{
    const juce_wchar asString[] = { ch, 0 };
    return operator+= (static_cast <const juce_wchar*> (asString));
}

#if JUCE_WINDOWS
String& String::operator+= (const wchar_t ch)
{
    return operator+= ((juce_wchar) ch);
}

String& String::operator+= (const wchar_t* t)
{
    return operator+= (String (t));
}
#endif

String& String::operator+= (const int number)
{
    juce_wchar buffer [16];
    juce_wchar* const end = buffer + numElementsInArray (buffer);
    juce_wchar* const start = NumberToStringConverters::numberToString (end, number);
    appendFixedLength (start, (int) (end - start));
    return *this;
}

//==============================================================================
JUCE_API const String JUCE_CALLTYPE operator+ (const char* const string1, const String& string2)
{
    String s (string1);
    return s += string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (const juce_wchar* const string1, const String& string2)
{
    String s (string1);
    return s += string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (const char string1, const String& string2)
{
    return String::charToString (string1) + string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (const juce_wchar string1, const String& string2)
{
    return String::charToString (string1) + string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (String string1, const String& string2)
{
    return string1 += string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (String string1, const char* const string2)
{
    return string1 += string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (String string1, const juce_wchar* const string2)
{
    return string1 += string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (String string1, const char string2)
{
    return string1 += string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (String string1, const juce_wchar string2)
{
    return string1 += string2;
}

#if JUCE_WINDOWS
JUCE_API const String JUCE_CALLTYPE operator+ (String string1, wchar_t string2)
{
    return string1 += string2;
}

JUCE_API const String JUCE_CALLTYPE operator+ (String string1, const wchar_t* string2)
{
    string1.appendCharPointer (CharPointer_UTF16 (string2));
    return string1;
}

JUCE_API const String JUCE_CALLTYPE operator+ (const wchar_t* string1, const String& string2)
{
    String s (string1);
    return s += string2;
}
#endif

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const char characterToAppend)
{
    return string1 += characterToAppend;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const juce_wchar characterToAppend)
{
    return string1 += characterToAppend;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const char* const string2)
{
    return string1 += string2;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const juce_wchar* const string2)
{
    return string1 += string2;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const String& string2)
{
    return string1 += string2;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const short number)
{
    return string1 += (int) number;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const int number)
{
    return string1 += number;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const long number)
{
    return string1 += (int) number;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const float number)
{
    return string1 += String (number);
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const double number)
{
    return string1 += String (number);
}

JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const String& text)
{
    // (This avoids using toUTF8() to prevent the memory bloat that it would leave behind
    // if lots of large, persistent strings were to be written to streams).
    const int numBytes = text.getNumBytesAsUTF8();
    HeapBlock<char> temp (numBytes + 1);
    text.copyToUTF8 (temp, numBytes + 1);
    stream.write (temp, numBytes);
    return stream;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const NewLine&)
{
    return string1 += NewLine::getDefault();
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
    return t.isEmpty() ? 0 : text.indexOf (t.text);
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

    CharPointerType t (text);
    int i = jmax (0, startIndex);
    t += i;

    while (! t.isEmpty())
    {
        if (charactersToLookFor.text.indexOf (*t, ignoreCase) >= 0)
            return i;

        ++i;
        ++t;
    }

    return -1;
}

int String::indexOf (const int startIndex, const String& other) const throw()
{
    if (startIndex > 0 && startIndex >= length())
        return -1;

    int i = CharPointerType (text + jmax (0, startIndex)).indexOf (other.text);
    return i >= 0 ? i + startIndex : -1;
}

int String::indexOfIgnoreCase (const String& other) const throw()
{
    if (other.isEmpty())
        return 0;

    const int len = other.length();
    const int end = length() - len;

    for (int i = 0; i <= end; ++i)
        if (CharPointerType (text + i).compareIgnoreCaseUpTo (other.text, len) == 0)
            return i;

    return -1;
}

int String::indexOfIgnoreCase (const int startIndex, const String& other) const throw()
{
    if (other.isNotEmpty())
    {
        const int len = other.length();
        const int end = length() - len;

        for (int i = jmax (0, startIndex); i <= end; ++i)
            if (CharPointerType (text + i).compareIgnoreCaseUpTo (other.text, len) == 0)
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
            CharPointerType n (text + i);

            while (i >= 0)
            {
                if (n.compareUpTo (other.text, len) == 0)
                    return i;

                --n;
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
            CharPointerType n (text + i);

            while (i >= 0)
            {
                if (n.compareIgnoreCaseUpTo (other.text, len) == 0)
                    return i;

                --n;
                --i;
            }
        }
    }

    return -1;
}

int String::lastIndexOfAnyOf (const String& charactersToLookFor, const bool ignoreCase) const throw()
{
    for (int i = length(); --i >= 0;)
        if (charactersToLookFor.text.indexOf (text[i], ignoreCase) >= 0)
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
        CharPointerType t (text);
        const int wordLen = word.length();
        const int end = (int) t.length() - wordLen;

        for (int i = 0; i <= end; ++i)
        {
            if (t.compareUpTo (word.text, wordLen) == 0
                  && (i == 0 || ! (t - 1).isLetterOrDigit())
                  && ! (t + wordLen).isLetterOrDigit())
                return i;

            ++t;
        }
    }

    return -1;
}

int String::indexOfWholeWordIgnoreCase (const String& word) const throw()
{
    if (word.isNotEmpty())
    {
        CharPointerType t (text);
        const int wordLen = word.length();
        const int end = (int) t.length() - wordLen;

        for (int i = 0; i <= end; ++i)
        {
            if (t.compareIgnoreCaseUpTo (word.text, wordLen) == 0
                  && (i == 0 || ! (t + -1).isLetterOrDigit())
                  && ! (t + wordLen).isLetterOrDigit())
                return i;

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
namespace WildCardHelpers
{
    int indexOfMatch (const String::CharPointerType& wildcard,
                      String::CharPointerType test,
                      const bool ignoreCase) throw()
    {
        int start = 0;

        while (! test.isEmpty())
        {
            String::CharPointerType t (test);
            String::CharPointerType w (wildcard);

            for (;;)
            {
                const juce_wchar wc = *w;
                const juce_wchar tc = *t;

                if (wc == tc
                     || (ignoreCase && w.toLowerCase() == t.toLowerCase())
                     || (wc == '?' && tc != 0))
                {
                    if (wc == 0)
                        return start;

                    ++t;
                    ++w;
                }
                else
                {
                    if (wc == '*' && (w[1] == 0 || indexOfMatch (w + 1, t, ignoreCase) >= 0))
                        return start;

                    break;
                }
            }

            ++start;
            ++test;
        }

        return -1;
    }
}

bool String::matchesWildcard (const String& wildcard, const bool ignoreCase) const throw()
{
    CharPointerType w (wildcard.text);
    CharPointerType t (text);

    for (;;)
    {
        const juce_wchar wc = *w;
        const juce_wchar tc = *t;

        if (wc == tc
             || (ignoreCase && w.toLowerCase() == t.toLowerCase())
             || (wc == '?' && tc != 0))
        {
            if (wc == 0)
                return true;

            ++w;
            ++t;
        }
        else
        {
            return wc == '*' && (w[1] == 0 || WildCardHelpers::indexOfMatch (w + 1, t, ignoreCase) >= 0);
        }
    }
}

//==============================================================================
const String String::repeatedString (const String& stringToRepeat, int numberOfTimesToRepeat)
{
    if (numberOfTimesToRepeat <= 0)
        return String::empty;

    const int len = stringToRepeat.length();
    String result (Preallocation (len * numberOfTimesToRepeat + 1));
    CharPointerType n (result.text);

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

    String result (Preallocation (minimumLength + 1));
    CharPointerType n (result.text);

    minimumLength -= len;
    while (--minimumLength >= 0)
        n.write (padCharacter);

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
    CharPointerType n (result.text + len);

    minimumLength -= len;
    while (--minimumLength >= 0)
        n.write (padCharacter);

    n.writeNull();
    return result;
}

//==============================================================================
const String String::replaceSection (int index, int numCharsToReplace, const String& stringToInsert) const
{
    if (index < 0)
    {
        // a negative index to replace from?
        jassertfalse;
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
            jassertfalse;
        }

        numCharsToReplace = len - index;
    }

    const int newStringLen = stringToInsert.length();
    const int newTotalLen = len + newStringLen - numCharsToReplace;

    if (newTotalLen <= 0)
        return String::empty;

    String result (Preallocation ((size_t) newTotalLen));

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
    CharPointerType t (result.text + index);

    while (! t.isEmpty())
    {
        if (*t == charToReplace)
            t.replaceChar (charToInsert);

        ++t;
    }

    return result;
}

const String String::replaceCharacters (const String& charactersToReplace,
                                        const String& charactersToInsertInstead) const
{
    String result (*this, size_t());
    CharPointerType t (result.text);
    const int len2 = charactersToInsertInstead.length();

    // the two strings passed in are supposed to be the same length!
    jassert (len2 == charactersToReplace.length());

    while (! t.isEmpty())
    {
        const int index = charactersToReplace.indexOfChar (*t);

        if (isPositiveAndBelow (index, len2))
            t.replaceChar (charactersToInsertInstead [index]);

        ++t;
    }

    return result;
}

//==============================================================================
bool String::startsWith (const String& other) const throw()
{
    return text.compareUpTo (other.text, other.length()) == 0;
}

bool String::startsWithIgnoreCase (const String& other) const throw()
{
    return text.compareIgnoreCaseUpTo (other.text, other.length()) == 0;
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
            && CharPointerType (text + thisLen - otherLen).compare (other.text) == 0;
}

bool String::endsWithIgnoreCase (const String& other) const throw()
{
    const int thisLen = length();
    const int otherLen = other.length();

    return thisLen >= otherLen
            && CharPointerType (text + thisLen - otherLen).compareIgnoreCase (other.text) == 0;
}

//==============================================================================
const String String::toUpperCase() const
{
    String result (Preallocation (this->length()));

    CharPointerType dest (result.text);
    CharPointerType src (text);

    for (;;)
    {
        const juce_wchar c = src.toUpperCase();
        dest.write (c);

        if (c == 0)
            break;

        ++src;
    }

    return result;
}

const String String::toLowerCase() const
{
    String result (Preallocation (this->length()));

    CharPointerType dest (result.text);
    CharPointerType src (text);

    for (;;)
    {
        const juce_wchar c = src.toLowerCase();
        dest.write (c);

        if (c == 0)
            break;

        ++src;
    }

    return result;
}

//==============================================================================
juce_wchar& String::operator[] (const int index)
{
    jassert (isPositiveAndNotGreaterThan (index, length()));
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

    while ((text + start).isWhitespace())
        ++start;

    const int len = length();
    int end = len - 1;

    while ((end >= start) && (text + end).isWhitespace())
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

    CharPointerType t (text);

    while (t.isWhitespace())
        ++t;

    if (t == text)
        return *this;

    return String (t.getAddress());
}

const String String::trimEnd() const
{
    if (isEmpty())
        return empty;

    CharPointerType endT (text);
    endT = endT.findTerminatingNull() - 1;

    while ((endT.getAddress() >= text) && endT.isWhitespace())
        --endT;

    return String (text, 1 + (int) (endT.getAddress() - text));
}

const String String::trimCharactersAtStart (const String& charactersToTrim) const
{
    CharPointerType t (text);

    while (charactersToTrim.containsChar (*t))
        ++t;

    return t == text ? *this : String (t);
}

const String String::trimCharactersAtEnd (const String& charactersToTrim) const
{
    if (isEmpty())
        return empty;

    const int len = length();
    const juce_wchar* endT = text + (len - 1);
    int numToRemove = 0;

    while (numToRemove < len && charactersToTrim.containsChar (*endT))
    {
        ++numToRemove;
        --endT;
    }

    return numToRemove > 0 ? String (text, len - numToRemove) : *this;
}

//==============================================================================
const String String::retainCharacters (const String& charactersToRetain) const
{
    if (isEmpty())
        return empty;

    String result (Preallocation (StringHolder::getAllocatedNumChars (text)));
    CharPointerType dst (result.text);
    CharPointerType src (text);

    for (;;)
    {
        const juce_wchar c = src.getAndAdvance();

        if (c == 0)
            break;

        if (charactersToRetain.containsChar (c))
            dst.write (c);
    }

    dst.writeNull();
    return result;
}

const String String::removeCharacters (const String& charactersToRemove) const
{
    if (isEmpty())
        return empty;

    String result (Preallocation (StringHolder::getAllocatedNumChars (text)));
    CharPointerType dst (result.text);
    CharPointerType src (text);

    for (;;)
    {
        const juce_wchar c = src.getAndAdvance();

        if (c == 0)
            break;

        if (! charactersToRemove.containsChar (c))
            dst.write (c);
    }

    dst.writeNull();
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
    CharPointerType t (text);

    while (! t.isEmpty())
        if (! chars.containsChar (t.getAndAdvance()))
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
    CharPointerType t (text);

    while (! t.isEmpty())
    {
        if (! t.isWhitespace())
            return true;

        ++t;
    }

    return false;
}

const String String::formatted (const juce_wchar* const pf, ... )
{
    jassert (pf != 0);

    va_list args;
    va_start (args, pf);

    size_t bufferSize = 256;
    String result (Preallocation ((size_t) bufferSize));
    result.text[0] = 0;

    for (;;)
    {
      #if JUCE_LINUX && JUCE_64BIT
        va_list tempArgs;
        va_copy (tempArgs, args);
        const int num = (int) vswprintf (result.text.getAddress(), bufferSize - 1, pf, tempArgs);
        va_end (tempArgs);
      #elif JUCE_WINDOWS
        HeapBlock <wchar_t> temp (bufferSize);
        const int num = (int) _vsnwprintf (temp.getData(), bufferSize - 1, String (pf).toUTF16(), args);
        if (num > 0)
            CharPointerType (result.text).writeAll (CharPointer_UTF16 (temp.getData()));
      #elif JUCE_ANDROID
        HeapBlock <char> temp (bufferSize);
        const int num = (int) vsnprintf (temp.getData(), bufferSize - 1, String (pf).toUTF8(), args);
        if (num > 0)
            CharPointerType (result.text).writeAll (CharPointer_UTF8 (temp.getData()));
      #else
        const int num = (int) vswprintf (result.text.getAddress(), bufferSize - 1, pf, args);
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
    return text.getIntValue32();
}

int String::getTrailingIntValue() const throw()
{
    int n = 0;
    int mult = 1;
    CharPointerType t (text.findTerminatingNull());

    while ((--t).getAddress() >= text)
    {
        if (! t.isDigit())
        {
            if (*t == '-')
                n = -n;

            break;
        }

        n += mult * (*t - '0');
        mult *= 10;
    }

    return n;
}

int64 String::getLargeIntValue() const throw()
{
    return text.getIntValue64();
}

float String::getFloatValue() const throw()
{
    return (float) getDoubleValue();
}

double String::getDoubleValue() const throw()
{
    return text.getDoubleValue();
}

static const char* const hexDigits = "0123456789abcdef";

const String String::toHexString (const int number)
{
    juce_wchar buffer[32];
    juce_wchar* const end = buffer + 32;
    juce_wchar* t = end;
    *--t = 0;
    unsigned int v = (unsigned int) number;

    do
    {
        *--t = (juce_wchar) hexDigits [v & 15];
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
        *--t = (juce_wchar) hexDigits [(int) (v & 15)];
        v >>= 4;

    } while (v != 0);

    return String (t, (int) (((char*) end) - (char*) t));
}

const String String::toHexString (const short number)
{
    return toHexString ((int) (unsigned short) number);
}

const String String::toHexString (const unsigned char* data, const int size, const int groupSize)
{
    if (size <= 0)
        return empty;

    int numChars = (size * 2) + 2;
    if (groupSize > 0)
        numChars += size / groupSize;

    String s (Preallocation ((size_t) numChars));

    CharPointerType dest (s.text);

    for (int i = 0; i < size; ++i)
    {
        dest.write ((juce_wchar) hexDigits [(*data) >> 4]);
        dest.write ((juce_wchar) hexDigits [(*data) & 0xf]);
        ++data;

        if (groupSize > 0 && (i % groupSize) == (groupSize - 1) && i < (size - 1))
            dest.write ((juce_wchar) ' ');
    }

    dest.writeNull();
    return s;
}

int String::getHexValue32() const throw()
{
    int result = 0;
    CharPointerType t (text);

    while (! t.isEmpty())
    {
        const int hexValue = CharacterFunctions::getHexDigitValue (t.getAndAdvance());

        if (hexValue >= 0)
            result = (result << 4) | hexValue;
    }

    return result;
}

int64 String::getHexValue64() const throw()
{
    int64 result = 0;
    CharPointerType t (text);

    while (! t.isEmpty())
    {
        const int hexValue = CharacterFunctions::getHexDigitValue (t.getAndAdvance());

        if (hexValue >= 0)
            result = (result << 4) | hexValue;
    }

    return result;
}

//==============================================================================
const String String::createStringFromData (const void* const data_, const int size)
{
    const uint8* const data = static_cast <const uint8*> (data_);

    if (size <= 0 || data == 0)
    {
        return empty;
    }
    else if (size == 1)
    {
        return charToString ((char) data[0]);
    }
    else if ((data[0] == (uint8) CharPointer_UTF16::byteOrderMarkBE1 && data[1] == (uint8) CharPointer_UTF16::byteOrderMarkBE2)
          || (data[0] == (uint8) CharPointer_UTF16::byteOrderMarkLE1 && data[1] == (uint8) CharPointer_UTF16::byteOrderMarkLE1))
    {
        const bool bigEndian = (data[0] == (uint8) CharPointer_UTF16::byteOrderMarkBE1);
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
        if (size >= 3
              && data[0] == (uint8) CharPointer_UTF8::byteOrderMark1
              && data[1] == (uint8) CharPointer_UTF8::byteOrderMark2
              && data[2] == (uint8) CharPointer_UTF8::byteOrderMark3)
            return String::fromUTF8 ((const char*) data + 3, size - 3);

        return String::fromUTF8 ((const char*) data, size);
    }
}

//==============================================================================
void* String::createSpaceAtEndOfBuffer (const size_t numExtraBytes) const
{
    const int currentLen = length() + 1;

    String& mutableThis = const_cast <String&> (*this);
    mutableThis.preallocateStorage (currentLen + 1 + numExtraBytes / sizeof (juce_wchar));

    return (mutableThis.text + currentLen).getAddress();
}

const CharPointer_UTF8 String::toUTF8() const
{
    if (isEmpty())
        return CharPointer_UTF8 (reinterpret_cast <const CharPointer_UTF8::CharType*> (text.getAddress()));

    const size_t extraBytesNeeded = CharPointer_UTF8::getBytesRequiredFor (text);
    CharPointer_UTF8 extraSpace (static_cast <CharPointer_UTF8::CharType*> (createSpaceAtEndOfBuffer (extraBytesNeeded)));

   #if JUCE_DEBUG // (This just avoids spurious warnings from valgrind about the uninitialised bytes at the end of the buffer..)
    *(juce_wchar*) (addBytesToPointer (extraSpace.getAddress(), (extraBytesNeeded & ~(sizeof (juce_wchar) - 1)))) = 0;
   #endif

    CharPointer_UTF8 (extraSpace).writeAll (text);
    return extraSpace;
}

CharPointer_UTF16 String::toUTF16() const
{
    if (isEmpty())
        return CharPointer_UTF16 (reinterpret_cast <const CharPointer_UTF16::CharType*> (text.getAddress()));

    const size_t extraBytesNeeded = CharPointer_UTF16::getBytesRequiredFor (text);
    CharPointer_UTF16 extraSpace (static_cast <CharPointer_UTF16::CharType*> (createSpaceAtEndOfBuffer (extraBytesNeeded)));

   #if JUCE_DEBUG // (This just avoids spurious warnings from valgrind about the uninitialised bytes at the end of the buffer..)
    *(juce_wchar*) (addBytesToPointer (extraSpace.getAddress(), (extraBytesNeeded & ~(sizeof (juce_wchar) - 1)))) = 0;
   #endif

    CharPointer_UTF16 (extraSpace).writeAll (text);
    return extraSpace;
}

int String::copyToUTF8 (CharPointer_UTF8::CharType* const buffer, const int maxBufferSizeBytes) const throw()
{
    jassert (maxBufferSizeBytes >= 0); // keep this value positive, or no characters will be copied!

    if (buffer == 0)
        return (int) CharPointer_UTF8::getBytesRequiredFor (text);

    return CharPointer_UTF8 (buffer).writeWithDestByteLimit (text, maxBufferSizeBytes);
}

int String::copyToUTF16 (CharPointer_UTF16::CharType* const buffer, int maxBufferSizeBytes) const throw()
{
    jassert (maxBufferSizeBytes >= 0); // keep this value positive, or no characters will be copied!

    if (buffer == 0)
        return (int) CharPointer_UTF16::getBytesRequiredFor (text);

    return CharPointer_UTF16 (buffer).writeWithDestByteLimit (text, maxBufferSizeBytes);
}

int String::getNumBytesAsUTF8() const throw()
{
    return (int) CharPointer_UTF8::getBytesRequiredFor (text);
}

const String String::fromUTF8 (const char* const buffer, int bufferSizeBytes)
{
    if (buffer == 0)
        return empty;

    const int len = (int) (bufferSizeBytes >= 0 ? CharPointer_UTF8 (buffer).lengthUpTo (bufferSizeBytes)
                                                : CharPointer_UTF8 (buffer).length());

    String result (Preallocation (len + 1));
    CharPointerType (result.text).writeWithCharLimit (CharPointer_UTF8 (buffer), len + 1);
    return result;
}

//==============================================================================
const char* String::toCString() const
{
   #if JUCE_NATIVE_WCHAR_IS_NOT_UTF32
    return toUTF8();
   #else
    if (isEmpty())
        return reinterpret_cast <const char*> (text.getAddress());

    const int len = getNumBytesAsCString();
    char* const extraSpace = static_cast <char*> (createSpaceAtEndOfBuffer (len + 1));
    wcstombs (extraSpace, text, len);
    extraSpace [len] = 0;
    return extraSpace;
   #endif
}

int String::getNumBytesAsCString() const throw()
{
   #if JUCE_NATIVE_WCHAR_IS_NOT_UTF32
    return getNumBytesAsUTF8();
   #else
    return (int) wcstombs (0, text, 0);
   #endif
}

int String::copyToCString (char* destBuffer, const int maxBufferSizeBytes) const throw()
{
   #if JUCE_NATIVE_WCHAR_IS_NOT_UTF32
    return copyToUTF8 (destBuffer, maxBufferSizeBytes);
   #else
    const int numBytes = (int) wcstombs (destBuffer, text, maxBufferSizeBytes);

    if (destBuffer != 0 && numBytes >= 0)
        destBuffer [numBytes] = 0;

    return numBytes;
   #endif
}

#if JUCE_MSVC
  #pragma warning (pop)
#endif

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
        CharPointerType (result.text + nextIndex).writeAll (s.text);
        nextIndex += len;
    }
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

#include "../utilities/juce_UnitTest.h"
#include "../maths/juce_Random.h"
#include "juce_StringArray.h"

class StringTests  : public UnitTest
{
public:
    StringTests() : UnitTest ("String class") {}

    template <class CharPointerType>
    struct TestUTFConversion
    {
        static void test (UnitTest& test)
        {
            String s (createRandomWideCharString());

            typename CharPointerType::CharType buffer [300];

            memset (buffer, 0xff, sizeof (buffer));
            CharPointerType (buffer).writeAll (s.toUTF32());
            test.expectEquals (String (CharPointerType (buffer)), s);

            memset (buffer, 0xff, sizeof (buffer));
            CharPointerType (buffer).writeAll (s.toUTF16());
            test.expectEquals (String (CharPointerType (buffer)), s);

            memset (buffer, 0xff, sizeof (buffer));
            CharPointerType (buffer).writeAll (s.toUTF8());
            test.expectEquals (String (CharPointerType (buffer)), s);
        }
    };

    static const String createRandomWideCharString()
    {
        juce_wchar buffer [50];
        zerostruct (buffer);

        for (int i = 0; i < numElementsInArray (buffer) - 1; ++i)
        {
            if (Random::getSystemRandom().nextBool())
            {
                do
                {
                    buffer[i] = (juce_wchar) (1 + Random::getSystemRandom().nextInt (0x10ffff - 1));
                }
                while (buffer[i] >= 0xd800 && buffer[i] <= 0xdfff); // (these code-points are illegal in UTF-16)
            }
            else
                buffer[i] = (juce_wchar) (1 + Random::getSystemRandom().nextInt (0xff));
        }

        return buffer;
    }

    void runTest()
    {
        {
            beginTest ("Basics");

            expect (String().length() == 0);
            expect (String() == String::empty);
            String s1, s2 ("abcd");
            expect (s1.isEmpty() && ! s1.isNotEmpty());
            expect (s2.isNotEmpty() && ! s2.isEmpty());
            expect (s2.length() == 4);
            s1 = "abcd";
            expect (s2 == s1 && s1 == s2);
            expect (s1 == "abcd" && s1 == L"abcd");
            expect (String ("abcd") == String (L"abcd"));
            expect (String ("abcdefg", 4) == L"abcd");
            expect (String ("abcdefg", 4) == String (L"abcdefg", 4));
            expect (String::charToString ('x') == "x");
            expect (String::charToString (0) == String::empty);
            expect (s2 + "e" == "abcde" && s2 + 'e' == "abcde");
            expect (s2 + L'e' == "abcde" && s2 + L"e" == "abcde");
            expect (s1.equalsIgnoreCase ("abcD") && s1 < "abce" && s1 > "abbb");
            expect (s1.startsWith ("ab") && s1.startsWith ("abcd") && ! s1.startsWith ("abcde"));
            expect (s1.startsWithIgnoreCase ("aB") && s1.endsWithIgnoreCase ("CD"));
            expect (s1.endsWith ("bcd") && ! s1.endsWith ("aabcd"));
            expect (s1.indexOf (String::empty) == 0);
            expect (s1.indexOfIgnoreCase (String::empty) == 0);
            expect (s1.startsWith (String::empty) && s1.endsWith (String::empty) && s1.contains (String::empty));
            expect (s1.contains ("cd") && s1.contains ("ab") && s1.contains ("abcd"));
            expect (s1.containsChar ('a'));
            expect (! s1.containsChar ('x'));
            expect (! s1.containsChar (0));
            expect (String ("abc foo bar").containsWholeWord ("abc") && String ("abc foo bar").containsWholeWord ("abc"));
        }

        {
            beginTest ("Operations");

            String s ("012345678");
            expect (s.hashCode() != 0);
            expect (s.hashCode64() != 0);
            expect (s.hashCode() != (s + s).hashCode());
            expect (s.hashCode64() != (s + s).hashCode64());
            expect (s.compare (String ("012345678")) == 0);
            expect (s.compare (String ("012345679")) < 0);
            expect (s.compare (String ("012345676")) > 0);
            expect (s.substring (2, 3) == String::charToString (s[2]));
            expect (s.substring (0, 1) == String::charToString (s[0]));
            expect (s.getLastCharacter() == s [s.length() - 1]);
            expect (String::charToString (s.getLastCharacter()) == s.getLastCharacters (1));
            expect (s.substring (0, 3) == L"012");
            expect (s.substring (0, 100) == s);
            expect (s.substring (-1, 100) == s);
            expect (s.substring (3) == "345678");
            expect (s.indexOf (L"45") == 4);
            expect (String ("444445").indexOf ("45") == 4);
            expect (String ("444445").lastIndexOfChar ('4') == 4);
            expect (String ("45454545x").lastIndexOf (L"45") == 6);
            expect (String ("45454545x").lastIndexOfAnyOf ("456") == 7);
            expect (String ("45454545x").lastIndexOfAnyOf (L"456x") == 8);
            expect (String ("abABaBaBa").lastIndexOfIgnoreCase ("aB") == 6);
            expect (s.indexOfChar (L'4') == 4);
            expect (s + s == "012345678012345678");
            expect (s.startsWith (s));
            expect (s.startsWith (s.substring (0, 4)));
            expect (s.startsWith (s.dropLastCharacters (4)));
            expect (s.endsWith (s.substring (5)));
            expect (s.endsWith (s));
            expect (s.contains (s.substring (3, 6)));
            expect (s.contains (s.substring (3)));
            expect (s.startsWithChar (s[0]));
            expect (s.endsWithChar (s.getLastCharacter()));
            expect (s [s.length()] == 0);
            expect (String ("abcdEFGH").toLowerCase() == String ("abcdefgh"));
            expect (String ("abcdEFGH").toUpperCase() == String ("ABCDEFGH"));

            String s2 ("123");
            s2 << ((int) 4) << ((short) 5) << "678" << L"9" << '0';
            s2 += "xyz";
            expect (s2 == "1234567890xyz");

            beginTest ("Numeric conversions");
            expect (String::empty.getIntValue() == 0);
            expect (String::empty.getDoubleValue() == 0.0);
            expect (String::empty.getFloatValue() == 0.0f);
            expect (s.getIntValue() == 12345678);
            expect (s.getLargeIntValue() == (int64) 12345678);
            expect (s.getDoubleValue() == 12345678.0);
            expect (s.getFloatValue() == 12345678.0f);
            expect (String (-1234).getIntValue() == -1234);
            expect (String ((int64) -1234).getLargeIntValue() == -1234);
            expect (String (-1234.56).getDoubleValue() == -1234.56);
            expect (String (-1234.56f).getFloatValue() == -1234.56f);
            expect (("xyz" + s).getTrailingIntValue() == s.getIntValue());
            expect (s.getHexValue32() == 0x12345678);
            expect (s.getHexValue64() == (int64) 0x12345678);
            expect (String::toHexString (0x1234abcd).equalsIgnoreCase ("1234abcd"));
            expect (String::toHexString ((int64) 0x1234abcd).equalsIgnoreCase ("1234abcd"));
            expect (String::toHexString ((short) 0x12ab).equalsIgnoreCase ("12ab"));

            unsigned char data[] = { 1, 2, 3, 4, 0xa, 0xb, 0xc, 0xd };
            expect (String::toHexString (data, 8, 0).equalsIgnoreCase ("010203040a0b0c0d"));
            expect (String::toHexString (data, 8, 1).equalsIgnoreCase ("01 02 03 04 0a 0b 0c 0d"));
            expect (String::toHexString (data, 8, 2).equalsIgnoreCase ("0102 0304 0a0b 0c0d"));

            beginTest ("Subsections");
            String s3;
            s3 = "abcdeFGHIJ";
            expect (s3.equalsIgnoreCase ("ABCdeFGhiJ"));
            expect (s3.compareIgnoreCase (L"ABCdeFGhiJ") == 0);
            expect (s3.containsIgnoreCase (s3.substring (3)));
            expect (s3.indexOfAnyOf ("xyzf", 2, true) == 5);
            expect (s3.indexOfAnyOf (L"xyzf", 2, false) == -1);
            expect (s3.indexOfAnyOf ("xyzF", 2, false) == 5);
            expect (s3.containsAnyOf (L"zzzFs"));
            expect (s3.startsWith ("abcd"));
            expect (s3.startsWithIgnoreCase (L"abCD"));
            expect (s3.startsWith (String::empty));
            expect (s3.startsWithChar ('a'));
            expect (s3.endsWith (String ("HIJ")));
            expect (s3.endsWithIgnoreCase (L"Hij"));
            expect (s3.endsWith (String::empty));
            expect (s3.endsWithChar (L'J'));
            expect (s3.indexOf ("HIJ") == 7);
            expect (s3.indexOf (L"HIJK") == -1);
            expect (s3.indexOfIgnoreCase ("hij") == 7);
            expect (s3.indexOfIgnoreCase (L"hijk") == -1);

            String s4 (s3);
            s4.append (String ("xyz123"), 3);
            expect (s4 == s3 + "xyz");

            expect (String (1234) < String (1235));
            expect (String (1235) > String (1234));
            expect (String (1234) >= String (1234));
            expect (String (1234) <= String (1234));
            expect (String (1235) >= String (1234));
            expect (String (1234) <= String (1235));

            String s5 ("word word2 word3");
            expect (s5.containsWholeWord (String ("word2")));
            expect (s5.indexOfWholeWord ("word2") == 5);
            expect (s5.containsWholeWord (L"word"));
            expect (s5.containsWholeWord ("word3"));
            expect (s5.containsWholeWord (s5));
            expect (s5.containsWholeWordIgnoreCase (L"Word2"));
            expect (s5.indexOfWholeWordIgnoreCase ("Word2") == 5);
            expect (s5.containsWholeWordIgnoreCase (L"Word"));
            expect (s5.containsWholeWordIgnoreCase ("Word3"));
            expect (! s5.containsWholeWordIgnoreCase (L"Wordx"));
            expect (!s5.containsWholeWordIgnoreCase ("xWord2"));
            expect (s5.containsNonWhitespaceChars());
            expect (s5.containsOnly ("ordw23 "));
            expect (! String (" \n\r\t").containsNonWhitespaceChars());

            expect (s5.matchesWildcard (L"wor*", false));
            expect (s5.matchesWildcard ("wOr*", true));
            expect (s5.matchesWildcard (L"*word3", true));
            expect (s5.matchesWildcard ("*word?", true));
            expect (s5.matchesWildcard (L"Word*3", true));

            expectEquals (s5.fromFirstOccurrenceOf (String::empty, true, false), s5);
            expectEquals (s5.fromFirstOccurrenceOf ("xword2", true, false), s5.substring (100));
            expectEquals (s5.fromFirstOccurrenceOf (L"word2", true, false), s5.substring (5));
            expectEquals (s5.fromFirstOccurrenceOf ("Word2", true, true), s5.substring (5));
            expectEquals (s5.fromFirstOccurrenceOf ("word2", false, false), s5.getLastCharacters (6));
            expectEquals (s5.fromFirstOccurrenceOf (L"Word2", false, true), s5.getLastCharacters (6));

            expectEquals (s5.fromLastOccurrenceOf (String::empty, true, false), s5);
            expectEquals (s5.fromLastOccurrenceOf (L"wordx", true, false), s5);
            expectEquals (s5.fromLastOccurrenceOf ("word", true, false), s5.getLastCharacters (5));
            expectEquals (s5.fromLastOccurrenceOf (L"worD", true, true), s5.getLastCharacters (5));
            expectEquals (s5.fromLastOccurrenceOf ("word", false, false), s5.getLastCharacters (1));
            expectEquals (s5.fromLastOccurrenceOf (L"worD", false, true), s5.getLastCharacters (1));

            expect (s5.upToFirstOccurrenceOf (String::empty, true, false).isEmpty());
            expectEquals (s5.upToFirstOccurrenceOf ("word4", true, false), s5);
            expectEquals (s5.upToFirstOccurrenceOf (L"word2", true, false), s5.substring (0, 10));
            expectEquals (s5.upToFirstOccurrenceOf ("Word2", true, true), s5.substring (0, 10));
            expectEquals (s5.upToFirstOccurrenceOf (L"word2", false, false), s5.substring (0, 5));
            expectEquals (s5.upToFirstOccurrenceOf ("Word2", false, true), s5.substring (0, 5));

            expectEquals (s5.upToLastOccurrenceOf (String::empty, true, false), s5);
            expectEquals (s5.upToLastOccurrenceOf ("zword", true, false), s5);
            expectEquals (s5.upToLastOccurrenceOf ("word", true, false), s5.dropLastCharacters (1));
            expectEquals (s5.dropLastCharacters(1).upToLastOccurrenceOf ("word", true, false), s5.dropLastCharacters (1));
            expectEquals (s5.upToLastOccurrenceOf ("Word", true, true), s5.dropLastCharacters (1));
            expectEquals (s5.upToLastOccurrenceOf ("word", false, false), s5.dropLastCharacters (5));
            expectEquals (s5.upToLastOccurrenceOf ("Word", false, true), s5.dropLastCharacters (5));

            expectEquals (s5.replace ("word", L"xyz", false), String ("xyz xyz2 xyz3"));
            expect (s5.replace (L"Word", "xyz", true) == "xyz xyz2 xyz3");
            expect (s5.dropLastCharacters (1).replace ("Word", String ("xyz"), true) == L"xyz xyz2 xyz");
            expect (s5.replace ("Word", "", true) == " 2 3");
            expectEquals (s5.replace ("Word2", L"xyz", true), String ("word xyz word3"));
            expect (s5.replaceCharacter (L'w', 'x') != s5);
            expectEquals (s5.replaceCharacter ('w', L'x').replaceCharacter ('x', 'w'), s5);
            expect (s5.replaceCharacters ("wo", "xy") != s5);
            expectEquals (s5.replaceCharacters ("wo", "xy").replaceCharacters ("xy", L"wo"), s5);
            expectEquals (s5.retainCharacters ("1wordxya"), String ("wordwordword"));
            expect (s5.retainCharacters (String::empty).isEmpty());
            expect (s5.removeCharacters ("1wordxya") == " 2 3");
            expectEquals (s5.removeCharacters (String::empty), s5);
            expect (s5.initialSectionContainingOnly ("word") == L"word");
            expectEquals (s5.initialSectionNotContaining (String ("xyz ")), String ("word"));
            expect (! s5.isQuotedString());
            expect (s5.quoted().isQuotedString());
            expect (! s5.quoted().unquoted().isQuotedString());
            expect (! String ("x'").isQuotedString());
            expect (String ("'x").isQuotedString());

            String s6 (" \t xyz  \t\r\n");
            expectEquals (s6.trim(), String ("xyz"));
            expect (s6.trim().trim() == "xyz");
            expectEquals (s5.trim(), s5);
            expectEquals (s6.trimStart().trimEnd(), s6.trim());
            expectEquals (s6.trimStart().trimEnd(), s6.trimEnd().trimStart());
            expectEquals (s6.trimStart().trimStart().trimEnd().trimEnd(), s6.trimEnd().trimStart());
            expect (s6.trimStart() != s6.trimEnd());
            expectEquals (("\t\r\n " + s6 + "\t\n \r").trim(), s6.trim());
            expect (String::repeatedString ("xyz", 3) == L"xyzxyzxyz");
        }

        {
            beginTest ("UTF conversions");

            TestUTFConversion <CharPointer_UTF32>::test (*this);
            TestUTFConversion <CharPointer_UTF8>::test (*this);
            TestUTFConversion <CharPointer_UTF16>::test (*this);
        }

        {
            beginTest ("StringArray");

            StringArray s;
            for (int i = 5; --i >= 0;)
                s.add (String (i));

            expectEquals (s.joinIntoString ("-"), String ("4-3-2-1-0"));
            s.remove (2);
            expectEquals (s.joinIntoString ("--"), String ("4--3--1--0"));
            expectEquals (s.joinIntoString (String::empty), String ("4310"));
            s.clear();
            expectEquals (s.joinIntoString ("x"), String::empty);
        }
    }
};

static StringTests stringUnitTests;

#endif


END_JUCE_NAMESPACE
