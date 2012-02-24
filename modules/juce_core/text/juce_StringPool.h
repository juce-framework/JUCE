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

#ifndef __JUCE_STRINGPOOL_JUCEHEADER__
#define __JUCE_STRINGPOOL_JUCEHEADER__

#include "juce_String.h"
#include "../containers/juce_Array.h"


//==============================================================================
/**
    A StringPool holds a set of shared strings, which reduces storage overheads and improves
    comparison speed when dealing with many duplicate strings.

    When you add a string to a pool using getPooledString, it'll return a character
    array containing the same string. This array is owned by the pool, and the same array
    is returned every time a matching string is asked for. This means that it's trivial to
    compare two pooled strings for equality, as you can simply compare their pointers. It
    also cuts down on storage if you're using many copies of the same string.
*/
class JUCE_API  StringPool
{
public:
    //==============================================================================
    /** Creates an empty pool. */
    StringPool() noexcept;

    /** Destructor */
    ~StringPool();

    //==============================================================================
    /** Returns a pointer to a copy of the string that is passed in.

        The pool will always return the same pointer when asked for a string that matches it.
        The pool will own all the pointers that it returns, deleting them when the pool itself
        is deleted.
    */
    String::CharPointerType getPooledString (const String& original);

    /** Returns a pointer to a copy of the string that is passed in.

        The pool will always return the same pointer when asked for a string that matches it.
        The pool will own all the pointers that it returns, deleting them when the pool itself
        is deleted.
    */
    String::CharPointerType getPooledString (const char* original);

    /** Returns a pointer to a copy of the string that is passed in.

        The pool will always return the same pointer when asked for a string that matches it.
        The pool will own all the pointers that it returns, deleting them when the pool itself
        is deleted.
    */
    String::CharPointerType getPooledString (const wchar_t* original);

    //==============================================================================
    /** Returns the number of strings in the pool. */
    int size() const noexcept;

    /** Returns one of the strings in the pool, by index. */
    String::CharPointerType operator[] (int index) const noexcept;

private:
    Array <String> strings;
    CriticalSection lock;
};


#endif   // __JUCE_STRINGPOOL_JUCEHEADER__
