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

#ifndef __JUCE_SORTEDSET_JUCEHEADER__
#define __JUCE_SORTEDSET_JUCEHEADER__

#include "juce_ArrayAllocationBase.h"
#include "../threads/juce_CriticalSection.h"

#if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable: 4512)
#endif


//==============================================================================
/**
    Holds a set of unique primitive objects, such as ints or doubles.

    A set can only hold one item with a given value, so if for example it's a
    set of integers, attempting to add the same integer twice will do nothing
    the second time.

    Internally, the list of items is kept sorted (which means that whatever
    kind of primitive type is used must support the ==, <, >, <= and >= operators
    to determine the order), and searching the set for known values is very fast
    because it uses a binary-chop method.

    Note that if you're using a class or struct as the element type, it must be
    capable of being copied or moved with a straightforward memcpy, rather than
    needing construction and destruction code.

    To make all the set's methods thread-safe, pass in "CriticalSection" as the templated
    TypeOfCriticalSectionToUse parameter, instead of the default DummyCriticalSection.

    @see Array, OwnedArray, ReferenceCountedArray, StringArray, CriticalSection
*/
template <class ElementType, class TypeOfCriticalSectionToUse = DummyCriticalSection>
class SortedSet
{
public:
    //==============================================================================
    /** Creates an empty set. */
    SortedSet() noexcept
       : numUsed (0)
    {
    }

    /** Creates a copy of another set.
        @param other    the set to copy
    */
    SortedSet (const SortedSet& other) noexcept
    {
        const ScopedLockType lock (other.getLock());
        numUsed = other.numUsed;
        data.setAllocatedSize (other.numUsed);
        memcpy (data.elements, other.data.elements, numUsed * sizeof (ElementType));
    }

    /** Destructor. */
    ~SortedSet() noexcept
    {
    }

    /** Copies another set over this one.
        @param other    the set to copy
    */
    SortedSet& operator= (const SortedSet& other) noexcept
    {
        if (this != &other)
        {
            const ScopedLockType lock1 (other.getLock());
            const ScopedLockType lock2 (getLock());

            data.ensureAllocatedSize (other.size());
            numUsed = other.numUsed;
            memcpy (data.elements, other.data.elements, numUsed * sizeof (ElementType));
            minimiseStorageOverheads();
        }

        return *this;
    }

    //==============================================================================
    /** Compares this set to another one.

        Two sets are considered equal if they both contain the same set of
        elements.

        @param other    the other set to compare with
    */
    bool operator== (const SortedSet<ElementType>& other) const noexcept
    {
        const ScopedLockType lock (getLock());

        if (numUsed != other.numUsed)
            return false;

        for (int i = numUsed; --i >= 0;)
            if (! (data.elements[i] == other.data.elements[i]))
                return false;

        return true;
    }

    /** Compares this set to another one.

        Two sets are considered equal if they both contain the same set of
        elements.

        @param other    the other set to compare with
    */
    bool operator!= (const SortedSet<ElementType>& other) const noexcept
    {
        return ! operator== (other);
    }

    //==============================================================================
    /** Removes all elements from the set.

        This will remove all the elements, and free any storage that the set is
        using. To clear it without freeing the storage, use the clearQuick()
        method instead.

        @see clearQuick
    */
    void clear() noexcept
    {
        const ScopedLockType lock (getLock());
        data.setAllocatedSize (0);
        numUsed = 0;
    }

    /** Removes all elements from the set without freeing the array's allocated storage.

        @see clear
    */
    void clearQuick() noexcept
    {
        const ScopedLockType lock (getLock());
        numUsed = 0;
    }

    //==============================================================================
    /** Returns the current number of elements in the set.
    */
    inline int size() const noexcept
    {
        return numUsed;
    }

    /** Returns one of the elements in the set.

        If the index passed in is beyond the range of valid elements, this
        will return zero.

        If you're certain that the index will always be a valid element, you
        can call getUnchecked() instead, which is faster.

        @param index    the index of the element being requested (0 is the first element in the set)
        @see getUnchecked, getFirst, getLast
    */
    inline ElementType operator[] (const int index) const noexcept
    {
        const ScopedLockType lock (getLock());
        return isPositiveAndBelow (index, numUsed) ? data.elements [index]
                                                   : ElementType();
    }

    /** Returns one of the elements in the set, without checking the index passed in.
        Unlike the operator[] method, this will try to return an element without
        checking that the index is within the bounds of the set, so should only
        be used when you're confident that it will always be a valid index.

        @param index    the index of the element being requested (0 is the first element in the set)
        @see operator[], getFirst, getLast
    */
    inline ElementType getUnchecked (const int index) const noexcept
    {
        const ScopedLockType lock (getLock());
        jassert (isPositiveAndBelow (index, numUsed));
        return data.elements [index];
    }

