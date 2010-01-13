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

#ifndef __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__
#define __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__

#include "juce_ReferenceCountedObject.h"
#include "juce_ArrayAllocationBase.h"
#include "juce_ElementComparator.h"
#include "../threads/juce_CriticalSection.h"


//==============================================================================
/**
    Holds a list of objects derived from ReferenceCountedObject.

    A ReferenceCountedArray holds objects derived from ReferenceCountedObject,
    and takes care of incrementing and decrementing their ref counts when they
    are added and removed from the array.

    To make all the array's methods thread-safe, pass in "CriticalSection" as the templated
    TypeOfCriticalSectionToUse parameter, instead of the default DummyCriticalSection.

    @see Array, OwnedArray, StringArray
*/
template <class ObjectClass, class TypeOfCriticalSectionToUse = DummyCriticalSection>
class ReferenceCountedArray
{
public:
    //==============================================================================
    /** Creates an empty array.
        @see ReferenceCountedObject, Array, OwnedArray
    */
    ReferenceCountedArray() throw()
        : numUsed (0)
    {
    }

    /** Creates a copy of another array */
    ReferenceCountedArray (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& other) throw()
    {
        other.lockArray();
        numUsed = other.numUsed;
        data.setAllocatedSize (numUsed);
        memcpy (data.elements, other.data.elements, numUsed * sizeof (ObjectClass*));

        for (int i = numUsed; --i >= 0;)
            if (data.elements[i] != 0)
                data.elements[i]->incReferenceCount();

        other.unlockArray();
    }

    /** Copies another array into this one.

        Any existing objects in this array will first be released.
    */
    const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& operator= (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& other) throw()
    {
        if (this != &other)
        {
            other.lockArray();
            lock.enter();

            clear();

            data.ensureAllocatedSize (other.numUsed);
            numUsed = other.numUsed;
            memcpy (data.elements, other.data.elements, numUsed * sizeof (ObjectClass*));
            minimiseStorageOverheads();

            for (int i = numUsed; --i >= 0;)
                if (data.elements[i] != 0)
                    data.elements[i]->incReferenceCount();

            lock.exit();
            other.unlockArray();
        }

        return *this;
    }

    /** Destructor.

        Any objects in the array will be released, and may be deleted if not referenced from elsewhere.
    */
    ~ReferenceCountedArray()
    {
        clear();
    }

    //==============================================================================
    /** Removes all objects from the array.

        Any objects in the array that are not referenced from elsewhere will be deleted.
    */
    void clear()
    {
        lock.enter();

        while (numUsed > 0)
            if (data.elements [--numUsed] != 0)
                data.elements [numUsed]->decReferenceCount();

        jassert (numUsed == 0);
        data.setAllocatedSize (0);

        lock.exit();
    }

    /** Returns the current number of objects in the array. */
    inline int size() const throw()
    {
        return numUsed;
    }

    /** Returns a pointer to the object at this index in the array.

        If the index is out-of-range, this will return a null pointer, (and
        it could be null anyway, because it's ok for the array to hold null
        pointers as well as objects).

        @see getUnchecked
    */
    inline const ReferenceCountedObjectPtr<ObjectClass> operator[] (const int index) const throw()
    {
        lock.enter();
        const ReferenceCountedObjectPtr<ObjectClass> result ((((unsigned int) index) < (unsigned int) numUsed)
                                                                ? data.elements [index]
                                                                : (ObjectClass*) 0);
        lock.exit();
        return result;
    }

    /** Returns a pointer to the object at this index in the array, without checking whether the index is in-range.

        This is a faster and less safe version of operator[] which doesn't check the index passed in, so
        it can be used when you're sure the index if always going to be legal.
    */
    inline const ReferenceCountedObjectPtr<ObjectClass> getUnchecked (const int index) const throw()
    {
        lock.enter();
        jassert (((unsigned int) index) < (unsigned int) numUsed);
        const ReferenceCountedObjectPtr<ObjectClass> result (data.elements [index]);
        lock.exit();
        return result;
    }

    /** Returns a pointer to the first object in the array.

        This will return a null pointer if the array's empty.
        @see getLast
    */
    inline const ReferenceCountedObjectPtr<ObjectClass> getFirst() const throw()
    {
        lock.enter();
        const ReferenceCountedObjectPtr<ObjectClass> result ((numUsed > 0) ? data.elements [0]
                                                                           : (ObjectClass*) 0);
        lock.exit();

        return result;
    }

