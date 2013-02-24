/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4514 4996)
#endif

NewLine newLine;

#if defined (JUCE_STRINGS_ARE_UNICODE) && ! JUCE_STRINGS_ARE_UNICODE
 #error "JUCE_STRINGS_ARE_UNICODE is deprecated! All strings are now unicode by default."
#endif

#if JUCE_NATIVE_WCHAR_IS_UTF8
 typedef CharPointer_UTF8          CharPointer_wchar_t;
#elif JUCE_NATIVE_WCHAR_IS_UTF16
 typedef CharPointer_UTF16         CharPointer_wchar_t;
#else
 typedef CharPointer_UTF32         CharPointer_wchar_t;
#endif

static inline CharPointer_wchar_t castToCharPointer_wchar_t (const void* t) noexcept
{
    return CharPointer_wchar_t (static_cast <const CharPointer_wchar_t::CharType*> (t));
}

//==============================================================================
class StringHolder
{
public:
    StringHolder() noexcept
        : refCount (0x3fffffff), allocatedNumBytes (sizeof (*text))
    {
        text[0] = 0;
    }

    typedef String::CharPointerType CharPointerType;
    typedef String::CharPointerType::CharType CharType;

    //==============================================================================
    static CharPointerType createUninitialisedBytes (const size_t numBytes)
    {
        StringHolder* const s = reinterpret_cast <StringHolder*> (new char [sizeof (StringHolder) - sizeof (CharType) + numBytes]);
        s->refCount.value = 0;
        s->allocatedNumBytes = numBytes;
        return CharPointerType (s->text);
    }

    template <class CharPointer>
    static CharPointerType createFromCharPointer (const CharPointer& text)
    {
        if (text.getAddress() == nullptr || text.isEmpty())
            return getEmpty();

        CharPointer t (text);
        size_t bytesNeeded = sizeof (CharType);

        while (! t.isEmpty())
            bytesNeeded += CharPointerType::getBytesRequiredFor (t.getAndAdvance());

        const CharPointerType dest (createUninitialisedBytes (bytesNeeded));
        CharPointerType (dest).writeAll (text);
        return dest;
    }

    template <class CharPointer>
    static CharPointerType createFromCharPointer (const CharPointer& text, size_t maxChars)
    {
        if (text.getAddress() == nullptr || text.isEmpty() || maxChars == 0)
            return getEmpty();

        CharPointer end (text);
        size_t numChars = 0;
        size_t bytesNeeded = sizeof (CharType);

        while (numChars < maxChars && ! end.isEmpty())
        {
            bytesNeeded += CharPointerType::getBytesRequiredFor (end.getAndAdvance());
            ++numChars;
        }

        const CharPointerType dest (createUninitialisedBytes (bytesNeeded));
        CharPointerType (dest).writeWithCharLimit (text, (int) numChars + 1);
        return dest;
    }

    template <class CharPointer>
    static CharPointerType createFromCharPointer (const CharPointer& start, const CharPointer& end)
    {
        if (start.getAddress() == nullptr || start.isEmpty())
            return getEmpty();

        CharPointer e (start);
        int numChars = 0;
        size_t bytesNeeded = sizeof (CharType);

        while (e < end && ! e.isEmpty())
        {
            bytesNeeded += CharPointerType::getBytesRequiredFor (e.getAndAdvance());
            ++numChars;
        }

        const CharPointerType dest (createUninitialisedBytes (bytesNeeded));
        CharPointerType (dest).writeWithCharLimit (start, numChars + 1);
        return dest;
    }

    static CharPointerType createFromCharPointer (const CharPointerType& start, const CharPointerType& end)
    {
        if (start.getAddress() == nullptr || start.isEmpty())
            return getEmpty();

        const size_t numBytes = (size_t) (end.getAddress() - start.getAddress());
        const CharPointerType dest (createUninitialisedBytes (numBytes + 1));
        memcpy (dest.getAddress(), start, numBytes);
        dest.getAddress()[numBytes] = 0;
        return dest;
    }

    static CharPointerType createFromFixedLength (const char* const src, const size_t numChars)
    {
        const CharPointerType dest (createUninitialisedBytes (numChars * sizeof (CharType) + sizeof (CharType)));
        CharPointerType (dest).writeWithCharLimit (CharPointer_UTF8 (src), (int) (numChars + 1));
        return dest;
    }

    static inline CharPointerType getEmpty() noexcept
    {
        return CharPointerType (empty.text);
    }

    //==============================================================================
    static void retain (const CharPointerType& text) noexcept
    {
        ++(bufferFromText (text)->refCount);
    }

    static inline void release (StringHolder* const b) noexcept
    {
        if (--(b->refCount) == -1 && b != &empty)
            delete[] reinterpret_cast <char*> (b);
    }

    static void release (const CharPointerType& text) noexcept
    {
        release (bufferFromText (text));
    }

    //==============================================================================
    static CharPointerType makeUnique (const CharPointerType& text)
    {
        StringHolder* const b = bufferFromText (text);

        if (b->refCount.get() <= 0)
            return text;

        CharPointerType newText (createUninitialisedBytes (b->allocatedNumBytes));
        memcpy (newText.getAddress(), text.getAddress(), b->allocatedNumBytes);
        release (b);

        return newText;
    }

    static CharPointerType makeUniqueWithByteSize (const CharPointerType& text, size_t numBytes)
    {
        StringHolder* const b = bufferFromText (text);

        if (b->refCount.get() <= 0 && b->allocatedNumBytes >= numBytes)
            return text;

        CharPointerType newText (createUninitialisedBytes (jmax (b->allocatedNumBytes, numBytes)));
        memcpy (newText.getAddress(), text.getAddress(), b->allocatedNumBytes);
        release (b);

        return newText;
    }

    static size_t getAllocatedNumBytes (const CharPointerType& text) noexcept
    {
        return bufferFromText (text)->allocatedNumBytes;
    }

    //==============================================================================
    Atomic<int> refCount;
    size_t allocatedNumBytes;
    CharType text[1];

    static StringHolder empty;

private:
    static inline StringHolder* bufferFromText (const CharPointerType& text) noexcept
    {
        // (Can't use offsetof() here because of warnings about this not being a POD)
        return reinterpret_cast <StringHolder*> (reinterpret_cast <char*> (text.getAddress())
                    - (reinterpret_cast <size_t> (reinterpret_cast <StringHolder*> (1)->text) - 1));
    }

    void compileTimeChecks()
    {
        // Let me know if any of these assertions fail on your system!
       #if JUCE_NATIVE_WCHAR_IS_UTF8
        static_jassert (sizeof (wchar_t) == 1);
       #elif JUCE_NATIVE_WCHAR_IS_UTF16
        static_jassert (sizeof (wchar_t) == 2);
       #elif JUCE_NATIVE_WCHAR_IS_UTF32
        static_jassert (sizeof (wchar_t) == 4);
       #else
        #error "native wchar_t size is unknown"
       #endif
    }
};

StringHolder StringHolder::empty;
const String String::empty;

//==============================================================================
void String::preallocateBytes (const size_t numBytesNeeded)
{
    text = StringHolder::makeUniqueWithByteSize (text, numBytesNeeded + sizeof (CharPointerType::CharType));
}

