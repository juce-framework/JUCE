/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

private:
    Array<String> strings;
    CriticalSection lock;
    uint32 lastGarbageCollectionTime;

    void garbageCollectIfNeeded();

    JUCE_DECLARE_NON_COPYABLE (StringPool)
};

} // namespace juce