    /** Returns a pointer to the last object in the array.

        This will return a null pointer if the array's empty.
        @see getFirst
    */
    inline const ReferenceCountedObjectPtr<ObjectClass> getLast() const throw()
    {
        lock.enter();
        const ReferenceCountedObjectPtr<ObjectClass> result ((numUsed > 0) ? data.elements [numUsed - 1]
                                                                           : (ObjectClass*) 0);
        lock.exit();

        return result;
    }

    //==============================================================================
    /** Finds the index of the first occurrence of an object in the array.

        @param objectToLookFor    the object to look for
        @returns                  the index at which the object was found, or -1 if it's not found
    */
    int indexOf (const ObjectClass* const objectToLookFor) const throw()
    {
        int result = -1;

        lock.enter();
        ObjectClass** e = data.elements;

        for (int i = numUsed; --i >= 0;)
        {
            if (objectToLookFor == *e)
            {
                result = (int) (e - data.elements);
                break;
            }

            ++e;
        }

        lock.exit();
        return result;
    }

    /** Returns true if the array contains a specified object.

        @param objectToLookFor      the object to look for
        @returns                    true if the object is in the array
    */
    bool contains (const ObjectClass* const objectToLookFor) const throw()
    {
        lock.enter();
        ObjectClass** e = data.elements;

        for (int i = numUsed; --i >= 0;)
        {
            if (objectToLookFor == *e)
            {
                lock.exit();
                return true;
            }

            ++e;
        }

        lock.exit();
        return false;
    }

    /** Appends a new object to the end of the array.

        This will increase the new object's reference count.

        @param newObject       the new object to add to the array
        @see set, insert, addIfNotAlreadyThere, addSorted, addArray
    */
    void add (ObjectClass* const newObject) throw()
    {
        lock.enter();
        data.ensureAllocatedSize (numUsed + 1);
        data.elements [numUsed++] = newObject;

        if (newObject != 0)
            newObject->incReferenceCount();

        lock.exit();
    }

    /** Inserts a new object into the array at the given index.

        If the index is less than 0 or greater than the size of the array, the
        element will be added to the end of the array.
        Otherwise, it will be inserted into the array, moving all the later elements
        along to make room.

        This will increase the new object's reference count.

        @param indexToInsertAt      the index at which the new element should be inserted
        @param newObject            the new object to add to the array
        @see add, addSorted, addIfNotAlreadyThere, set
    */
    void insert (int indexToInsertAt,
                 ObjectClass* const newObject) throw()
    {
        if (indexToInsertAt >= 0)
        {
            lock.enter();

            if (indexToInsertAt > numUsed)
                indexToInsertAt = numUsed;

            data.ensureAllocatedSize (numUsed + 1);

            ObjectClass** const e = data.elements + indexToInsertAt;
            const int numToMove = numUsed - indexToInsertAt;

            if (numToMove > 0)
                memmove (e + 1, e, numToMove * sizeof (ObjectClass*));

            *e = newObject;

            if (newObject != 0)
                newObject->incReferenceCount();

            ++numUsed;
            lock.exit();
        }
        else
        {
            add (newObject);
        }
    }

    /** Appends a new object at the end of the array as long as the array doesn't
        already contain it.

        If the array already contains a matching object, nothing will be done.

        @param newObject   the new object to add to the array
    */
    void addIfNotAlreadyThere (ObjectClass* const newObject) throw()
    {
        lock.enter();

        if (! contains (newObject))
            add (newObject);

        lock.exit();
    }

    /** Replaces an object in the array with a different one.

        If the index is less than zero, this method does nothing.
        If the index is beyond the end of the array, the new object is added to the end of the array.

        The object being added has its reference count increased, and if it's replacing
        another object, then that one has its reference count decreased, and may be deleted.

        @param indexToChange        the index whose value you want to change
        @param newObject            the new value to set for this index.
        @see add, insert, remove
    */
    void set (const int indexToChange,
              ObjectClass* const newObject)
    {
        if (indexToChange >= 0)
        {
            lock.enter();

            if (newObject != 0)
                newObject->incReferenceCount();

            if (indexToChange < numUsed)
            {
                if (data.elements [indexToChange] != 0)
                    data.elements [indexToChange]->decReferenceCount();

                data.elements [indexToChange] = newObject;
            }
            else
            {
                data.ensureAllocatedSize (numUsed + 1);
                data.elements [numUsed++] = newObject;
            }

            lock.exit();
        }
    }

