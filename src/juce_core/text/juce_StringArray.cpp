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

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_StringArray.h"


//==============================================================================
StringArray::StringArray() throw()
{
}

StringArray::StringArray (const StringArray& other) throw()
{
    addArray (other);
}

StringArray::StringArray (const juce_wchar** const strings,
                          const int numberOfStrings) throw()
{
    for (int i = 0; i < numberOfStrings; ++i)
        add (strings [i]);
}

StringArray::StringArray (const char** const strings,
                          const int numberOfStrings) throw()
{
    for (int i = 0; i < numberOfStrings; ++i)
        add (strings [i]);
}

StringArray::StringArray (const juce_wchar** const strings) throw()
{
    int i = 0;

    while (strings[i] != 0)
        add (strings [i++]);
}

StringArray::StringArray (const char** const strings) throw()
{
    int i = 0;

    while (strings[i] != 0)
        add (strings [i++]);
}

const StringArray& StringArray::operator= (const StringArray& other) throw()
{
    if (this != &other)
    {
        clear();
        addArray (other);
    }

    return *this;
}

StringArray::~StringArray() throw()
{
    clear();
}

bool StringArray::operator== (const StringArray& other) const throw()
{
    if (other.size() != size())
        return false;

    for (int i = size(); --i >= 0;)
    {
        if (*(String*) other.strings.getUnchecked(i)
               != *(String*) strings.getUnchecked(i))
        {
            return false;
        }
    }

    return true;
}

bool StringArray::operator!= (const StringArray& other) const throw()
{
    return ! operator== (other);
}

void StringArray::clear() throw()
{
    for (int i = size(); --i >= 0;)
    {
        String* const s = (String*) strings.getUnchecked(i);
        delete s;
    }

    strings.clear();
}

const String& StringArray::operator[] (const int index) const throw()
{
    if (index >= 0 && index < strings.size())
        return *(const String*) (strings.getUnchecked (index));

    return String::empty;
}

void StringArray::add (const String& newString) throw()
{
    strings.add (new String (newString));
}

void StringArray::insert (const int index,
                          const String& newString) throw()
{
    strings.insert (index, new String (newString));
}

void StringArray::addIfNotAlreadyThere (const String& newString,
                                        const bool ignoreCase) throw()
{
    if (! contains (newString, ignoreCase))
        add (newString);
}

void StringArray::addArray (const StringArray& otherArray,
                            int startIndex,
                            int numElementsToAdd) throw()
{
    if (startIndex < 0)
    {
        jassertfalse
        startIndex = 0;
    }

    if (numElementsToAdd < 0 || startIndex + numElementsToAdd > otherArray.size())
        numElementsToAdd = otherArray.size() - startIndex;

    while (--numElementsToAdd >= 0)
        strings.add (new String (*(const String*) otherArray.strings.getUnchecked (startIndex++)));
}

void StringArray::set (const int index,
                       const String& newString) throw()
{
    String* const s = (String*) strings [index];

    if (s != 0)
    {
        *s = newString;
    }
    else if (index >= 0)
    {
        add (newString);
    }
}

bool StringArray::contains (const String& stringToLookFor,
                            const bool ignoreCase) const throw()
{
    if (ignoreCase)
    {
        for (int i = size(); --i >= 0;)
            if (stringToLookFor.equalsIgnoreCase (*(const String*)(strings.getUnchecked(i))))
                return true;
    }
    else
    {
        for (int i = size(); --i >= 0;)
            if (stringToLookFor == *(const String*)(strings.getUnchecked(i)))
                return true;
    }

    return false;
}

int StringArray::indexOf (const String& stringToLookFor,
                          const bool ignoreCase,
                          int i) const throw()
{
    if (i < 0)
        i = 0;

    const int numElements = size();

    if (ignoreCase)
    {
        while (i < numElements)
        {
            if (stringToLookFor.equalsIgnoreCase (*(const String*) strings.getUnchecked (i)))
                return i;

            ++i;
        }
    }
    else
    {
        while (i < numElements)
        {
            if (stringToLookFor == *(const String*) strings.getUnchecked (i))
                return i;

            ++i;
        }
    }

    return -1;
}