    /** Returns a direct reference to one of the elements in the set, without checking the index passed in.

        This is like getUnchecked, but returns a direct reference to the element, so that
        you can alter it directly. Obviously this can be dangerous, so only use it when
        absolutely necessary.

        @param index    the index of the element being requested (0 is the first element in the array)
    */
    inline ElementType& getReference (const int index) const noexcept
    {
        const ScopedLockType lock (getLock());
        jassert (isPositiveAndBelow (index, numUsed));
        return data.elements [index];
    }

    /** Returns the first element in the set, or 0 if the set is empty.

        @see operator[], getUnchecked, getLast
    */
    inline ElementType getFirst() const noexcept
    {
        const ScopedLockType lock (getLock());
        return numUsed > 0 ? data.elements [0] : ElementType();
    }

    /** Returns the last element in the set, or 0 if the set is empty.

        @see operator[], getUnchecked, getFirst
    */
    inline ElementType getLast() const noexcept
    {
        const ScopedLockType lock (getLock());
        return numUsed > 0 ? data.elements [numUsed - 1] : ElementType();
    }

    //==============================================================================
    /** Returns a pointer to the first element in the set.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    inline ElementType* begin() const noexcept
    {
        return data.elements;
    }

    /** Returns a pointer to the element which follows the last element in the set.
        This method is provided for compatibility with standard C++ iteration mechanisms.
    */
    inline ElementType* end() const noexcept
    {
        return data.elements + numUsed;
    }

    //==============================================================================
    /** Finds the index of the first element which matches the value passed in.

        This will search the set for the given object, and return the index
        of its first occurrence. If the object isn't found, the method will return -1.

        @param elementToLookFor   the value or object to look for
        @returns                  the index of the object, or -1 if it's not found
    */
    int indexOf (const ElementType elementToLookFor) const noexcept
    {
        const ScopedLockType lock (getLock());

        int start = 0;
        int end_ = numUsed;

        for (;;)
        {
            if (start >= end_)
            {
                return -1;
            }
            else if (elementToLookFor == data.elements [start])
            {
                return start;
            }
            else
            {
                const int halfway = (start + end_) >> 1;

                if (halfway == start)
                    return -1;
                else if (elementToLookFor < data.elements [halfway])
                    end_ = halfway;
                else
                    start = halfway;
            }
        }
    }

    /** Returns true if the set contains at least one occurrence of an object.

        @param elementToLookFor     the value or object to look for
        @returns                    true if the item is found
    */
    bool contains (const ElementType elementToLookFor) const noexcept
    {
        const ScopedLockType lock (getLock());

        int start = 0;
        int end_ = numUsed;

        for (;;)
        {
            if (start >= end_)
            {
                return false;
            }
            else if (elementToLookFor == data.elements [start])
            {
                return true;
            }
            else
            {
                const int halfway = (start + end_) >> 1;

                if (halfway == start)
                    return false;
                else if (elementToLookFor < data.elements [halfway])
                    end_ = halfway;
                else
                    start = halfway;
            }
        }
    }

    //==============================================================================
    /** Adds a new element to the set, (as long as it's not already in there).

        @param newElement       the new object to add to the set
        @see set, insert, addIfNotAlreadyThere, addSorted, addSet, addArray
    */
    void add (const ElementType newElement) noexcept
    {
        const ScopedLockType lock (getLock());

        int start = 0;
        int end_ = numUsed;

        for (;;)
        {
            if (start >= end_)
            {
                jassert (start <= end_);
                insertInternal (start, newElement);
                break;
            }
            else if (newElement == data.elements [start])
            {
                break;
            }
            else
            {
                const int halfway = (start + end_) >> 1;

                if (halfway == start)
                {
                    if (newElement < data.elements [halfway])
                        insertInternal (start, newElement);
                    else
                        insertInternal (start + 1, newElement);

                    break;
                }
                else if (newElement < data.elements [halfway])
                    end_ = halfway;
                else
                    start = halfway;
            }
        }
    }

    /** Adds elements from an array to this set.

        @param elementsToAdd        the array of elements to add
        @param numElementsToAdd     how many elements are in this other array
        @see add
    */
    void addArray (const ElementType* elementsToAdd,
                   int numElementsToAdd) noexcept
    {
        const ScopedLockType lock (getLock());

        while (--numElementsToAdd >= 0)
            add (*elementsToAdd++);
    }

    /** Adds elements from another set to this one.

        @param setToAddFrom         the set from which to copy the elements
        @param startIndex           the first element of the other set to start copying from
        @param numElementsToAdd     how many elements to add from the other set. If this
                                    value is negative or greater than the number of available elements,
                                    all available elements will be copied.
        @see add
    */
    template <class OtherSetType>
    void addSet (const OtherSetType& setToAddFrom,
                 int startIndex = 0,
                 int numElementsToAdd = -1) noexcept
    {
        const typename OtherSetType::ScopedLockType lock1 (setToAddFrom.getLock());

        {
            const ScopedLockType lock2 (getLock());
            jassert (this != &setToAddFrom);

            if (this != &setToAddFrom)
            {
                if (startIndex < 0)
                {
                    jassertfalse;
                    startIndex = 0;
                }

                if (numElementsToAdd < 0 || startIndex + numElementsToAdd > setToAddFrom.size())
                    numElementsToAdd = setToAddFrom.size() - startIndex;

                addArray (setToAddFrom.elements + startIndex, numElementsToAdd);
            }
        }
    }


