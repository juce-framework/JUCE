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

#ifndef __JUCE_ARRAY_JUCEHEADER__
#define __JUCE_ARRAY_JUCEHEADER__

#include "juce_ArrayAllocationBase.h"
#include "juce_ElementComparator.h"
#include "../threads/juce_CriticalSection.h"


//==============================================================================
/**
    Holds a list of primitive objects, such as ints, doubles, or pointers.

    Examples of arrays are: Array<int> or Array<MyClass*>

    Note that when holding pointers to objects, the array doesn't take any ownership
    of the objects - for doing this, see the OwnedArray class or the ReferenceCountedArray class.

    If you're using a class or struct as the element type, it must be
    capable of being copied or moved with a straightforward memcpy, rather than
    needing construction and destruction code.

    For holding lists of strings, use the specialised class StringArray.

    To make all the array's methods thread-safe, pass in "CriticalSection" as the templated
    TypeOfCriticalSectionToUse parameter, instead of the default DummyCriticalSection.

    @see OwnedArray, ReferenceCountedArray, StringArray, CriticalSection
*/
template <class ElementType, class TypeOfCriticalSectionToUse = DummyCriticalSection>
class Array   : private ArrayAllocationBase <ElementType>
{
public:
    //==============================================================================
    /** Creates an empty array.

        @param granularity  this is the size of increment by which the internal storage
        used by the array will grow. Only change it from the default if you know the
        array is going to be very big and needs to be able to grow efficiently.

        @see ArrayAllocationBase
    */
    Array (const int granularity = juceDefaultArrayGranularity) throw()
       : ArrayAllocationBase <ElementType> (granularity),
         numUsed (0)
    {
    }

    /** Creates a copy of another array.
        @param other    the array to copy
    */
    Array (const Array<ElementType, TypeOfCriticalSectionToUse>& other) throw()
       : ArrayAllocationBase <ElementType> (other.granularity)
    {
        other.lockArray();
        numUsed = other.numUsed;
        this->setAllocatedSize (other.numUsed);
        memcpy (this->elements, other.elements, numUsed * sizeof (ElementType));
        other.unlockArray();
    }

    /** Initalises from a null-terminated C array of values.

        @param values   the array to copy from
    */
    Array (const ElementType* values) throw()
       : ArrayAllocationBase <ElementType> (juceDefaultArrayGranularity),
         numUsed (0)
    {
        while (*values != 0)
            add (*values++);
    }

    /** Initalises from a C array of values.

        @param values       the array to copy from
        @param numValues    the number of values in the array
    */
    Array (const ElementType* values, int numValues) throw()
       : ArrayAllocationBase <ElementType> (juceDefaultArrayGranularity),
         numUsed (numValues)
    {
        this->setAllocatedSize (numValues);
        memcpy (this->elements, values, numValues * sizeof (ElementType));
    }

    /** Destructor. */
    ~Array() throw()
    {
    }

    /** Copies another array.
        @param other    the array to copy
    */
    const Array <ElementType, TypeOfCriticalSectionToUse>& operator= (const Array <ElementType, TypeOfCriticalSectionToUse>& other) throw()
    {
        if (this != &other)
        {
            other.lockArray();
            lock.enter();

            this->granularity = other.granularity;
            this->ensureAllocatedSize (other.size());
            numUsed = other.numUsed;
            memcpy (this->elements, other.elements, this->numUsed * sizeof (ElementType));
            minimiseStorageOverheads();

            lock.exit();
            other.unlockArray();
        }

        return *this;
    }

    //==============================================================================
    /** Compares this array to another one.
        Two arrays are considered equal if they both contain the same set of
        elements, in the same order.
        @param other    the other array to compare with
    */
    template <class OtherArrayType>
    bool operator== (const OtherArrayType& other) const throw()
    {
        lock.enter();

        if (this->numUsed != other.numUsed)
        {
            lock.exit();
            return false;
        }

        for (int i = numUsed; --i >= 0;)
        {
            if (this->elements [i] != other.elements [i])
            {
                lock.exit();
                return false;
            }
        }

        lock.exit();
        return true;
    }

