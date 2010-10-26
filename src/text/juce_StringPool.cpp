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

BEGIN_JUCE_NAMESPACE

#include "juce_StringPool.h"


//==============================================================================
StringPool::StringPool() throw()    {}
StringPool::~StringPool()           {}

namespace StringPoolHelpers
{
    template <class StringType>
    const juce_wchar* getPooledStringFromArray (Array<String>& strings, StringType newString)
    {
        int start = 0;
        int end = strings.size();

        for (;;)
        {
            if (start >= end)
            {
                jassert (start <= end);
                strings.insert (start, newString);
                return strings.getReference (start);
            }
            else
            {
                const String& startString = strings.getReference (start);

                if (startString == newString)
                    return startString;

                const int halfway = (start + end) >> 1;

                if (halfway == start)
                {
                    if (startString.compare (newString) < 0)
                        ++start;

                    strings.insert (start, newString);
                    return strings.getReference (start);
                }

                const int comp = strings.getReference (halfway).compare (newString);

                if (comp == 0)
                    return strings.getReference (halfway);
                else if (comp < 0)
                    start = halfway;
                else
                    end = halfway;
            }
        }
    }
}

const juce_wchar* StringPool::getPooledString (const String& s)
{
    if (s.isEmpty())
        return String::empty;

    return StringPoolHelpers::getPooledStringFromArray (strings, s);
}

const juce_wchar* StringPool::getPooledString (const char* const s)
{
    if (s == 0 || *s == 0)
        return String::empty;

    return StringPoolHelpers::getPooledStringFromArray (strings, s);
}

const juce_wchar* StringPool::getPooledString (const juce_wchar* const s)
{
    if (s == 0 || *s == 0)
        return String::empty;

    return StringPoolHelpers::getPooledStringFromArray (strings, s);
}

int StringPool::size() const throw()
{
    return strings.size();
}

const juce_wchar* StringPool::operator[] (const int index) const throw()
{
    return strings [index];
}

END_JUCE_NAMESPACE
