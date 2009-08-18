/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_SPARSESET_JUCEHEADER__
#define __JUCE_SPARSESET_JUCEHEADER__

#include "juce_ArrayAllocationBase.h"
#include "../threads/juce_CriticalSection.h"


//==============================================================================
/**
    Holds a set of primitive values, storing them as a set of ranges.

    This container acts like a simple BitArray, but can efficiently hold large
    continguous ranges of values. It's quite a specialised class, mostly useful
    for things like keeping the set of selected rows in a listbox.

    The type used as a template paramter must be an integer type, such as int, short,
    int64, etc.
*/
template <class Type>
class SparseSet
{
public:
    //==============================================================================
    /** Creates a new empty set. */
    SparseSet() throw()
    {
    }

    /** Creates a copy of another SparseSet. */
    SparseSet (const SparseSet<Type>& other) throw()
        : values (other.values)
    {
    }

    /** Destructor. */
    ~SparseSet() throw()
    {
    }

    //==============================================================================
    /** Clears the set. */
    void clear() throw()
    {
        values.clear();
    }

    /** Checks whether the set is empty.

        This is much quicker than using (size() == 0).
    */
    bool isEmpty() const throw()
    {
        return values.size() == 0;
    }

    /** Returns the number of values in the set.

        Because of the way the data is stored, this method can take longer if there
        are a lot of items in the set. Use isEmpty() for a quick test of whether there
        are any items.
    */
    Type size() const throw()
    {
        Type num = 0;

        for (int i = 0; i < values.size(); i += 2)
            num += values[i + 1] - values[i];

        return num;
    }

    /** Returns one of the values in the set.

        @param index    the index of the value to retrieve, in the range 0 to (size() - 1).
        @returns        the value at this index, or 0 if it's out-of-range
    */
    Type operator[] (int index) const throw()
    {
        for (int i = 0; i < values.size(); i += 2)
        {
            const Type s = values.getUnchecked(i);
            const Type e = values.getUnchecked(i + 1);

            if (index < e - s)
                return s + index;

            index -= e - s;
        }

        return (Type) 0;
    }

    /** Checks whether a particular value is in the set. */
    bool contains (const Type valueToLookFor) const throw()
    {
        bool on = false;

        for (int i = 0; i < values.size(); ++i)
        {
            if (values.getUnchecked(i) > valueToLookFor)
                return on;

            on = ! on;
        }

        return false;
    }

    //==============================================================================
    /** Returns the number of contiguous blocks of values.

        @see getRange
    */
    int getNumRanges() const throw()
    {
        return values.size() >> 1;
    }

    /** Returns one of the contiguous ranges of values stored.

        @param rangeIndex   the index of the range to look up, between 0
                            and (getNumRanges() - 1)
        @param startValue   on return, the value at the start of the range
        @param numValues    on return, the number of values in the range

        @see getTotalRange
    */
    bool getRange (const int rangeIndex,
                   Type& startValue,
                   Type& numValues) const throw()
    {
        if (((unsigned int) rangeIndex) < (unsigned int) getNumRanges())
        {
            startValue = values [rangeIndex << 1];
            numValues = values [(rangeIndex << 1) + 1] - startValue;

            return true;
        }

        return false;
    }

    /** Returns the lowest and highest values in the set.

        @see getRange
    */
    bool getTotalRange (Type& lowestValue,
                        Type& highestValue) const throw()
    {
        if (values.size() > 0)
        {
            lowestValue = values.getUnchecked (0);
            highestValue = values.getUnchecked (values.size() - 1);
            return true;
        }

        return false;
    }