    /** Compares this array to another one.
        Two arrays are considered equal if they both contain the same set of
        elements, in the same order.
        @param other    the other array to compare with
    */
    template <class OtherArrayType>
    bool operator!= (const OtherArrayType& other) const throw()
    {
        return ! operator== (other);
    }

    //==============================================================================
    /** Removes all elements from the array.
        This will remove all the elements, and free any storage that the array is
        using. To clear the array without freeing the storage, use the clearQuick()
        method instead.

        @see clearQuick
    */
    void clear() throw()
    {
        lock.enter();
        this->setAllocatedSize (0);
        numUsed = 0;
        lock.exit();
    }

    /** Removes all elements from the array without freeing the array's allocated storage.

        @see clear
    */
    void clearQuick() throw()
    {
        lock.enter();
        numUsed = 0;
        lock.exit();
    }

    //==============================================================================
    /** Returns the current number of elements in the array.
    */
    inline int size() const throw()
    {
        return numUsed;
    }

    /** Returns one of the elements in the array.
        If the index passed in is beyond the range of valid elements, this
        will return zero.

        If you're certain that the index will always be a valid element, you
        can call getUnchecked() instead, which is faster.

        @param index    the index of the element being requested (0 is the first element in the array)
        @see getUnchecked, getFirst, getLast
    */
    inline ElementType operator[] (const int index) const throw()
    {
        lock.enter();
        const ElementType result = (index >= 0 && index < numUsed) ? this->elements [index]
                                                                   : ElementType();
        lock.exit();

        return result;
    }

    /** Returns one of the elements in the array, without checking the index passed in.

        Unlike the operator[] method, this will try to return an element without
        checking that the index is within the bounds of the array, so should only
        be used when you're confident that it will always be a valid index.

        @param index    the index of the element being requested (0 is the first element in the array)
        @see operator[], getFirst, getLast
    */
    inline ElementType getUnchecked (const int index) const throw()
    {
        lock.enter();
        jassert (index >= 0 && index < numUsed);
        const ElementType result = this->elements [index];
        lock.exit();

        return result;
    }

    /** Returns a direct reference to one of the elements in the array, without checking the index passed in.

        This is like getUnchecked, but returns a direct reference to the element, so that
        you can alter it directly. Obviously this can be dangerous, so only use it when
        absolutely necessary.

        @param index    the index of the element being requested (0 is the first element in the array)
        @see operator[], getFirst, getLast
    */
    inline ElementType& getReference (const int index) const throw()
    {
        jassert (index >= 0 && index < numUsed);
        return this->elements [index];
    }

    /** Returns the first element in the array, or 0 if the array is empty.

        @see operator[], getUnchecked, getLast
    */
    inline ElementType getFirst() const throw()
    {
        lock.enter();
        const ElementType result = (numUsed > 0) ? this->elements [0]
                                                 : ElementType();
        lock.exit();

        return result;
    }

    /** Returns the last element in the array, or 0 if the array is empty.

        @see operator[], getUnchecked, getFirst
    */
    inline ElementType getLast() const throw()
    {
        lock.enter();
        const ElementType result = (numUsed > 0) ? this->elements [numUsed - 1]
                                                 : ElementType();
        lock.exit();

        return result;
    }

    //==============================================================================
    /** Finds the index of the first element which matches the value passed in.

        This will search the array for the given object, and return the index
        of its first occurrence. If the object isn't found, the method will return -1.

        @param elementToLookFor   the value or object to look for
        @returns                  the index of the object, or -1 if it's not found
    */
    int indexOf (const ElementType elementToLookFor) const throw()
    {
        int result = -1;

        lock.enter();
        const ElementType* e = this->elements;

        for (int i = numUsed; --i >= 0;)
        {
            if (elementToLookFor == *e)
            {
                result = (int) (e - this->elements);
                break;
            }

            ++e;
        }

        lock.exit();
        return result;
    }