//==============================================================================
String::String() noexcept
    : text (StringHolder::getEmpty())
{
}

String::~String() noexcept
{
    StringHolder::release (text);
}

String::String (const String& other) noexcept
    : text (other.text)
{
    StringHolder::retain (text);
}

void String::swapWith (String& other) noexcept
{
    std::swap (text, other.text);
}

String& String::operator= (const String& other) noexcept
{
    StringHolder::retain (other.text);
    StringHolder::release (text.atomicSwap (other.text));
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
String::String (String&& other) noexcept
    : text (other.text)
{
    other.text = StringHolder::getEmpty();
}

String& String::operator= (String&& other) noexcept
{
    std::swap (text, other.text);
    return *this;
}
#endif

inline String::PreallocationBytes::PreallocationBytes (const size_t numBytes_) : numBytes (numBytes_) {}

String::String (const PreallocationBytes& preallocationSize)
    : text (StringHolder::createUninitialisedBytes (preallocationSize.numBytes + sizeof (CharPointerType::CharType)))
{
}

//==============================================================================
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
    jassert (t == nullptr || CharPointer_ASCII::isValidString (t, std::numeric_limits<int>::max()));
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
    jassert (t == nullptr || CharPointer_ASCII::isValidString (t, (int) maxChars));
}

String::String (const wchar_t* const t)      : text (StringHolder::createFromCharPointer (castToCharPointer_wchar_t (t))) {}
String::String (const CharPointer_UTF8&  t)  : text (StringHolder::createFromCharPointer (t)) {}
String::String (const CharPointer_UTF16& t)  : text (StringHolder::createFromCharPointer (t)) {}
String::String (const CharPointer_UTF32& t)  : text (StringHolder::createFromCharPointer (t)) {}
String::String (const CharPointer_ASCII& t)  : text (StringHolder::createFromCharPointer (t)) {}

String::String (const CharPointer_UTF8&  t, const size_t maxChars)  : text (StringHolder::createFromCharPointer (t, maxChars)) {}
String::String (const CharPointer_UTF16& t, const size_t maxChars)  : text (StringHolder::createFromCharPointer (t, maxChars)) {}
String::String (const CharPointer_UTF32& t, const size_t maxChars)  : text (StringHolder::createFromCharPointer (t, maxChars)) {}
String::String (const wchar_t* const t, size_t maxChars)            : text (StringHolder::createFromCharPointer (castToCharPointer_wchar_t (t), maxChars)) {}

String::String (const CharPointer_UTF8&  start, const CharPointer_UTF8&  end) : text (StringHolder::createFromCharPointer (start, end)) {}
String::String (const CharPointer_UTF16& start, const CharPointer_UTF16& end) : text (StringHolder::createFromCharPointer (start, end)) {}
String::String (const CharPointer_UTF32& start, const CharPointer_UTF32& end) : text (StringHolder::createFromCharPointer (start, end)) {}

String String::charToString (const juce_wchar character)
{
    String result (PreallocationBytes (CharPointerType::getBytesRequiredFor (character)));
    CharPointerType t (result.text);
    t.write (character);
    t.writeNull();
    return result;
}

//==============================================================================
namespace NumberToStringConverters
{
    // pass in a pointer to the END of a buffer..
    static char* numberToString (char* t, const int64 n) noexcept
    {
        *--t = 0;
        int64 v = (n >= 0) ? n : -n;

        do
        {
            *--t = (char) ('0' + (int) (v % 10));
            v /= 10;

        } while (v > 0);

        if (n < 0)
            *--t = '-';

        return t;
    }

