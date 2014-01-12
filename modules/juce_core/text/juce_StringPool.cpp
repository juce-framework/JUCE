/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

StringPool::StringPool() noexcept   {}
StringPool::~StringPool()           {}

namespace StringPoolHelpers
{
    template <class StringType>
    String::CharPointerType getPooledStringFromArray (Array<String>& strings,
                                                      StringType newString,
                                                      const CriticalSection& lock)
    {
        const ScopedLock sl (lock);
        int start = 0;
        int end = strings.size();

        for (;;)
        {
            if (start >= end)
            {
                jassert (start <= end);
                strings.insert (start, newString);
                return strings.getReference (start).getCharPointer();
            }

            const String& startString = strings.getReference (start);

            if (startString == newString)
                return startString.getCharPointer();

            const int halfway = (start + end) >> 1;

            if (halfway == start)
            {
                if (startString.compare (newString) < 0)
                    ++start;

                strings.insert (start, newString);
                return strings.getReference (start).getCharPointer();
            }

            const int comp = strings.getReference (halfway).compare (newString);

            if (comp == 0)
                return strings.getReference (halfway).getCharPointer();

            if (comp < 0)
                start = halfway;
            else
                end = halfway;
        }
    }
}

String::CharPointerType StringPool::getPooledString (const String& s)
{
    if (s.isEmpty())
        return String().getCharPointer();

    return StringPoolHelpers::getPooledStringFromArray (strings, s, lock);
}

String::CharPointerType StringPool::getPooledString (const char* const s)
{
    if (s == nullptr || *s == 0)
        return String().getCharPointer();

    return StringPoolHelpers::getPooledStringFromArray (strings, s, lock);
}

String::CharPointerType StringPool::getPooledString (const wchar_t* const s)
{
    if (s == nullptr || *s == 0)
        return String().getCharPointer();

    return StringPoolHelpers::getPooledStringFromArray (strings, s, lock);
}

int StringPool::size() const noexcept
{
    return strings.size();
}

String::CharPointerType StringPool::operator[] (const int index) const noexcept
{
    return strings [index].getCharPointer();
}