    /** Returns true if the array contains at least one occurrence of an object.

        @param elementToLookFor     the value or object to look for
        @returns                    true if the item is found
    */
    bool contains (const ElementType elementToLookFor) const throw()
    {
        lock.enter();

        const ElementType* e = this->elements;
        int num = numUsed;

        while (num >= 4)
        {
            if (*e == elementToLookFor
                 || *++e == elementToLookFor
                 || *++e == elementToLookFor
                 || *++e == elementToLookFor)
            {
                lock.exit();
                return true;
            }

            num -= 4;
            ++e;
        }

        while (num > 0)
        {
            if (elementToLookFor == *e)
            {
                lock.exit();
                return true;
            }

            --num;
            ++e;
        }

        lock.exit();
        return false;
    }

    //==============================================================================
    /** Appends a new element at the end of the array.

        @param newElement       the new object to add to the array
        @see set, insert, addIfNotAlreadyThere, addSorted, addArray
    */
    void add (const ElementType newElement) throw()
    {
        lock.enter();
        this->ensureAllocatedSize (numUsed + 1);
        this->elements [numUsed++] = newElement;
        lock.exit();
    }

    /** Inserts a new element into the array at a given position.

        If the index is less than 0 or greater than the size of the array, the
        element will be added to the end of the array.
        Otherwise, it will be inserted into the array, moving all the later elements
        along to make room.

        @param indexToInsertAt    the index at which the new element should be
                                  inserted (pass in -1 to add it to the end)
        @param newElement         the new object to add to the array
        @see add, addSorted, set
    */
    void insert (int indexToInsertAt, const ElementType newElement) throw()
    {
        lock.enter();
        this->ensureAllocatedSize (numUsed + 1);

        if (indexToInsertAt >= 0)
        {
            if (indexToInsertAt > numUsed)
                indexToInsertAt = numUsed;

            ElementType* const insertPos = this->elements + indexToInsertAt;
            const int numberToMove = numUsed - indexToInsertAt;

            if (numberToMove > 0)
                memmove (insertPos + 1, insertPos, numberToMove * sizeof (ElementType));

            *insertPos = newElement;
            ++numUsed;
        }
        else
        {
            this->elements [numUsed++] = newElement;
        }

        lock.exit();
    }

    /** Inserts multiple copies of an element into the array at a given position.

        If the index is less than 0 or greater than the size of the array, the
        element will be added to the end of the array.
        Otherwise, it will be inserted into the array, moving all the later elements
        along to make room.

        @param indexToInsertAt    the index at which the new element should be inserted
        @param newElement         the new object to add to the array
        @param numberOfTimesToInsertIt  how many copies of the value to insert
        @see insert, add, addSorted, set
    */
    void insertMultiple (int indexToInsertAt, const ElementType newElement,
                         int numberOfTimesToInsertIt) throw()
    {
        if (numberOfTimesToInsertIt > 0)
        {
            lock.enter();
            this->ensureAllocatedSize (numUsed + numberOfTimesToInsertIt);

            if (indexToInsertAt >= 0 && indexToInsertAt < numUsed)
            {
                ElementType* insertPos = this->elements + indexToInsertAt;
                const int numberToMove = numUsed - indexToInsertAt;

                memmove (insertPos + numberOfTimesToInsertIt, insertPos, numberToMove * sizeof (ElementType));
                numUsed += numberOfTimesToInsertIt;

                while (--numberOfTimesToInsertIt >= 0)
                    *insertPos++ = newElement;
            }
            else
            {
                while (--numberOfTimesToInsertIt >= 0)
                    this->elements [numUsed++] = newElement;
            }

            lock.exit();
        }
    }