    static char* numberToString (char* t, uint64 v) noexcept
    {
        *--t = 0;

        do
        {
            *--t = (char) ('0' + (int) (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    static char* numberToString (char* t, const int n) noexcept
    {
        if (n == (int) 0x80000000) // (would cause an overflow)
            return numberToString (t, (int64) n);

        *--t = 0;
        int v = abs (n);

        do
        {
            *--t = (char) ('0' + (v % 10));
            v /= 10;

        } while (v > 0);

        if (n < 0)
            *--t = '-';

        return t;
    }

    static char* numberToString (char* t, unsigned int v) noexcept
    {
        *--t = 0;

        do
        {
            *--t = (char) ('0' + (v % 10));
            v /= 10;

        } while (v > 0);

        return t;
    }

    static char* doubleToString (char* buffer, const int numChars, double n, int numDecPlaces, size_t& len) noexcept
    {
        if (numDecPlaces > 0 && numDecPlaces < 7 && n > -1.0e20 && n < 1.0e20)
        {
            char* const end = buffer + numChars;
            char* t = end;
            int64 v = (int64) (pow (10.0, numDecPlaces) * std::abs (n) + 0.5);
            *--t = (char) 0;

            while (numDecPlaces >= 0 || v > 0)
            {
                if (numDecPlaces == 0)
                    *--t = '.';

                *--t = (char) ('0' + (v % 10));

                v /= 10;
                --numDecPlaces;
            }

            if (n < 0)
                *--t = '-';

            len = (size_t) (end - t - 1);
            return t;
        }

       // Use a locale-free sprintf where possible (not available on linux AFAICT)
       #if JUCE_MSVC
        static _locale_t cLocale = _create_locale (LC_NUMERIC, "C");

        len = (size_t) (numDecPlaces > 0 ? _sprintf_l (buffer, "%.*f", cLocale, numDecPlaces, n)
                                         : _sprintf_l (buffer, "%.9g", cLocale, n));
       #elif JUCE_MAC || JUCE_IOS
        len = (size_t) (numDecPlaces > 0 ? sprintf_l (buffer, nullptr, "%.*f", numDecPlaces, n)
                                         : sprintf_l (buffer, nullptr, "%.9g", n));
       #else
        len = (size_t) (numDecPlaces > 0 ? sprintf (buffer, "%.*f", numDecPlaces, n)
                                         : sprintf (buffer, "%.9g", n));
       #endif

        return buffer;
    }

    template <typename IntegerType>
    String::CharPointerType createFromInteger (const IntegerType number)
    {
        char buffer [32];
        char* const end = buffer + numElementsInArray (buffer);
        char* const start = numberToString (end, number);

        return StringHolder::createFromFixedLength (start, (size_t) (end - start - 1));
    }

    static String::CharPointerType createFromDouble (const double number, const int numberOfDecimalPlaces)
    {
        char buffer [48];
        size_t len;
        char* const start = doubleToString (buffer, numElementsInArray (buffer), (double) number, numberOfDecimalPlaces, len);
        return StringHolder::createFromFixedLength (start, len);
    }
}

//==============================================================================
String::String (const int number)            : text (NumberToStringConverters::createFromInteger (number)) {}
String::String (const unsigned int number)   : text (NumberToStringConverters::createFromInteger (number)) {}
String::String (const short number)          : text (NumberToStringConverters::createFromInteger ((int) number)) {}
String::String (const unsigned short number) : text (NumberToStringConverters::createFromInteger ((unsigned int) number)) {}
String::String (const int64 number)          : text (NumberToStringConverters::createFromInteger (number)) {}
String::String (const uint64 number)         : text (NumberToStringConverters::createFromInteger (number)) {}

String::String (const float number)          : text (NumberToStringConverters::createFromDouble ((double) number, 0)) {}
String::String (const double number)         : text (NumberToStringConverters::createFromDouble (number, 0)) {}
String::String (const float number, const int numberOfDecimalPlaces)   : text (NumberToStringConverters::createFromDouble ((double) number, numberOfDecimalPlaces)) {}
String::String (const double number, const int numberOfDecimalPlaces)  : text (NumberToStringConverters::createFromDouble (number, numberOfDecimalPlaces)) {}

//==============================================================================
int String::length() const noexcept
{
    return (int) text.length();
}

size_t String::getByteOffsetOfEnd() const noexcept
{
    return (size_t) (((char*) text.findTerminatingNull().getAddress()) - (char*) text.getAddress());
}

juce_wchar String::operator[] (int index) const noexcept
{
    jassert (index == 0 || (index > 0 && index <= (int) text.lengthUpTo ((size_t) index + 1)));
    return text [index];
}

int String::hashCode() const noexcept
{
    CharPointerType t (text);
    int result = 0;

    while (! t.isEmpty())
        result = 31 * result + (int) t.getAndAdvance();

    return result;
}

int64 String::hashCode64() const noexcept
{
    CharPointerType t (text);
    int64 result = 0;

    while (! t.isEmpty())
        result = 101 * result + t.getAndAdvance();

    return result;
}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE operator== (const String& s1, const String& s2) noexcept            { return s1.compare (s2) == 0; }
JUCE_API bool JUCE_CALLTYPE operator== (const String& s1, const char* const s2) noexcept        { return s1.compare (s2) == 0; }
JUCE_API bool JUCE_CALLTYPE operator== (const String& s1, const wchar_t* const s2) noexcept     { return s1.compare (s2) == 0; }
JUCE_API bool JUCE_CALLTYPE operator== (const String& s1, const CharPointer_UTF8& s2) noexcept  { return s1.getCharPointer().compare (s2) == 0; }
JUCE_API bool JUCE_CALLTYPE operator== (const String& s1, const CharPointer_UTF16& s2) noexcept { return s1.getCharPointer().compare (s2) == 0; }
JUCE_API bool JUCE_CALLTYPE operator== (const String& s1, const CharPointer_UTF32& s2) noexcept { return s1.getCharPointer().compare (s2) == 0; }
JUCE_API bool JUCE_CALLTYPE operator!= (const String& s1, const String& s2) noexcept            { return s1.compare (s2) != 0; }
JUCE_API bool JUCE_CALLTYPE operator!= (const String& s1, const char* const s2) noexcept        { return s1.compare (s2) != 0; }
JUCE_API bool JUCE_CALLTYPE operator!= (const String& s1, const wchar_t* const s2) noexcept     { return s1.compare (s2) != 0; }
JUCE_API bool JUCE_CALLTYPE operator!= (const String& s1, const CharPointer_UTF8& s2) noexcept  { return s1.getCharPointer().compare (s2) != 0; }
JUCE_API bool JUCE_CALLTYPE operator!= (const String& s1, const CharPointer_UTF16& s2) noexcept { return s1.getCharPointer().compare (s2) != 0; }
JUCE_API bool JUCE_CALLTYPE operator!= (const String& s1, const CharPointer_UTF32& s2) noexcept { return s1.getCharPointer().compare (s2) != 0; }
JUCE_API bool JUCE_CALLTYPE operator>  (const String& s1, const String& s2) noexcept            { return s1.compare (s2) > 0; }
JUCE_API bool JUCE_CALLTYPE operator<  (const String& s1, const String& s2) noexcept            { return s1.compare (s2) < 0; }
JUCE_API bool JUCE_CALLTYPE operator>= (const String& s1, const String& s2) noexcept            { return s1.compare (s2) >= 0; }
JUCE_API bool JUCE_CALLTYPE operator<= (const String& s1, const String& s2) noexcept            { return s1.compare (s2) <= 0; }

bool String::equalsIgnoreCase (const wchar_t* const t) const noexcept
{
    return t != nullptr ? text.compareIgnoreCase (castToCharPointer_wchar_t (t)) == 0
                        : isEmpty();
}

bool String::equalsIgnoreCase (const char* const t) const noexcept
{
    return t != nullptr ? text.compareIgnoreCase (CharPointer_UTF8 (t)) == 0
                        : isEmpty();
}

bool String::equalsIgnoreCase (const String& other) const noexcept
{
    return text == other.text
            || text.compareIgnoreCase (other.text) == 0;
}

int String::compare (const String& other) const noexcept           { return (text == other.text) ? 0 : text.compare (other.text); }
int String::compare (const char* const other) const noexcept       { return text.compare (CharPointer_UTF8 (other)); }
int String::compare (const wchar_t* const other) const noexcept    { return text.compare (castToCharPointer_wchar_t (other)); }
int String::compareIgnoreCase (const String& other) const noexcept { return (text == other.text) ? 0 : text.compareIgnoreCase (other.text); }

int String::compareLexicographically (const String& other) const noexcept
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

String& String::operator+= (const wchar_t* const t)
{
    appendCharPointer (castToCharPointer_wchar_t (t));
    return *this;
}

String& String::operator+= (const char* const t)
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
    jassert (t == nullptr || CharPointer_ASCII::isValidString (t, std::numeric_limits<int>::max()));

    appendCharPointer (CharPointer_ASCII (t));
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
    const char asString[] = { ch, 0 };
    return operator+= (asString);
}

String& String::operator+= (const wchar_t ch)
{
    const wchar_t asString[] = { ch, 0 };
    return operator+= (asString);
}

#if ! JUCE_NATIVE_WCHAR_IS_UTF32
String& String::operator+= (const juce_wchar ch)
{
    const juce_wchar asString[] = { ch, 0 };
    appendCharPointer (CharPointer_UTF32 (asString));
    return *this;
}
#endif

String& String::operator+= (const int number)
{
    char buffer [16];
    char* const end = buffer + numElementsInArray (buffer);
    char* const start = NumberToStringConverters::numberToString (end, number);

    const int numExtraChars = (int) (end - start);

    if (numExtraChars > 0)
    {
        const size_t byteOffsetOfNull = getByteOffsetOfEnd();
        const size_t newBytesNeeded = sizeof (CharPointerType::CharType) + byteOffsetOfNull
                                        + sizeof (CharPointerType::CharType) * (size_t) numExtraChars;

        text = StringHolder::makeUniqueWithByteSize (text, newBytesNeeded);

        CharPointerType newEnd (addBytesToPointer (text.getAddress(), (int) byteOffsetOfNull));
        newEnd.writeWithCharLimit (CharPointer_ASCII (start), numExtraChars);
    }

    return *this;
}

//==============================================================================
JUCE_API String JUCE_CALLTYPE operator+ (const char* const string1, const String& string2)
{
    String s (string1);
    return s += string2;
}

JUCE_API String JUCE_CALLTYPE operator+ (const wchar_t* const string1, const String& string2)
{
    String s (string1);
    return s += string2;
}

JUCE_API String JUCE_CALLTYPE operator+ (const char s1, const String& s2)       { return String::charToString ((juce_wchar) (uint8) s1) + s2; }
JUCE_API String JUCE_CALLTYPE operator+ (const wchar_t s1, const String& s2)    { return String::charToString (s1) + s2; }
#if ! JUCE_NATIVE_WCHAR_IS_UTF32
JUCE_API String JUCE_CALLTYPE operator+ (const juce_wchar s1, const String& s2) { return String::charToString (s1) + s2; }
#endif

JUCE_API String JUCE_CALLTYPE operator+ (String s1, const String& s2)       { return s1 += s2; }
JUCE_API String JUCE_CALLTYPE operator+ (String s1, const char* const s2)   { return s1 += s2; }
JUCE_API String JUCE_CALLTYPE operator+ (String s1, const wchar_t* s2)      { return s1 += s2; }

JUCE_API String JUCE_CALLTYPE operator+ (String s1, const char s2)          { return s1 += s2; }
JUCE_API String JUCE_CALLTYPE operator+ (String s1, const wchar_t s2)       { return s1 += s2; }
#if ! JUCE_NATIVE_WCHAR_IS_UTF32
JUCE_API String JUCE_CALLTYPE operator+ (String s1, const juce_wchar s2)    { return s1 += s2; }
#endif

JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const char s2)             { return s1 += s2; }
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const wchar_t s2)          { return s1 += s2; }
#if ! JUCE_NATIVE_WCHAR_IS_UTF32
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const juce_wchar s2)       { return s1 += s2; }
#endif

JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const char* const s2)      { return s1 += s2; }
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const wchar_t* const s2)   { return s1 += s2; }
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const String& s2)          { return s1 += s2; }

JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const short number)        { return s1 += (int) number; }
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const int number)          { return s1 += number; }
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const long number)         { return s1 += (int) number; }
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const int64 number)        { return s1 << String (number); }
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const float number)        { return s1 += String (number); }
JUCE_API String& JUCE_CALLTYPE operator<< (String& s1, const double number)       { return s1 += String (number); }

JUCE_API OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const String& text)
{
    const size_t numBytes = text.getNumBytesAsUTF8();

   #if (JUCE_STRING_UTF_TYPE == 8)
    stream.write (text.getCharPointer().getAddress(), numBytes);
   #else
    // (This avoids using toUTF8() to prevent the memory bloat that it would leave behind
    // if lots of large, persistent strings were to be written to streams).
    HeapBlock<char> temp (numBytes + 1);
    CharPointer_UTF8 (temp).writeAll (text.getCharPointer());
    stream.write (temp, numBytes);
   #endif

    return stream;
}

JUCE_API String& JUCE_CALLTYPE operator<< (String& string1, const NewLine&)
{
    return string1 += NewLine::getDefault();
}

//==============================================================================
int String::indexOfChar (const juce_wchar character) const noexcept
{
    return text.indexOf (character);
}

int String::indexOfChar (const int startIndex, const juce_wchar character) const noexcept
{
    CharPointerType t (text);

    for (int i = 0; ! t.isEmpty(); ++i)
    {
        if (i >= startIndex)
        {
            if (t.getAndAdvance() == character)
                return i;
        }
        else
        {
            ++t;
        }
    }

    return -1;
}

int String::lastIndexOfChar (const juce_wchar character) const noexcept
{
    CharPointerType t (text);
    int last = -1;

    for (int i = 0; ! t.isEmpty(); ++i)
        if (t.getAndAdvance() == character)
            last = i;

    return last;
}

int String::indexOfAnyOf (const String& charactersToLookFor, const int startIndex, const bool ignoreCase) const noexcept
{
    CharPointerType t (text);

    for (int i = 0; ! t.isEmpty(); ++i)
    {
        if (i >= startIndex)
        {
            if (charactersToLookFor.text.indexOf (t.getAndAdvance(), ignoreCase) >= 0)
                return i;
        }
        else
        {
            ++t;
        }
    }

    return -1;
}

int String::indexOf (const String& other) const noexcept
{
    return other.isEmpty() ? 0 : text.indexOf (other.text);
}

int String::indexOfIgnoreCase (const String& other) const noexcept
{
    return other.isEmpty() ? 0 : CharacterFunctions::indexOfIgnoreCase (text, other.text);
}

int String::indexOf (const int startIndex, const String& other) const noexcept
{
    if (other.isEmpty())
        return -1;

    CharPointerType t (text);

    for (int i = startIndex; --i >= 0;)
    {
        if (t.isEmpty())
            return -1;

        ++t;
    }

    int found = t.indexOf (other.text);
    if (found >= 0)
        found += startIndex;
    return found;
}

int String::indexOfIgnoreCase (const int startIndex, const String& other) const noexcept
{
    if (other.isEmpty())
        return -1;

    CharPointerType t (text);

    for (int i = startIndex; --i >= 0;)
    {
        if (t.isEmpty())
            return -1;

        ++t;
    }

    int found = CharacterFunctions::indexOfIgnoreCase (t, other.text);
    if (found >= 0)
        found += startIndex;
    return found;
}

int String::lastIndexOf (const String& other) const noexcept
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

int String::lastIndexOfIgnoreCase (const String& other) const noexcept
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

int String::lastIndexOfAnyOf (const String& charactersToLookFor, const bool ignoreCase) const noexcept
{
    CharPointerType t (text);
    int last = -1;

    for (int i = 0; ! t.isEmpty(); ++i)
        if (charactersToLookFor.text.indexOf (t.getAndAdvance(), ignoreCase) >= 0)
            last = i;

    return last;
}

bool String::contains (const String& other) const noexcept
{
    return indexOf (other) >= 0;
}

bool String::containsChar (const juce_wchar character) const noexcept
{
    return text.indexOf (character) >= 0;
}

bool String::containsIgnoreCase (const String& t) const noexcept
{
    return indexOfIgnoreCase (t) >= 0;
}

int String::indexOfWholeWord (const String& word) const noexcept
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

int String::indexOfWholeWordIgnoreCase (const String& word) const noexcept
{
    if (word.isNotEmpty())
    {
        CharPointerType t (text);
        const int wordLen = word.length();
        const int end = (int) t.length() - wordLen;

        for (int i = 0; i <= end; ++i)
        {
            if (t.compareIgnoreCaseUpTo (word.text, wordLen) == 0
                  && (i == 0 || ! (t - 1).isLetterOrDigit())
                  && ! (t + wordLen).isLetterOrDigit())
                return i;

            ++t;
        }
    }

    return -1;
}

bool String::containsWholeWord (const String& wordToLookFor) const noexcept
{
    return indexOfWholeWord (wordToLookFor) >= 0;
}

bool String::containsWholeWordIgnoreCase (const String& wordToLookFor) const noexcept
{
    return indexOfWholeWordIgnoreCase (wordToLookFor) >= 0;
}

//==============================================================================
template <typename CharPointer>
struct WildCardMatcher
{
    static bool matches (CharPointer wildcard, CharPointer test, const bool ignoreCase) noexcept
    {
        for (;;)
        {
            const juce_wchar wc = wildcard.getAndAdvance();

            if (wc == '*')
                return wildcard.isEmpty() || matchesAnywhere (wildcard, test, ignoreCase);

            if (! characterMatches (wc, test.getAndAdvance(), ignoreCase))
                return false;

            if (wc == 0)
                return true;
        }
    }

