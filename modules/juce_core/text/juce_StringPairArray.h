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

#ifndef __JUCE_STRINGPAIRARRAY_JUCEHEADER__
#define __JUCE_STRINGPAIRARRAY_JUCEHEADER__

#include "juce_StringArray.h"


//==============================================================================
/**
    A container for holding a set of strings which are keyed by another string.

    @see StringArray
*/
class JUCE_API  StringPairArray
{
public:
    //==============================================================================
    /** Creates an empty array */
    StringPairArray (bool ignoreCaseWhenComparingKeys = true);

    /** Creates a copy of another array */
    StringPairArray (const StringPairArray& other);

    /** Destructor. */
    ~StringPairArray();

    /** Copies the contents of another string array into this one */
    StringPairArray& operator= (const StringPairArray& other);

    //==============================================================================
    /** Compares two arrays.
        Comparisons are case-sensitive.
        @returns    true only if the other array contains exactly the same strings with the same keys
    */
    bool operator== (const StringPairArray& other) const;

    /** Compares two arrays.
        Comparisons are case-sensitive.
        @returns    false if the other array contains exactly the same strings with the same keys
    */
    bool operator!= (const StringPairArray& other) const;

    //==============================================================================
    /** Finds the value corresponding to a key string.

        If no such key is found, this will just return an empty string. To check whether
        a given key actually exists (because it might actually be paired with an empty string), use
        the getAllKeys() method to obtain a list.

        Obviously the reference returned shouldn't be stored for later use, as the
        string it refers to may disappear when the array changes.

        @see getValue
    */
    const String& operator[] (const String& key) const;

    /** Finds the value corresponding to a key string.

        If no such key is found, this will just return the value provided as a default.

        @see operator[]
    */
    String getValue (const String& key, const String& defaultReturnValue) const;


    /** Returns a list of all keys in the array. */
    const StringArray& getAllKeys() const noexcept          { return keys; }

    /** Returns a list of all values in the array. */
    const StringArray& getAllValues() const noexcept        { return values; }

    /** Returns the number of strings in the array */
    inline int size() const noexcept                        { return keys.size(); };


    //==============================================================================
    /** Adds or amends a key/value pair.

        If a value already exists with this key, its value will be overwritten,
        otherwise the key/value pair will be added to the array.
    */
    void set (const String& key, const String& value);

    /** Adds the items from another array to this one.

        This is equivalent to using set() to add each of the pairs from the other array.
    */
    void addArray (const StringPairArray& other);

    //==============================================================================
    /** Removes all elements from the array. */
    void clear();

    /** Removes a string from the array based on its key.

        If the key isn't found, nothing will happen.
    */
    void remove (const String& key);

    /** Removes a string from the array based on its index.

        If the index is out-of-range, no action will be taken.
    */
    void remove (int index);

    //==============================================================================
    /** Indicates whether to use a case-insensitive search when looking up a key string.
    */
    void setIgnoresCase (bool shouldIgnoreCase);

    //==============================================================================
    /** Returns a descriptive string containing the items.
        This is handy for dumping the contents of an array.
    */
    String getDescription() const;

    //==============================================================================
    /** Reduces the amount of storage being used by the array.

        Arrays typically allocate slightly more storage than they need, and after
        removing elements, they may have quite a lot of unused space allocated.
        This method will reduce the amount of allocated storage to a minimum.
    */
    void minimiseStorageOverheads();


private:
    //==============================================================================
    StringArray keys, values;
    bool ignoreCase;

    JUCE_LEAK_DETECTOR (StringPairArray)
};


#endif   // __JUCE_STRINGPAIRARRAY_JUCEHEADER__