    /** Inserts an array of values into this array at a given position.

        If the index is less than 0 or greater than the size of the array, the
        new elements will be added to the end of the array.
        Otherwise, they will be inserted into the array, moving all the later elements
        along to make room.

        @param indexToInsertAt      the index at which the first new element should be inserted
        @param newElements          the new values to add to the array
        @param numberOfElements     how many items are in the array
        @see insert, add, addSorted, set
    */
    void insertArray (int indexToInsertAt,
                      const ElementType* newElements,
                      int numberOfElements) throw()
    {
        if (numberOfElements > 0)
        {
            lock.enter();
            this->ensureAllocatedSize (numUsed + numberOfElements);

            if (indexToInsertAt >= 0 && indexToInsertAt < numUsed)
            {
                ElementType* insertPos = this->elements + indexToInsertAt;
                const int numberToMove = numUsed - indexToInsertAt;

                memmove (insertPos + numberOfElements, insertPos, numberToMove * sizeof (ElementType));
                numUsed += numberOfElements;

                while (--numberOfElements >= 0)
                    *insertPos++ = *newElements++;
            }
            else
            {
                while (--numberOfElements >= 0)
                    this->elements [numUsed++] = *newElements++;
            }

            lock.exit();
        }
    }

    /** Appends a new element at the end of the array as long as the array doesn't
        already contain it.

        If the array already contains an element that matches the one passed in, nothing
        will be done.

        @param newElement   the new object to add to the array
    */
    void addIfNotAlreadyThere (const ElementType newElement) throw()
    {
        lock.enter();

        if (! contains (newElement))
            add (newElement);

        lock.exit();
    }

    /** Replaces an element with a new value.

        If the index is less than zero, this method does nothing.
        If the index is beyond the end of the array, the item is added to the end of the array.

        @param indexToChange    the index whose value you want to change
        @param newValue         the new value to set for this index.
        @see add, insert
    */
    void set (const int indexToChange,
              const ElementType newValue) throw()
    {
        jassert (indexToChange >= 0);

        if (indexToChange >= 0)
        {
            lock.enter();

            if (indexToChange < numUsed)
            {
                this->elements [indexToChange] = newValue;
            }
            else
            {
                this->ensureAllocatedSize (numUsed + 1);
                this->elements [numUsed++] = newValue;
            }

            lock.exit();
        }
    }

    /** Replaces an element with a new value without doing any bounds-checking.

        This just sets a value directly in the array's internal storage, so you'd
        better make sure it's in range!

        @param indexToChange    the index whose value you want to change
        @param newValue         the new value to set for this index.
        @see set, getUnchecked
    */
    void setUnchecked (const int indexToChange,
                       const ElementType newValue) throw()
    {
        lock.enter();
        jassert (indexToChange >= 0 && indexToChange < numUsed);
        this->elements [indexToChange] = newValue;
        lock.exit();
    }

    /** Adds elements from an array to the end of this array.

        @param elementsToAdd        the array of elements to add
        @param numElementsToAdd     how many elements are in this other array
        @see add
    */
    void addArray (const ElementType* elementsToAdd,
                   int numElementsToAdd) throw()
    {
        lock.enter();

        if (numElementsToAdd > 0)
        {
            this->ensureAllocatedSize (numUsed + numElementsToAdd);

            while (--numElementsToAdd >= 0)
                this->elements [numUsed++] = *elementsToAdd++;
        }

        lock.exit();
    }

    /** This swaps the contents of this array with those of another array.

        If you need to exchange two arrays, this is vastly quicker than using copy-by-value
        because it just swaps their internal pointers.
    */
    template <class OtherArrayType>
    void swapWithArray (OtherArrayType& otherArray) throw()
    {
        lock.enter();
        otherArray.lock.enter();
        swapVariables <int> (this->numUsed, otherArray.numUsed);
        swapVariables <ElementType*> (this->elements, otherArray.elements);
        swapVariables <int> (this->numAllocated, otherArray.numAllocated);
        otherArray.lock.exit();
        lock.exit();
    }