    //==============================================================================
    /** Removes an element from the set.

        This will remove the element at a given index.
        If the index passed in is out-of-range, nothing will happen.

        @param indexToRemove    the index of the element to remove
        @returns                the element that has been removed
        @see removeValue, removeRange
    */
    ElementType remove (const int indexToRemove) noexcept
    {
        const ScopedLockType lock (getLock());

        if (isPositiveAndBelow (indexToRemove, numUsed))
        {
            --numUsed;

            ElementType* const e = data.elements + indexToRemove;
            ElementType const removed = *e;
            const int numberToShift = numUsed - indexToRemove;

            if (numberToShift > 0)
                memmove (e, e + 1, numberToShift * sizeof (ElementType));

            if ((numUsed << 1) < data.numAllocated)
                minimiseStorageOverheads();

            return removed;
        }

        return ElementType();
    }

    /** Removes an item from the set.

        This will remove the given element from the set, if it's there.

        @param valueToRemove   the object to try to remove
        @see remove, removeRange
    */
    void removeValue (const ElementType valueToRemove) noexcept
    {
        const ScopedLockType lock (getLock());
        remove (indexOf (valueToRemove));
    }

    /** Removes any elements which are also in another set.

        @param otherSet   the other set in which to look for elements to remove
        @see removeValuesNotIn, remove, removeValue, removeRange
    */
    template <class OtherSetType>
    void removeValuesIn (const OtherSetType& otherSet) noexcept
    {
        const typename OtherSetType::ScopedLockType lock1 (otherSet.getLock());
        const ScopedLockType lock2 (getLock());

        if (this == &otherSet)
        {
            clear();
        }
        else
        {
            if (otherSet.size() > 0)
            {
                for (int i = numUsed; --i >= 0;)
                    if (otherSet.contains (data.elements [i]))
                        remove (i);
            }
        }
    }

    /** Removes any elements which are not found in another set.

        Only elements which occur in this other set will be retained.

        @param otherSet    the set in which to look for elements NOT to remove
        @see removeValuesIn, remove, removeValue, removeRange
    */
    template <class OtherSetType>
    void removeValuesNotIn (const OtherSetType& otherSet) noexcept
    {
        const typename OtherSetType::ScopedLockType lock1 (otherSet.getLock());
        const ScopedLockType lock2 (getLock());

        if (this != &otherSet)
        {
            if (otherSet.size() <= 0)
            {
                clear();
            }
            else
            {
                for (int i = numUsed; --i >= 0;)
                    if (! otherSet.contains (data.elements [i]))
                        remove (i);
            }
        }
    }

    //==============================================================================
    /** Reduces the amount of storage being used by the set.

        Sets typically allocate slightly more storage than they need, and after
        removing elements, they may have quite a lot of unused space allocated.
        This method will reduce the amount of allocated storage to a minimum.
    */
    void minimiseStorageOverheads() noexcept
    {
        const ScopedLockType lock (getLock());
        data.shrinkToNoMoreThan (numUsed);
    }

    /** Increases the set's internal storage to hold a minimum number of elements.

        Calling this before adding a large known number of elements means that
        the set won't have to keep dynamically resizing itself as the elements
        are added, and it'll therefore be more efficient.
    */
    void ensureStorageAllocated (const int minNumElements)
    {
        const ScopedLockType lock (getLock());
        data.ensureAllocatedSize (minNumElements);
    }

    //==============================================================================
    /** Returns the CriticalSection that locks this array.
        To lock, you can call getLock().enter() and getLock().exit(), or preferably use
        an object of ScopedLockType as an RAII lock for it.
    */
    inline const TypeOfCriticalSectionToUse& getLock() const noexcept      { return data; }

    /** Returns the type of scoped lock to use for locking this array */
    typedef typename TypeOfCriticalSectionToUse::ScopedLockType ScopedLockType;


private:
    //==============================================================================
    ArrayAllocationBase <ElementType, TypeOfCriticalSectionToUse> data;
    int numUsed;

    void insertInternal (const int indexToInsertAt, const ElementType newElement) noexcept
    {
        data.ensureAllocatedSize (numUsed + 1);

        ElementType* const insertPos = data.elements + indexToInsertAt;
        const int numberToMove = numUsed - indexToInsertAt;

        if (numberToMove > 0)
            memmove (insertPos + 1, insertPos, numberToMove * sizeof (ElementType));

        *insertPos = newElement;
        ++numUsed;
    }
};

#if JUCE_MSVC
  #pragma warning (pop)
#endif

#endif   // __JUCE_SORTEDSET_JUCEHEADER__