//==============================================================================
void StringArray::remove (const int index) throw()
{
    String* const s = (String*) strings [index];

    if (s != 0)
    {
        strings.remove (index);
        delete s;
    }
}

void StringArray::removeString (const String& stringToRemove,
                                const bool ignoreCase) throw()
{
    if (ignoreCase)
    {
        for (int i = size(); --i >= 0;)
            if (stringToRemove.equalsIgnoreCase (*(const String*) strings.getUnchecked (i)))
                remove (i);
    }
    else
    {
        for (int i = size(); --i >= 0;)
            if (stringToRemove == *(const String*) strings.getUnchecked (i))
                remove (i);
    }
}

//==============================================================================
void StringArray::removeEmptyStrings (const bool removeWhitespaceStrings) throw()
{
    if (removeWhitespaceStrings)
    {
        for (int i = size(); --i >= 0;)
            if (((const String*) strings.getUnchecked(i))->trim().isEmpty())
                remove (i);
    }
    else
    {
        for (int i = size(); --i >= 0;)
            if (((const String*) strings.getUnchecked(i))->isEmpty())
                remove (i);
    }
}

void StringArray::trim() throw()
{
    for (int i = size(); --i >= 0;)
    {
        String& s = *(String*) strings.getUnchecked(i);
        s = s.trim();
    }
}

//==============================================================================
class InternalStringArrayComparator
{
public:
    static int compareElements (void* const first, void* const second) throw()
    {
        return ((const String*) first)->compare (*(const String*) second);
    }
};

class InsensitiveInternalStringArrayComparator
{
public:
    static int compareElements (void* const first, void* const second) throw()
    {
        return ((const String*) first)->compareIgnoreCase (*(const String*) second);
    }
};

void StringArray::sort (const bool ignoreCase) throw()
{
    if (ignoreCase)
    {
        InsensitiveInternalStringArrayComparator comp;
        strings.sort (comp);
    }
    else
    {
        InternalStringArrayComparator comp;
        strings.sort (comp);
    }
}

void StringArray::move (const int currentIndex, int newIndex) throw()
{
    strings.move (currentIndex, newIndex);
}


//==============================================================================
const String StringArray::joinIntoString (const String& separator,
                                          int start,
                                          int numberToJoin) const throw()
{
    const int last = (numberToJoin < 0) ? size()
                                        : jmin (size(), start + numberToJoin);

    if (start < 0)
        start = 0;

    if (start >= last)
        return String::empty;

    if (start == last - 1)
        return *(const String*) strings.getUnchecked (start);

    const int separatorLen = separator.length();
    int charsNeeded = separatorLen * (last - start - 1);

    for (int i = start; i < last; ++i)
        charsNeeded += ((const String*) strings.getUnchecked(i))->length();

    String result;
    result.preallocateStorage (charsNeeded);

    tchar* dest = (tchar*) (const tchar*) result;

    while (start < last)
    {
        const String& s = *(const String*) strings.getUnchecked (start);
        const int len = s.length();

        if (len > 0)
        {
            s.copyToBuffer (dest, len);
            dest += len;
        }

        if (++start < last && separatorLen > 0)
        {
            separator.copyToBuffer (dest, separatorLen);
            dest += separatorLen;
        }
    }

    *dest = 0;

    return result;
}

int StringArray::addTokens (const tchar* const text,
                            const bool preserveQuotedStrings) throw()
{
    return addTokens (text,
                      T(" \n\r\t"),
                      preserveQuotedStrings ? T("\"") : 0);
}