    /** Adds elements from another array to the end of this array.

        @param arrayToAddFrom       the array from which to copy the elements
        @param startIndex           the first element of the other array to start copying from
        @param numElementsToAdd     how many elements to add from the other array. If this
                                    value is negative or greater than the number of available elements,
                                    all available elements will be copied.
        @see add
    */
    void addArray (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& arrayToAddFrom,
                   int startIndex = 0,
                   int numElementsToAdd = -1) throw()
    {
        arrayToAddFrom.lockArray();
        lock.enter();

        if (startIndex < 0)
        {
            jassertfalse
            startIndex = 0;
        }

        if (numElementsToAdd < 0 || startIndex + numElementsToAdd > arrayToAddFrom.size())
            numElementsToAdd = arrayToAddFrom.size() - startIndex;

        if (numElementsToAdd > 0)
        {
            data.ensureAllocatedSize (numUsed + numElementsToAdd);

            while (--numElementsToAdd >= 0)
                add (arrayToAddFrom.getUnchecked (startIndex++));
        }

        lock.exit();
        arrayToAddFrom.unlockArray();
    }

    /** Inserts a new object into the array assuming that the array is sorted.

        This will use a comparator to find the position at which the new object
        should go. If the array isn't sorted, the behaviour of this
        method will be unpredictable.

        @param comparator       the comparator object to use to compare the elements - see the
                                sort() method for details about this object's form
        @param newObject        the new object to insert to the array
        @see add, sort
    */
    template <class ElementComparator>
    void addSorted (ElementComparator& comparator,
                    ObjectClass* newObject) throw()
    {
        lock.enter();
        insert (findInsertIndexInSortedArray (comparator, data.elements, newObject, 0, numUsed), newObject);
        lock.exit();
    }

    /** Inserts or replaces an object in the array, assuming it is sorted.

        This is similar to addSorted, but if a matching element already exists, then it will be
        replaced by the new one, rather than the new one being added as well.
    */
    template <class ElementComparator>
    void addOrReplaceSorted (ElementComparator& comparator,
                             ObjectClass* newObject) throw()
    {
        lock.enter();
        const int index = findInsertIndexInSortedArray (comparator, data.elements, newObject, 0, numUsed);

        if (index > 0 && comparator.compareElements (newObject, data.elements [index - 1]) == 0)
            set (index - 1, newObject); // replace an existing object that matches
        else
            insert (index, newObject);  // no match, so insert the new one

        lock.exit();
    }

    //==============================================================================
    /** Removes an object from the array.

        This will remove the object at a given index and move back all the
        subsequent objects to close the gap.

        If the index passed in is out-of-range, nothing will happen.

        The object that is removed will have its reference count decreased,
        and may be deleted if not referenced from elsewhere.

        @param indexToRemove    the index of the element to remove
        @see removeObject, removeRange
    */
    void remove (const int indexToRemove)
    {
        lock.enter();

        if (((unsigned int) indexToRemove) < (unsigned int) numUsed)
        {
            ObjectClass** const e = data.elements + indexToRemove;

            if (*e != 0)
                (*e)->decReferenceCount();

            --numUsed;
            const int numberToShift = numUsed - indexToRemove;

            if (numberToShift > 0)
                memmove (e, e + 1, numberToShift * sizeof (ObjectClass*));

            if ((numUsed << 1) < data.numAllocated)
                minimiseStorageOverheads();
        }

        lock.exit();
    }

    /** Removes the first occurrence of a specified object from the array.

        If the item isn't found, no action is taken. If it is found, it is
        removed and has its reference count decreased.

        @param objectToRemove   the object to try to remove
        @see remove, removeRange
    */
    void removeObject (ObjectClass* const objectToRemove)
    {
        lock.enter();
        remove (indexOf (objectToRemove));
        lock.exit();
    }