    static bool characterMatches (const juce_wchar wc, const juce_wchar tc, const bool ignoreCase) noexcept
    {
        return (wc == tc) || (wc == '?' && tc != 0)
                || (ignoreCase && CharacterFunctions::toLowerCase (wc) == CharacterFunctions::toLowerCase (tc));
    }

    static bool matchesAnywhere (const CharPointer& wildcard, CharPointer test, const bool ignoreCase) noexcept
    {
        for (; ! test.isEmpty(); ++test)
            if (matches (wildcard, test, ignoreCase))
                return true;

        return false;
    }
};

bool String::matchesWildcard (const String& wildcard, const bool ignoreCase) const noexcept
{
    return WildCardMatcher<CharPointerType>::matches (wildcard.text, text, ignoreCase);
}

//==============================================================================
String String::repeatedString (const String& stringToRepeat, int numberOfTimesToRepeat)
{
    if (numberOfTimesToRepeat <= 0)
        return empty;

    String result (PreallocationBytes (stringToRepeat.getByteOffsetOfEnd() * (size_t) numberOfTimesToRepeat));
    CharPointerType n (result.text);

    while (--numberOfTimesToRepeat >= 0)
        n.writeAll (stringToRepeat.text);

    return result;
}

String String::paddedLeft (const juce_wchar padCharacter, int minimumLength) const
{
    jassert (padCharacter != 0);

    int extraChars = minimumLength;
    CharPointerType end (text);

    while (! end.isEmpty())
    {
        --extraChars;
        ++end;
    }

    if (extraChars <= 0 || padCharacter == 0)
        return *this;

    const size_t currentByteSize = (size_t) (((char*) end.getAddress()) - (char*) text.getAddress());
    String result (PreallocationBytes (currentByteSize + (size_t) extraChars * CharPointerType::getBytesRequiredFor (padCharacter)));
    CharPointerType n (result.text);

    while (--extraChars >= 0)
        n.write (padCharacter);

    n.writeAll (text);
    return result;
}

String String::paddedRight (const juce_wchar padCharacter, int minimumLength) const
{
    jassert (padCharacter != 0);

    int extraChars = minimumLength;
    CharPointerType end (text);

    while (! end.isEmpty())
    {
        --extraChars;
        ++end;
    }

    if (extraChars <= 0 || padCharacter == 0)
        return *this;

    const size_t currentByteSize = (size_t) (((char*) end.getAddress()) - (char*) text.getAddress());
    String result (PreallocationBytes (currentByteSize + (size_t) extraChars * CharPointerType::getBytesRequiredFor (padCharacter)));
    CharPointerType n (result.text);

    n.writeAll (text);

    while (--extraChars >= 0)
        n.write (padCharacter);

    n.writeNull();
    return result;
}

//==============================================================================
String String::replaceSection (int index, int numCharsToReplace, const String& stringToInsert) const
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

    int i = 0;
    CharPointerType insertPoint (text);

    while (i < index)
    {
        if (insertPoint.isEmpty())
        {
            // replacing beyond the end of the string?
            jassertfalse;
            return *this + stringToInsert;
        }

        ++insertPoint;
        ++i;
    }

    CharPointerType startOfRemainder (insertPoint);

    i = 0;
    while (i < numCharsToReplace && ! startOfRemainder.isEmpty())
    {
        ++startOfRemainder;
        ++i;
    }

    if (insertPoint == text && startOfRemainder.isEmpty())
        return stringToInsert;

    const size_t initialBytes = (size_t) (((char*) insertPoint.getAddress()) - (char*) text.getAddress());
    const size_t newStringBytes = stringToInsert.getByteOffsetOfEnd();
    const size_t remainderBytes = (size_t) (((char*) startOfRemainder.findTerminatingNull().getAddress()) - (char*) startOfRemainder.getAddress());

    const size_t newTotalBytes = initialBytes + newStringBytes + remainderBytes;
    if (newTotalBytes <= 0)
        return String::empty;

    String result (PreallocationBytes ((size_t) newTotalBytes));

    char* dest = (char*) result.text.getAddress();
    memcpy (dest, text.getAddress(), initialBytes);
    dest += initialBytes;
    memcpy (dest, stringToInsert.text.getAddress(), newStringBytes);
    dest += newStringBytes;
    memcpy (dest, startOfRemainder.getAddress(), remainderBytes);
    dest += remainderBytes;
    CharPointerType ((CharPointerType::CharType*) dest).writeNull();

    return result;
}

String String::replace (const String& stringToReplace, const String& stringToInsert, const bool ignoreCase) const
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

class StringCreationHelper
{
public:
    StringCreationHelper (const size_t initialBytes)
        : source (nullptr), dest (nullptr), allocatedBytes (initialBytes), bytesWritten (0)
    {
        result.preallocateBytes (allocatedBytes);
        dest = result.getCharPointer();
    }

    StringCreationHelper (const String::CharPointerType& source_)
        : source (source_), dest (nullptr), allocatedBytes (StringHolder::getAllocatedNumBytes (source)), bytesWritten (0)
    {
        result.preallocateBytes (allocatedBytes);
        dest = result.getCharPointer();
    }

    void write (juce_wchar c)
    {
        bytesWritten += String::CharPointerType::getBytesRequiredFor (c);

        if (bytesWritten > allocatedBytes)
        {
            allocatedBytes += jmax ((size_t) 8, allocatedBytes / 16);
            const size_t destOffset = (size_t) (((char*) dest.getAddress()) - (char*) result.getCharPointer().getAddress());
            result.preallocateBytes (allocatedBytes);
            dest = addBytesToPointer (result.getCharPointer().getAddress(), (int) destOffset);
        }

        dest.write (c);
    }

    String result;
    String::CharPointerType source;

private:
    String::CharPointerType dest;
    size_t allocatedBytes, bytesWritten;
};

String String::replaceCharacter (const juce_wchar charToReplace, const juce_wchar charToInsert) const
{
    if (! containsChar (charToReplace))
        return *this;

    StringCreationHelper builder (text);

    for (;;)
    {
        juce_wchar c = builder.source.getAndAdvance();

        if (c == charToReplace)
            c = charToInsert;

        builder.write (c);

        if (c == 0)
            break;
    }

    return builder.result;
}

String String::replaceCharacters (const String& charactersToReplace, const String& charactersToInsertInstead) const
{
    StringCreationHelper builder (text);

    for (;;)
    {
        juce_wchar c = builder.source.getAndAdvance();

        const int index = charactersToReplace.indexOfChar (c);
        if (index >= 0)
            c = charactersToInsertInstead [index];

        builder.write (c);

        if (c == 0)
            break;
    }

    return builder.result;
}

//==============================================================================
bool String::startsWith (const String& other) const noexcept
{
    return text.compareUpTo (other.text, other.length()) == 0;
}

bool String::startsWithIgnoreCase (const String& other) const noexcept
{
    return text.compareIgnoreCaseUpTo (other.text, other.length()) == 0;
}

bool String::startsWithChar (const juce_wchar character) const noexcept
{
    jassert (character != 0); // strings can't contain a null character!

    return *text == character;
}

bool String::endsWithChar (const juce_wchar character) const noexcept
{
    jassert (character != 0); // strings can't contain a null character!

    if (text.isEmpty())
        return false;

    CharPointerType t (text.findTerminatingNull());
    return *--t == character;
}