int StringArray::addTokens (const tchar* const text,
                            const tchar* breakCharacters,
                            const tchar* quoteCharacters) throw()
{
    int num = 0;

    if (text != 0 && *text != 0)
    {
        if (breakCharacters == 0)
            breakCharacters = T("");

        if (quoteCharacters == 0)
            quoteCharacters = T("");

        bool insideQuotes = false;
        tchar currentQuoteChar = 0;

        int i = 0;
        int tokenStart = 0;

        for (;;)
        {
            const tchar c = text[i];

            bool isBreak = (c == 0);

            if (! (insideQuotes || isBreak))
            {
                const tchar* b = breakCharacters;
                while (*b != 0)
                {
                    if (*b++ == c)
                    {
                        isBreak = true;
                        break;
                    }
                }
            }

            if (! isBreak)
            {
                bool isQuote = false;
                const tchar* q = quoteCharacters;
                while (*q != 0)
                {
                    if (*q++ == c)
                    {
                        isQuote = true;
                        break;
                    }
                }

                if (isQuote)
                {
                    if (insideQuotes)
                    {
                        // only break out of quotes-mode if we find a matching quote to the
                        // one that we opened with..
                        if (currentQuoteChar == c)
                            insideQuotes = false;
                    }
                    else
                    {
                        insideQuotes = true;
                        currentQuoteChar = c;
                    }
                }
            }
            else
            {
                add (String (text + tokenStart, i - tokenStart));

                ++num;
                tokenStart = i + 1;
            }

            if (c == 0)
                break;

            ++i;
        }
    }

    return num;
}

int StringArray::addLines (const tchar* text) throw()
{
    int numLines = 0;

    if (text != 0)
    {
        while (*text != 0)
        {
            const tchar* const startOfLine = text;

            while (*text != 0)
            {
                if (*text == T('\r'))
                {
                    ++text;
                    if (*text == T('\n'))
                        ++text;

                    break;
                }

                if (*text == T('\n'))
                {
                    ++text;
                    break;
                }

                ++text;
            }

            const tchar* endOfLine = text;
            if (endOfLine > startOfLine && (*(endOfLine - 1) == T('\r') || *(endOfLine - 1) == T('\n')))
                --endOfLine;

            if (endOfLine > startOfLine && (*(endOfLine - 1) == T('\r') || *(endOfLine - 1) == T('\n')))
                --endOfLine;

            add (String (startOfLine, jmax (0, (int) (endOfLine - startOfLine))));

            ++numLines;
        }
    }

    return numLines;
}

//==============================================================================
void StringArray::removeDuplicates (const bool ignoreCase) throw()
{
    for (int i = 0; i < size() - 1; ++i)
    {
        const String& s = *(String*) strings.getUnchecked(i);

        int nextIndex = i + 1;

        for (;;)
        {
            nextIndex = indexOf (s, ignoreCase, nextIndex);

            if (nextIndex < 0)
                break;

            remove (nextIndex);
        }
    }
}

void StringArray::appendNumbersToDuplicates (const bool ignoreCase,
                                             const bool appendNumberToFirstInstance,
                                             const tchar* const preNumberString,
                                             const tchar* const postNumberString) throw()
{
    for (int i = 0; i < size() - 1; ++i)
    {
        String& s = *(String*) strings.getUnchecked(i);

        int nextIndex = indexOf (s, ignoreCase, i + 1);

        if (nextIndex >= 0)
        {
            const String original (s);

            int number = 0;

            if (appendNumberToFirstInstance)
                s = original + preNumberString + String (++number) + postNumberString;
            else
                ++number;

            while (nextIndex >= 0)
            {
                set (nextIndex, (*this)[nextIndex] + preNumberString + String (++number) + postNumberString);
                nextIndex = indexOf (original, ignoreCase, nextIndex + 1);
            }
        }
    }
}

void StringArray::minimiseStorageOverheads() throw()
{
    strings.minimiseStorageOverheads();
}

END_JUCE_NAMESPACE