    /** Adds elements from another array to the end of this array.

        @param arrayToAddFrom       the array from which to copy the elements
        @param startIndex           the first element of the other array to start copying from
        @param numElementsToAdd     how many elements to add from the other array. If this
                                    value is negative or greater than the number of available elements,
                                    all available elements will be copied.
        @see add
    */
    template <class OtherArrayType>
    void addArray (const OtherArrayType& arrayToAddFrom,
                   int startIndex = 0,
                   int numElementsToAdd = -1) throw()
    {
        arrayToAddFrom.lockArray();
        lock.enter();
        jassert (this != &arrayToAddFrom);

        if (startIndex < 0)
        {
            jassertfalse
            startIndex = 0;
        }

        if (numElementsToAdd < 0 || startIndex + numElementsToAdd > arrayToAddFrom.size())
            numElementsToAdd = arrayToAddFrom.size() - startIndex;

        this->addArray ((const ElementType*) (arrayToAddFrom.elements + startIndex), numElementsToAdd);

        lock.exit();
        arrayToAddFrom.unlockArray();
    }

    /** Inserts a new element into the array, assuming that the array is sorted.

        This will use a comparator to find the position at which the new element
        should go. If the array isn't sorted, the behaviour of this
        method will be unpredictable.

        @param comparator   the comparator to use to compare the elements - see the sort()
                            method for details about the form this object should take
        @param newElement   the new element to insert to the array
        @see add, sort
    */
    template <class ElementComparator>
    void addSorted (ElementComparator& comparator,
                    const ElementType newElement) throw()
    {
        lock.enter();
        insert (findInsertIndexInSortedArray (comparator, this->elements, newElement, 0, numUsed), newElement);
        lock.exit();
    }

    /** Finds the index of an element in the array, assuming that the array is sorted.

        This will use a comparator to do a binary-chop to find the index of the given
        element, if it exists. If the array isn't sorted, the behaviour of this
        method will be unpredictable.

        @param comparator           the comparator to use to compare the elements - see the sort()
                                    method for details about the form this object should take
        @param elementToLookFor     the element to search for
        @returns                    the index of the element, or -1 if it's not found
        @see addSorted, sort
    */
    template <class ElementComparator>
    int indexOfSorted (ElementComparator& comparator,
                       const ElementType elementToLookFor) const throw()
    {
        (void) comparator;  // if you pass in an object with a static compareElements() method, this
                            // avoids getting warning messages about the parameter being unused
        lock.enter();

        int start = 0;
        int end = numUsed;

        for (;;)
        {
            if (start >= end)
            {
                lock.exit();
                return -1;
            }
            else if (comparator.compareElements (elementToLookFor, this->elements [start]) == 0)
            {
                lock.exit();
                return start;
            }
            else
            {
                const int halfway = (start + end) >> 1;

                if (halfway == start)
                {
                    lock.exit();
                    return -1;
                }
                else if (comparator.compareElements (elementToLookFor, this->elements [halfway]) >= 0)
                    start = halfway;
                else
                    end = halfway;
            }
        }
    }

    //==============================================================================
    /** Removes an element from the array.

        This will remove the element at a given index, and move back
        all the subsequent elements to close the gap.
        If the index passed in is out-of-range, nothing will happen.

        @param indexToRemove    the index of the element to remove
        @returns                the element that has been removed
        @see removeValue, removeRange
    */
    ElementType remove (const int indexToRemove) throw()
    {
        lock.enter();

        if (indexToRemove >= 0 && indexToRemove < numUsed)
        {
            --numUsed;

            ElementType* const e = this->elements + indexToRemove;
            ElementType const removed = *e;
            const int numberToShift = numUsed - indexToRemove;

            if (numberToShift > 0)
                memmove (e, e + 1, numberToShift * sizeof (ElementType));

            if ((numUsed << 1) < this->numAllocated)
                minimiseStorageOverheads();

            lock.exit();
            return removed;
        }
        else
        {
            lock.exit();
            return ElementType();
        }
    }