    /** Removes a range of objects from the array.

        This will remove a set of objects, starting from the given index,
        and move any subsequent elements down to close the gap.

        If the range extends beyond the bounds of the array, it will
        be safely clipped to the size of the array.

        The objects that are removed will have their reference counts decreased,
        and may be deleted if not referenced from elsewhere.

        @param startIndex       the index of the first object to remove
        @param numberToRemove   how many objects should be removed
        @see remove, removeObject
    */
    void removeRange (const int startIndex,
                      const int numberToRemove)
    {
        lock.enter();

        const int start = jlimit (0, numUsed, startIndex);
        const int end   = jlimit (0, numUsed, startIndex + numberToRemove);

        if (end > start)
        {
            int i;
            for (i = start; i < end; ++i)
            {
                if (data.elements[i] != 0)
                {
                    data.elements[i]->decReferenceCount();
                    data.elements[i] = 0; // (in case one of the destructors accesses this array and hits a dangling pointer)
                }
            }

            const int rangeSize = end - start;
            ObjectClass** e = data.elements + start;
            i = numUsed - end;
            numUsed -= rangeSize;

            while (--i >= 0)
            {
                *e = e [rangeSize];
                ++e;
            }

            if ((numUsed << 1) < data.numAllocated)
                minimiseStorageOverheads();
        }

        lock.exit();
    }

    /** Removes the last n objects from the array.

        The objects that are removed will have their reference counts decreased,
        and may be deleted if not referenced from elsewhere.

        @param howManyToRemove   how many objects to remove from the end of the array
        @see remove, removeObject, removeRange
    */
    void removeLast (int howManyToRemove = 1)
    {
        lock.enter();

        if (howManyToRemove > numUsed)
            howManyToRemove = numUsed;

        while (--howManyToRemove >= 0)
            remove (numUsed - 1);

        lock.exit();
    }

    /** Swaps a pair of objects in the array.

        If either of the indexes passed in is out-of-range, nothing will happen,
        otherwise the two objects at these positions will be exchanged.
    */
    void swap (const int index1,
               const int index2) throw()
    {
        lock.enter();

        if (((unsigned int) index1) < (unsigned int) numUsed
             && ((unsigned int) index2) < (unsigned int) numUsed)
        {
            swapVariables (data.elements [index1],
                           data.elements [index2]);
        }

        lock.exit();
    }

    /** Moves one of the objects to a different position.

        This will move the object to a specified index, shuffling along
        any intervening elements as required.

        So for example, if you have the array { 0, 1, 2, 3, 4, 5 } then calling
        move (2, 4) would result in { 0, 1, 3, 4, 2, 5 }.

        @param currentIndex     the index of the object to be moved. If this isn't a
                                valid index, then nothing will be done
        @param newIndex         the index at which you'd like this object to end up. If this
                                is less than zero, it will be moved to the end of the array
    */
    void move (const int currentIndex,
               int newIndex) throw()
    {
        if (currentIndex != newIndex)
        {
            lock.enter();

            if (((unsigned int) currentIndex) < (unsigned int) numUsed)
            {
                if (((unsigned int) newIndex) >= (unsigned int) numUsed)
                    newIndex = numUsed - 1;

                ObjectClass* const value = data.elements [currentIndex];

                if (newIndex > currentIndex)
                {
                    memmove (data.elements + currentIndex,
                             data.elements + currentIndex + 1,
                             (newIndex - currentIndex) * sizeof (ObjectClass*));
                }
                else
                {
                    memmove (data.elements + newIndex + 1,
                             data.elements + newIndex,
                             (currentIndex - newIndex) * sizeof (ObjectClass*));
                }

                data.elements [newIndex] = value;
            }

            lock.exit();
        }
    }

    //==============================================================================
    /** Compares this array to another one.

        @returns true only if the other array contains the same objects in the same order
    */
    bool operator== (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& other) const throw()
    {
        other.lockArray();
        lock.enter();

        bool result = numUsed == other.numUsed;

        if (result)
        {
            for (int i = numUsed; --i >= 0;)
            {
                if (data.elements [i] != other.data.elements [i])
                {
                    result = false;
                    break;
                }
            }
        }

        lock.exit();
        other.unlockArray();

        return result;
    }

    /** Compares this array to another one.

        @see operator==
    */
    bool operator!= (const ReferenceCountedArray<ObjectClass, TypeOfCriticalSectionToUse>& other) const throw()
    {
        return ! operator== (other);
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

        @see sortArray
    */
    template <class ElementComparator>
    void sort (ElementComparator& comparator,
               const bool retainOrderOfEquivalentItems = false) const throw()
    {
        (void) comparator;  // if you pass in an object with a static compareElements() method, this
                            // avoids getting warning messages about the parameter being unused

        lock.enter();
        sortArray (comparator, data.elements, 0, size() - 1, retainOrderOfEquivalentItems);
        lock.exit();
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
        data.shrinkToNoMoreThan (numUsed);
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
    ArrayAllocationBase <ObjectClass*> data;
    int numUsed;
    TypeOfCriticalSectionToUse lock;
};


#endif   // __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__