    //==============================================================================
    /** Adds a range of contiguous values to the set.

        e.g. addRange (10, 4) will add (10, 11, 12, 13) to the set.

        @param firstValue       the start of the range of values to add
        @param numValuesToAdd   how many values to add
    */
    void addRange (const Type firstValue,
                   const Type numValuesToAdd) throw()
    {
        jassert (numValuesToAdd >= 0);

        if (numValuesToAdd > 0)
        {
            removeRange (firstValue, numValuesToAdd);

            IntegerElementComparator<Type> sorter;
            values.addSorted (sorter, firstValue);
            values.addSorted (sorter, firstValue + numValuesToAdd);

            simplify();
        }
    }

    /** Removes a range of values from the set.

        e.g. removeRange (10, 4) will remove (10, 11, 12, 13) from the set.

        @param firstValue           the start of the range of values to remove
        @param numValuesToRemove    how many values to remove
    */
    void removeRange (const Type firstValue,
                      const Type numValuesToRemove) throw()
    {
        jassert (numValuesToRemove >= 0);

        if (numValuesToRemove >= 0
             && firstValue < values.getLast())
        {
            const bool onAtStart = contains (firstValue - 1);
            const Type lastValue = firstValue + jmin (numValuesToRemove, values.getLast() - firstValue);
            const bool onAtEnd = contains (lastValue);

            for (int i = values.size(); --i >= 0;)
            {
                if (values.getUnchecked(i) <= lastValue)
                {
                    while (values.getUnchecked(i) >= firstValue)
                    {
                        values.remove (i);

                        if (--i < 0)
                            break;
                    }

                    break;
                }
            }

            IntegerElementComparator<Type> sorter;

            if (onAtStart)
                values.addSorted (sorter, firstValue);

            if (onAtEnd)
                values.addSorted (sorter, lastValue);

            simplify();
        }
    }

    /** Does an XOR of the values in a given range. */
    void invertRange (const Type firstValue,
                      const Type numValues)
    {
        SparseSet newItems;
        newItems.addRange (firstValue, numValues);

        int i;
        for (i = getNumRanges(); --i >= 0;)
        {
            const int start = values [i << 1];
            const int end = values [(i << 1) + 1];

            newItems.removeRange (start, end);
        }

        removeRange (firstValue, numValues);

        for (i = newItems.getNumRanges(); --i >= 0;)
        {
            const int start = newItems.values [i << 1];
            const int end = newItems.values [(i << 1) + 1];

            addRange (start, end);
        }
    }

    /** Checks whether any part of a given range overlaps any part of this one. */
    bool overlapsRange (const Type firstValue,
                        const Type numValues) throw()
    {
        jassert (numValues >= 0);

        if (numValues > 0)
        {
            for (int i = getNumRanges(); --i >= 0;)
            {
                if (firstValue >= values.getUnchecked ((i << 1) + 1))
                    return false;

                if (firstValue + numValues > values.getUnchecked (i << 1))
                    return true;
            }
        }

        return false;
    }

    /** Checks whether the whole of a given range is contained within this one. */
    bool containsRange (const Type firstValue,
                        const Type numValues) throw()
    {
        jassert (numValues >= 0);

        if (numValues > 0)
        {
            for (int i = getNumRanges(); --i >= 0;)
            {
                if (firstValue >= values.getUnchecked ((i << 1) + 1))
                    return false;

                if (firstValue >= values.getUnchecked (i << 1)
                     && firstValue + numValues <= values.getUnchecked ((i << 1) + 1))
                    return true;
            }
        }

        return false;
    }

    //==============================================================================
    bool operator== (const SparseSet<Type>& other) throw()
    {
        return values == other.values;
    }

    bool operator!= (const SparseSet<Type>& other) throw()
    {
        return values != other.values;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    // alternating start/end values of ranges of values that are present.
    Array<Type> values;

    void simplify() throw()
    {
        jassert ((values.size() & 1) == 0);

        for (int i = values.size(); --i > 0;)
            if (values.getUnchecked(i) == values.getUnchecked (i - 1))
                values.removeRange (i - 1, 2);
    }
};



#endif   // __JUCE_SPARSESET_JUCEHEADER__
