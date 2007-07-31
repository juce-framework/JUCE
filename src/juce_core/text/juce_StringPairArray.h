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
    StringPairArray (const bool ignoreCaseWhenComparingKeys = true) throw();

    /** Creates a copy of another array */
    StringPairArray (const StringPairArray& other) throw();

    /** Destructor. */
    ~StringPairArray() throw();

    /** Copies the contents of another string array into this one */
    const StringPairArray& operator= (const StringPairArray& other) throw();

    //==============================================================================
    /** Compares two arrays.

        Comparisons are case-sensitive.

        @returns    true only if the other array contains exactly the same strings with the same keys
    */
    bool operator== (const StringPairArray& other) const throw();

    /** Compares two arrays.

        Comparisons are case-sensitive.

        @returns    false if the other array contains exactly the same strings with the same keys
    */
    bool operator!= (const StringPairArray& other) const throw();

    //==============================================================================
    /** Finds the value corresponding to a key string.

        If no such key is found, this will just return an empty string. To check whether
        a given key actually exists (because it might actually be paired with an empty string), use
        the getAllKeys() method to obtain a list.

        Obviously the reference returned shouldn't be stored for later use, as the
        string it refers to may disappear when the array changes.

        @see getValue
    */
    const String& operator[] (const String& key) const throw();

    /** Finds the value corresponding to a key string.

        If no such key is found, this will just return the value provided as a default.

        @see operator[]
    */
    const String getValue (const String& key, const String& defaultReturnValue) const;


    /** Returns a list of all keys in the array. */
    const StringArray& getAllKeys() const throw()       { return keys; }

    /** Returns a list of all values in the array. */
    const StringArray& getAllValues() const throw()     { return values; }

    /** Returns the number of strings in the array */
    inline int size() const throw()                     { return keys.size(); };


    //==============================================================================
    /** Adds or amends a key/value pair.

        If a value already exists with this key, its value will be overwritten,
        otherwise the key/value pair will be added to the array.
    */
    void set (const String& key,
              const String& value) throw();

    /** Adds the items from another array to this one.

        This is equivalent to using set() to add each of the pairs from the other array.
    */
    void addArray (const StringPairArray& other);

    //==============================================================================
    /** Removes all elements from the array. */
    void clear() throw();

    /** Removes a string from the array based on its key.

        If the key isn't found, nothing will happen.
    */
    void remove (const String& key) throw();

    /** Removes a string from the array based on its index.

        If the index is out-of-range, no action will be taken.
    */
    void remove (const int index) throw();

    //==============================================================================
    /** Reduces the amount of storage being used by the array.

        Arrays typically allocate slightly more storage than they need, and after
        removing elements, they may have quite a lot of unused space allocated.
        This method will reduce the amount of allocated storage to a minimum.
    */
    void minimiseStorageOverheads() throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    StringArray keys, values;
    bool ignoreCase;
};


#endif   // __JUCE_STRINGPAIRARRAY_JUCEHEADER__