bool String::endsWith (const String& other) const noexcept
{
    CharPointerType end (text.findTerminatingNull());
    CharPointerType otherEnd (other.text.findTerminatingNull());

    while (end > text && otherEnd > other.text)
    {
        --end;
        --otherEnd;

        if (*end != *otherEnd)
            return false;
    }

    return otherEnd == other.text;
}

bool String::endsWithIgnoreCase (const String& other) const noexcept
{
    CharPointerType end (text.findTerminatingNull());
    CharPointerType otherEnd (other.text.findTerminatingNull());

    while (end > text && otherEnd > other.text)
    {
        --end;
        --otherEnd;

        if (end.toLowerCase() != otherEnd.toLowerCase())
            return false;
    }

    return otherEnd == other.text;
}

//==============================================================================
String String::toUpperCase() const
{
    StringCreationHelper builder (text);

    for (;;)
    {
        const juce_wchar c = builder.source.toUpperCase();
        ++(builder.source);
        builder.write (c);

        if (c == 0)
            break;
    }

    return builder.result;
}

String String::toLowerCase() const
{
    StringCreationHelper builder (text);

    for (;;)
    {
        const juce_wchar c = builder.source.toLowerCase();
        ++(builder.source);
        builder.write (c);

        if (c == 0)
            break;
    }

    return builder.result;
}

//==============================================================================
juce_wchar String::getLastCharacter() const noexcept
{
    return isEmpty() ? juce_wchar() : text [length() - 1];
}

String String::substring (int start, const int end) const
{
    if (start < 0)
        start = 0;

    if (end <= start)
        return empty;

    int i = 0;
    CharPointerType t1 (text);

    while (i < start)
    {
        if (t1.isEmpty())
            return empty;

        ++i;
        ++t1;
    }

    CharPointerType t2 (t1);
    while (i < end)
    {
        if (t2.isEmpty())
        {
            if (start == 0)
                return *this;

            break;
        }

        ++i;
        ++t2;
    }

    return String (t1, t2);
}

String String::substring (int start) const
{
    if (start <= 0)
        return *this;

    CharPointerType t (text);

    while (--start >= 0)
    {
        if (t.isEmpty())
            return empty;

        ++t;
    }

    return String (t);
}

String String::dropLastCharacters (const int numberToDrop) const
{
    return String (text, (size_t) jmax (0, length() - numberToDrop));
}

String String::getLastCharacters (const int numCharacters) const
{
    return String (text + jmax (0, length() - jmax (0, numCharacters)));
}

String String::fromFirstOccurrenceOf (const String& sub,
                                      const bool includeSubString,
                                      const bool ignoreCase) const
{
    const int i = ignoreCase ? indexOfIgnoreCase (sub)
                             : indexOf (sub);
    if (i < 0)
        return empty;

    return substring (includeSubString ? i : i + sub.length());
}

String String::fromLastOccurrenceOf (const String& sub,
                                     const bool includeSubString,
                                     const bool ignoreCase) const
{
    const int i = ignoreCase ? lastIndexOfIgnoreCase (sub)
                             : lastIndexOf (sub);
    if (i < 0)
        return *this;

    return substring (includeSubString ? i : i + sub.length());
}

String String::upToFirstOccurrenceOf (const String& sub,
                                      const bool includeSubString,
                                      const bool ignoreCase) const
{
    const int i = ignoreCase ? indexOfIgnoreCase (sub)
                             : indexOf (sub);
    if (i < 0)
        return *this;

    return substring (0, includeSubString ? i + sub.length() : i);
}

