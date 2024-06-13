/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class Identifier;

//==============================================================================
/**
    A StringPool holds a set of shared strings, which reduces storage overheads and improves
    comparison speed when dealing with many duplicate strings.

    When you add a string to a pool using getPooledString, it'll return a character
    array containing the same string. This array is owned by the pool, and the same array
    is returned every time a matching string is asked for. This means that it's trivial to
    compare two pooled strings for equality, as you can simply compare their pointers. It
    also cuts down on storage if you're using many copies of the same string.

    @tags{Core}
*/
class JUCE_API  StringPool
{
public:
    //==============================================================================
    /** Creates an empty pool. */
    StringPool() noexcept;

    //==============================================================================
    /** Returns a pointer to a shared copy of the string that is passed in.
        The pool will always return the same String object when asked for a string that matches it.
    */
    String getPooledString (const String& original);

    /** Returns a pointer to a copy of the string that is passed in.
        The pool will always return the same String object when asked for a string that matches it.
    */
    String getPooledString (const char* original);

    /** Returns a pointer to a shared copy of the string that is passed in.
        The pool will always return the same String object when asked for a string that matches it.
    */
    String getPooledString (StringRef original);

    /** Returns a pointer to a copy of the string that is passed in.
        The pool will always return the same String object when asked for a string that matches it.
    */
    String getPooledString (String::CharPointerType start, String::CharPointerType end);

    //==============================================================================
    /** Scans the pool, and removes any strings that are unreferenced.
        You don't generally need to call this - it'll be called automatically when the pool grows
        large enough to warrant it.
    */
    void garbageCollect();

    /** Returns a shared global pool which is used for things like Identifiers, XML parsing. */
    static StringPool& getGlobalPool() noexcept;

    //==============================================================================
    void ensureAdditionalStorageAllocated (int numStringsNeeded);

    int getCapacity() const noexcept { return strings.size(); }

    Array<Identifier> addSortedStrings(const Array<String>& stringsToAdd);


private:

    Array<String> strings;
    CriticalSection lock;
    uint32 lastGarbageCollectionTime;

    void garbageCollectIfNeeded();

    /**
     * Determine if the newString is in the pool and return the index of the string if it's the case.
     * Otherwise it returns false and gives the index at which the string should be inserted.
     * @param newString String to check
     * @param startIndex Start index for the search
     * @param endIndex End index (exclusive) for the search
     * @return Pair of bool and int. The bool is true if the string is in the pool, false otherwise. The int is the index at which the string should be inserted or the index of the string in the pool.
     */
    std::pair<bool, int> getIndex(const String& newString, int startIndex, int endIndex) const;

    JUCE_DECLARE_NON_COPYABLE (StringPool)
};

} // namespace juce