    /** Removes an item from the array.

        This will remove the first occurrence of the given element from the array.
        If the item isn't found, no action is taken.

        @param valueToRemove   the object to try to remove
        @see remove, removeRange
    */
    void removeValue (const ElementType valueToRemove) throw()
    {
        lock.enter();
        ElementType* e = this->elements;

        for (int i = numUsed; --i >= 0;)
        {
            if (valueToRemove == *e)
            {
                remove ((int) (e - this->elements));
                break;
            }

            ++e;
        }

        lock.exit();
    }

    /** Removes a range of elements from the array.

        This will remove a set of elements, starting from the given index,
        and move subsequent elements down to close the gap.

        If the range extends beyond the bounds of the array, it will
        be safely clipped to the size of the array.

        @param startIndex       the index of the first element to remove
        @param numberToRemove   how many elements should be removed
        @see remove, removeValue
    */
    void removeRange (int startIndex,
                      const int numberToRemove) throw()
    {
        lock.enter();
        const int endIndex = jlimit (0, numUsed, startIndex + numberToRemove);
        startIndex = jlimit (0, numUsed, startIndex);

        if (endIndex > startIndex)
        {
            const int rangeSize = endIndex - startIndex;
            ElementType* e = this->elements + startIndex;
            int numToShift = numUsed - endIndex;
            numUsed -= rangeSize;

            while (--numToShift >= 0)
            {
                *e = e [rangeSize];
                ++e;
            }

            if ((numUsed << 1) < this->numAllocated)
                minimiseStorageOverheads();
        }

        lock.exit();
    }

    /** Removes the last n elements from the array.

        @param howManyToRemove   how many elements to remove from the end of the array
        @see remove, removeValue, removeRange
    */
    void removeLast (const int howManyToRemove = 1) throw()
    {
        lock.enter();
        numUsed = jmax (0, numUsed - howManyToRemove);

        if ((numUsed << 1) < this->numAllocated)
            minimiseStorageOverheads();

        lock.exit();
    }

    /** Removes any elements which are also in another array.

        @param otherArray   the other array in which to look for elements to remove
        @see removeValuesNotIn, remove, removeValue, removeRange
    */
    template <class OtherArrayType>
    void removeValuesIn (const OtherArrayType& otherArray) throw()
    {
        otherArray.lockArray();
        lock.enter();

        if (this == &otherArray)
        {
            clear();
        }
        else
        {
            if (otherArray.size() > 0)
            {
                for (int i = numUsed; --i >= 0;)
                    if (otherArray.contains (this->elements [i]))
                        remove (i);
            }
        }

        lock.exit();
        otherArray.unlockArray();
    }

    /** Removes any elements which are not found in another array.

        Only elements which occur in this other array will be retained.

        @param otherArray    the array in which to look for elements NOT to remove
        @see removeValuesIn, remove, removeValue, removeRange
    */
    template <class OtherArrayType>
    void removeValuesNotIn (const OtherArrayType& otherArray) throw()
    {
        otherArray.lockArray();
        lock.enter();

        if (this != &otherArray)
        {
            if (otherArray.size() <= 0)
            {
                clear();
            }
            else
            {
                for (int i = numUsed; --i >= 0;)
                    if (! otherArray.contains (this->elements [i]))
                        remove (i);
            }
        }

        lock.exit();
        otherArray.unlockArray();
    }

    /** Swaps over two elements in the array.

        This swaps over the elements found at the two indexes passed in.
        If either index is out-of-range, this method will do nothing.

        @param index1   index of one of the elements to swap
        @param index2   index of the other element to swap
    */
    void swap (const int index1,
               const int index2) throw()
    {
        lock.enter();

        if (index1 >= 0 && index1 < numUsed
            && index2 >= 0 && index2 < numUsed)
        {
            swapVariables (this->elements [index1],
                           this->elements [index2]);
        }

        lock.exit();
    }