String String::upToLastOccurrenceOf (const String& sub,
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

String String::unquoted() const
{
    const int len = length();

    if (len == 0)
        return empty;

    const juce_wchar lastChar = text [len - 1];
    const int dropAtStart = (*text == '"' || *text == '\'') ? 1 : 0;
    const int dropAtEnd = (lastChar == '"' || lastChar == '\'') ? 1 : 0;

    return substring (dropAtStart, len - dropAtEnd);
}

String String::quoted (const juce_wchar quoteCharacter) const
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
static String::CharPointerType findTrimmedEnd (const String::CharPointerType& start, String::CharPointerType end)
{
    while (end > start)
    {
        if (! (--end).isWhitespace())
        {
            ++end;
            break;
        }
    }

    return end;
}

String String::trim() const
{
    if (isNotEmpty())
    {
        CharPointerType start (text.findEndOfWhitespace());

        const CharPointerType end (start.findTerminatingNull());
        CharPointerType trimmedEnd (findTrimmedEnd (start, end));

        if (trimmedEnd <= start)
            return empty;

        if (text < start || trimmedEnd < end)
            return String (start, trimmedEnd);
    }

    return *this;
}

String String::trimStart() const
{
    if (isNotEmpty())
    {
        const CharPointerType t (text.findEndOfWhitespace());

        if (t != text)
            return String (t);
    }

    return *this;
}

String String::trimEnd() const
{
    if (isNotEmpty())
    {
        const CharPointerType end (text.findTerminatingNull());
        CharPointerType trimmedEnd (findTrimmedEnd (text, end));

        if (trimmedEnd < end)
            return String (text, trimmedEnd);
    }

    return *this;
}

String String::trimCharactersAtStart (const String& charactersToTrim) const
{
    CharPointerType t (text);

    while (charactersToTrim.containsChar (*t))
        ++t;

    return t == text ? *this : String (t);
}

String String::trimCharactersAtEnd (const String& charactersToTrim) const
{
    if (isNotEmpty())
    {
        const CharPointerType end (text.findTerminatingNull());
        CharPointerType trimmedEnd (end);

        while (trimmedEnd > text)
        {
            if (! charactersToTrim.containsChar (*--trimmedEnd))
            {
                ++trimmedEnd;
                break;
            }
        }

        if (trimmedEnd < end)
            return String (text, trimmedEnd);
    }

    return *this;
}

//==============================================================================
String String::retainCharacters (const String& charactersToRetain) const
{
    if (isEmpty())
        return empty;

    StringCreationHelper builder (text);

    for (;;)
    {
        juce_wchar c = builder.source.getAndAdvance();

        if (charactersToRetain.containsChar (c))
            builder.write (c);

        if (c == 0)
            break;
    }

    builder.write (0);
    return builder.result;
}

String String::removeCharacters (const String& charactersToRemove) const
{
    if (isEmpty())
        return empty;

    StringCreationHelper builder (text);

    for (;;)
    {
        juce_wchar c = builder.source.getAndAdvance();

        if (! charactersToRemove.containsChar (c))
            builder.write (c);

        if (c == 0)
            break;
    }

    return builder.result;
}

String String::initialSectionContainingOnly (const String& permittedCharacters) const
{
    CharPointerType t (text);

    while (! t.isEmpty())
    {
        if (! permittedCharacters.containsChar (*t))
            return String (text, t);

        ++t;
    }

    return *this;
}

String String::initialSectionNotContaining (const String& charactersToStopAt) const
{
    CharPointerType t (text);

    while (! t.isEmpty())
    {
        if (charactersToStopAt.containsChar (*t))
            return String (text, t);

        ++t;
    }

    return *this;
}

bool String::containsOnly (const String& chars) const noexcept
{
    CharPointerType t (text);

    while (! t.isEmpty())
        if (! chars.containsChar (t.getAndAdvance()))
            return false;

    return true;
}

bool String::containsAnyOf (const String& chars) const noexcept
{
    CharPointerType t (text);

    while (! t.isEmpty())
        if (chars.containsChar (t.getAndAdvance()))
            return true;

    return false;
}

bool String::containsNonWhitespaceChars() const noexcept
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

// Note! The format parameter here MUST NOT be a reference, otherwise MS's va_start macro fails to work (but still compiles).
String String::formatted (const String pf, ... )
{
    size_t bufferSize = 256;

    for (;;)
    {
        va_list args;
        va_start (args, pf);

       #if JUCE_WINDOWS
        HeapBlock <wchar_t> temp (bufferSize);
        const int num = (int) _vsnwprintf (temp.getData(), bufferSize - 1, pf.toWideCharPointer(), args);
       #elif JUCE_ANDROID
        HeapBlock <char> temp (bufferSize);
        const int num = (int) vsnprintf (temp.getData(), bufferSize - 1, pf.toUTF8(), args);
       #else
        HeapBlock <wchar_t> temp (bufferSize);
        const int num = (int) vswprintf (temp.getData(), bufferSize - 1, pf.toWideCharPointer(), args);
       #endif

        va_end (args);

        if (num > 0)
            return String (temp);

        bufferSize += 256;

        if (num == 0 || bufferSize > 65536) // the upper limit is a sanity check to avoid situations where vprintf repeatedly
            break;                          // returns -1 because of an error rather than because it needs more space.
    }

    return empty;
}

//==============================================================================
int String::getIntValue() const noexcept
{
    return text.getIntValue32();
}

int String::getTrailingIntValue() const noexcept
{
    int n = 0;
    int mult = 1;
    CharPointerType t (text.findTerminatingNull());

    while (--t >= text)
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

int64 String::getLargeIntValue() const noexcept
{
    return text.getIntValue64();
}

float String::getFloatValue() const noexcept
{
    return (float) getDoubleValue();
}

double String::getDoubleValue() const noexcept
{
    return text.getDoubleValue();
}

static const char hexDigits[] = "0123456789abcdef";

template <typename Type>
struct HexConverter
{
    static String hexToString (Type v)
    {
        char buffer[32];
        char* const end = buffer + 32;
        char* t = end;
        *--t = 0;

        do
        {
            *--t = hexDigits [(int) (v & 15)];
            v >>= 4;

        } while (v != 0);

        return String (t, (size_t) (end - t) - 1);
    }

    static Type stringToHex (String::CharPointerType t) noexcept
    {
        Type result = 0;

        while (! t.isEmpty())
        {
            const int hexValue = CharacterFunctions::getHexDigitValue (t.getAndAdvance());

            if (hexValue >= 0)
                result = (result << 4) | hexValue;
        }

        return result;
    }
};

String String::toHexString (const int number)
{
    return HexConverter <unsigned int>::hexToString ((unsigned int) number);
}

String String::toHexString (const int64 number)
{
    return HexConverter <uint64>::hexToString ((uint64) number);
}

String String::toHexString (const short number)
{
    return toHexString ((int) (unsigned short) number);
}

String String::toHexString (const void* const d, const int size, const int groupSize)
{
    if (size <= 0)
        return empty;

    int numChars = (size * 2) + 2;
    if (groupSize > 0)
        numChars += size / groupSize;

    String s (PreallocationBytes (sizeof (CharPointerType::CharType) * (size_t) numChars));

    const unsigned char* data = static_cast <const unsigned char*> (d);
    CharPointerType dest (s.text);

    for (int i = 0; i < size; ++i)
    {
        const unsigned char nextByte = *data++;
        dest.write ((juce_wchar) hexDigits [nextByte >> 4]);
        dest.write ((juce_wchar) hexDigits [nextByte & 0xf]);

        if (groupSize > 0 && (i % groupSize) == (groupSize - 1) && i < (size - 1))
            dest.write ((juce_wchar) ' ');
    }

    dest.writeNull();
    return s;
}

int String::getHexValue32() const noexcept
{
    return HexConverter<int>::stringToHex (text);
}

int64 String::getHexValue64() const noexcept
{
    return HexConverter<int64>::stringToHex (text);
}

//==============================================================================
String String::createStringFromData (const void* const data_, const int size)
{
    const uint8* const data = static_cast <const uint8*> (data_);

    if (size <= 0 || data == nullptr)
        return empty;

    if (size == 1)
        return charToString ((juce_wchar) data[0]);

    if ((data[0] == (uint8) CharPointer_UTF16::byteOrderMarkBE1 && data[1] == (uint8) CharPointer_UTF16::byteOrderMarkBE2)
         || (data[0] == (uint8) CharPointer_UTF16::byteOrderMarkLE1 && data[1] == (uint8) CharPointer_UTF16::byteOrderMarkLE2))
    {
        const bool bigEndian = (data[0] == (uint8) CharPointer_UTF16::byteOrderMarkBE1);
        const int numChars = size / 2 - 1;

        StringCreationHelper builder ((size_t) numChars);

        const uint16* const src = (const uint16*) (data + 2);

        if (bigEndian)
        {
            for (int i = 0; i < numChars; ++i)
                builder.write ((juce_wchar) ByteOrder::swapIfLittleEndian (src[i]));
        }
        else
        {
            for (int i = 0; i < numChars; ++i)
                builder.write ((juce_wchar) ByteOrder::swapIfBigEndian (src[i]));
        }

        builder.write (0);
        return builder.result;
    }

    const uint8* start = data;
    const uint8* end = data + size;

    if (size >= 3
          && data[0] == (uint8) CharPointer_UTF8::byteOrderMark1
          && data[1] == (uint8) CharPointer_UTF8::byteOrderMark2
          && data[2] == (uint8) CharPointer_UTF8::byteOrderMark3)
        start += 3;

    return String (CharPointer_UTF8 ((const char*) start),
                   CharPointer_UTF8 ((const char*) end));
}

//==============================================================================
static const juce_wchar emptyChar = 0;

template <class CharPointerType_Src, class CharPointerType_Dest>
struct StringEncodingConverter
{
    static CharPointerType_Dest convert (const String& s)
    {
        String& source = const_cast <String&> (s);

        typedef typename CharPointerType_Dest::CharType DestChar;

        if (source.isEmpty())
            return CharPointerType_Dest (reinterpret_cast <const DestChar*> (&emptyChar));

        CharPointerType_Src text (source.getCharPointer());
        const size_t extraBytesNeeded = CharPointerType_Dest::getBytesRequiredFor (text);
        const size_t endOffset = (text.sizeInBytes() + 3) & ~3u; // the new string must be word-aligned or many Windows
                                                                // functions will fail to read it correctly!
        source.preallocateBytes (endOffset + extraBytesNeeded);
        text = source.getCharPointer();

        void* const newSpace = addBytesToPointer (text.getAddress(), (int) endOffset);
        const CharPointerType_Dest extraSpace (static_cast <DestChar*> (newSpace));

       #if JUCE_DEBUG // (This just avoids spurious warnings from valgrind about the uninitialised bytes at the end of the buffer..)
        const size_t bytesToClear = (size_t) jmin ((int) extraBytesNeeded, 4);
        zeromem (addBytesToPointer (newSpace, extraBytesNeeded - bytesToClear), bytesToClear);
       #endif

        CharPointerType_Dest (extraSpace).writeAll (text);
        return extraSpace;
    }
};

template <>
struct StringEncodingConverter <CharPointer_UTF8, CharPointer_UTF8>
{
    static CharPointer_UTF8 convert (const String& source) noexcept   { return CharPointer_UTF8 ((CharPointer_UTF8::CharType*) source.getCharPointer().getAddress()); }
};

template <>
struct StringEncodingConverter <CharPointer_UTF16, CharPointer_UTF16>
{
    static CharPointer_UTF16 convert (const String& source) noexcept  { return CharPointer_UTF16 ((CharPointer_UTF16::CharType*) source.getCharPointer().getAddress()); }
};

template <>
struct StringEncodingConverter <CharPointer_UTF32, CharPointer_UTF32>
{
    static CharPointer_UTF32 convert (const String& source) noexcept  { return CharPointer_UTF32 ((CharPointer_UTF32::CharType*) source.getCharPointer().getAddress()); }
};

CharPointer_UTF8  String::toUTF8()  const { return StringEncodingConverter <CharPointerType, CharPointer_UTF8 >::convert (*this); }
CharPointer_UTF16 String::toUTF16() const { return StringEncodingConverter <CharPointerType, CharPointer_UTF16>::convert (*this); }
CharPointer_UTF32 String::toUTF32() const { return StringEncodingConverter <CharPointerType, CharPointer_UTF32>::convert (*this); }

const wchar_t* String::toWideCharPointer() const
{
    return StringEncodingConverter <CharPointerType, CharPointer_wchar_t>::convert (*this).getAddress();
}

//==============================================================================
template <class CharPointerType_Src, class CharPointerType_Dest>
struct StringCopier
{
    static size_t copyToBuffer (const CharPointerType_Src& source, typename CharPointerType_Dest::CharType* const buffer, const size_t maxBufferSizeBytes)
    {
        jassert (((ssize_t) maxBufferSizeBytes) >= 0); // keep this value positive!

        if (buffer == nullptr)
            return CharPointerType_Dest::getBytesRequiredFor (source) + sizeof (typename CharPointerType_Dest::CharType);

        return CharPointerType_Dest (buffer).writeWithDestByteLimit (source, maxBufferSizeBytes);
    }
};

size_t String::copyToUTF8 (CharPointer_UTF8::CharType* const buffer, size_t maxBufferSizeBytes) const noexcept
{
    return StringCopier <CharPointerType, CharPointer_UTF8>::copyToBuffer (text, buffer, maxBufferSizeBytes);
}

size_t String::copyToUTF16 (CharPointer_UTF16::CharType* const buffer, size_t maxBufferSizeBytes) const noexcept
{
    return StringCopier <CharPointerType, CharPointer_UTF16>::copyToBuffer (text, buffer, maxBufferSizeBytes);
}

size_t String::copyToUTF32 (CharPointer_UTF32::CharType* const buffer, size_t maxBufferSizeBytes) const noexcept
{
    return StringCopier <CharPointerType, CharPointer_UTF32>::copyToBuffer (text, buffer, maxBufferSizeBytes);
}

//==============================================================================
size_t String::getNumBytesAsUTF8() const noexcept
{
    return CharPointer_UTF8::getBytesRequiredFor (text);
}

String String::fromUTF8 (const char* const buffer, int bufferSizeBytes)
{
    if (buffer != nullptr)
    {
        if (bufferSizeBytes < 0) return String (CharPointer_UTF8 (buffer));
        if (bufferSizeBytes > 0) return String (CharPointer_UTF8 (buffer),
                                                CharPointer_UTF8 (buffer + bufferSizeBytes));
    }

    return String::empty;
}

#if JUCE_MSVC
 #pragma warning (pop)
#endif

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

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

            test.expect (CharPointerType::isValidString (buffer, (int) strlen ((const char*) buffer)));
        }
    };

    static String createRandomWideCharString()
    {
        juce_wchar buffer[50] = { 0 };
        Random r;

        for (int i = 0; i < numElementsInArray (buffer) - 1; ++i)
        {
            if (r.nextBool())
            {
                do
                {
                    buffer[i] = (juce_wchar) (1 + r.nextInt (0x10ffff - 1));
                }
                while (! CharPointer_UTF16::canRepresent (buffer[i]));
            }
            else
                buffer[i] = (juce_wchar) (1 + r.nextInt (0xff));
        }

        return CharPointer_UTF32 (buffer);
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
            expectEquals (s1.indexOf (String::empty), 0);
            expectEquals (s1.indexOfIgnoreCase (String::empty), 0);
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
            expect (! s5.containsWholeWordIgnoreCase ("xWord2"));
            expect (s5.containsNonWhitespaceChars());
            expect (s5.containsOnly ("ordw23 "));
            expect (! String (" \n\r\t").containsNonWhitespaceChars());

            expect (s5.matchesWildcard (L"wor*", false));
            expect (s5.matchesWildcard ("wOr*", true));
            expect (s5.matchesWildcard (L"*word3", true));
            expect (s5.matchesWildcard ("*word?", true));
            expect (s5.matchesWildcard (L"Word*3", true));
            expect (! s5.matchesWildcard (L"*34", true));
            expect (String ("xx**y").matchesWildcard ("*y", true));
            expect (String ("xx**y").matchesWildcard ("x*y", true));
            expect (String ("xx**y").matchesWildcard ("xx*y", true));
            expect (String ("xx**y").matchesWildcard ("xx*", true));
            expect (String ("xx?y").matchesWildcard ("x??y", true));
            expect (String ("xx?y").matchesWildcard ("xx?y", true));
            expect (! String ("xx?y").matchesWildcard ("xx?y?", true));
            expect (String ("xx?y").matchesWildcard ("xx??", true));

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
            expect (String ("word").initialSectionContainingOnly ("word") == L"word");
            expectEquals (s5.initialSectionNotContaining (String ("xyz ")), String ("word"));
            expectEquals (s5.initialSectionNotContaining (String (";[:'/")), s5);
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
            s.addTokens ("4,3,2,1,0", ";,", "x");
            expectEquals (s.size(), 5);

            expectEquals (s.joinIntoString ("-"), String ("4-3-2-1-0"));
            s.remove (2);
            expectEquals (s.joinIntoString ("--"), String ("4--3--1--0"));
            expectEquals (s.joinIntoString (String::empty), String ("4310"));
            s.clear();
            expectEquals (s.joinIntoString ("x"), String::empty);

            StringArray toks;
            toks.addTokens ("x,,", ";,", "");
            expectEquals (toks.size(), 3);
            expectEquals (toks.joinIntoString ("-"), String ("x--"));
            toks.clear();

            toks.addTokens (",x,", ";,", "");
            expectEquals (toks.size(), 3);
            expectEquals (toks.joinIntoString ("-"), String ("-x-"));
            toks.clear();

            toks.addTokens ("x,'y,z',", ";,", "'");
            expectEquals (toks.size(), 3);
            expectEquals (toks.joinIntoString ("-"), String ("x-'y,z'-"));
        }
    }
};

static StringTests stringUnitTests;

#endif