    /** Moves one of the values to a different position.

        This will move the value to a specified index, shuffling along
        any intervening elements as required.

        So for example, if you have the array { 0, 1, 2, 3, 4, 5 } then calling
        move (2, 4) would result in { 0, 1, 3, 4, 2, 5 }.

        @param currentIndex     the index of the value to be moved. If this isn't a
                                valid index, then nothing will be done
        @param newIndex         the index at which you'd like this value to end up. If this
                                is less than zero, the value will be moved to the end
                                of the array
    */
    void move (const int currentIndex,
               int newIndex) throw()
    {
        if (currentIndex != newIndex)
        {
            lock.enter();

            if (currentIndex >= 0 && currentIndex < numUsed)
            {
                if (newIndex < 0 || newIndex > numUsed - 1)
                    newIndex = numUsed - 1;

                const ElementType value = this->elements [currentIndex];

                if (newIndex > currentIndex)
                {
                    memmove (this->elements + currentIndex,
                             this->elements + currentIndex + 1,
                             (newIndex - currentIndex) * sizeof (ElementType));
                }
                else
                {
                    memmove (this->elements + newIndex + 1,
                             this->elements + newIndex,
                             (currentIndex - newIndex) * sizeof (ElementType));
                }

                this->elements [newIndex] = value;
            }

            lock.exit();
        }
    }

    //==============================================================================
    /** Reduces the amount of storage being used by the array.

        Arrays typically allocate slightly more storage than they need, and after
        removing elements, they may have quite a lot of unused space allocated.
        This method will reduce the amount of allocated storage to a minimum.
    */
    void minimiseStorageOverheads() throw()
    {
        lock.enter();

        if (numUsed == 0)
        {
            this->setAllocatedSize (0);
        }
        else
        {
            const int newAllocation = this->granularity * (numUsed / this->granularity + 1);

            if (newAllocation < this->numAllocated)
                this->setAllocatedSize (newAllocation);
        }

        lock.exit();
    }

    /** Increases the array's internal storage to hold a minimum number of elements.

        Calling this before adding a large known number of elements means that
        the array won't have to keep dynamically resizing itself as the elements
        are added, and it'll therefore be more efficient.
    */
    void ensureStorageAllocated (const int minNumElements) throw()
    {
        this->ensureAllocatedSize (minNumElements);
    }

    //==============================================================================
    /** Sorts the elements in the array.

        This will use a comparator object to sort the elements into order. The object
        passed must have a method of the form:
        @code
        int compareElements (ElementType first, ElementType second);
        @endcode

        ..and this method must return:
          - a value of < 0 if the first comes before the second
          - a value of 0 if the two objects are equivalent
          - a value of > 0 if the second comes before the first

        To improve performance, the compareElements() method can be declared as static or const.

        @param comparator   the comparator to use for comparing elements.
        @param retainOrderOfEquivalentItems     if this is true, then items
                            which the comparator says are equivalent will be
                            kept in the order in which they currently appear
                            in the array. This is slower to perform, but may
                            be important in some cases. If it's false, a faster
                            algorithm is used, but equivalent elements may be
                            rearranged.

        @see addSorted, indexOfSorted, sortArray
    */
    template <class ElementComparator>
    void sort (ElementComparator& comparator,
               const bool retainOrderOfEquivalentItems = false) const throw()
    {
        (void) comparator;  // if you pass in an object with a static compareElements() method, this
                            // avoids getting warning messages about the parameter being unused
        lock.enter();
        sortArray (comparator, this->elements, 0, size() - 1, retainOrderOfEquivalentItems);
        lock.exit();
    }

    //==============================================================================
    /** Locks the array's CriticalSection.

        Of course if the type of section used is a DummyCriticalSection, this won't
        have any effect.

        @see unlockArray
    */
    void lockArray() const throw()
    {
        lock.enter();
    }

    /** Unlocks the array's CriticalSection.

        Of course if the type of section used is a DummyCriticalSection, this won't
        have any effect.

        @see lockArray
    */
    void unlockArray() const throw()
    {
        lock.exit();
    }


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    int numUsed;
    TypeOfCriticalSectionToUse lock;
};


#endif   // __JUCE_ARRAY_JUCEHEADER__
