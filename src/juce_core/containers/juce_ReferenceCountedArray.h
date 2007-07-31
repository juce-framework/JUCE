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

    @see Array, OwnedArray, StringArray
*/
template <class ObjectClass>
class ReferenceCountedArray   : private ArrayAllocationBase <ObjectClass*>
{
public:
    //==============================================================================
    /** Creates an empty array.

        @param granularity  this is the size of increment by which the internal storage
        used by the array will grow. Only change it from the default if you know the
        array is going to be very big and needs to be able to grow efficiently.

        @see ReferenceCountedObject, ArrayAllocationBase, Array, OwnedArray
    */
    ReferenceCountedArray (const int granularity = juceDefaultArrayGranularity) throw()
        : ArrayAllocationBase <ObjectClass*> (granularity),
          numUsed (0)
    {
    }

    /** Creates a copy of another array */
    ReferenceCountedArray (const ReferenceCountedArray<ObjectClass>& other) throw()
        : ArrayAllocationBase <ObjectClass*> (other.granularity),
          numUsed (other.numUsed)
    {
        this->setAllocatedSize (numUsed);
        memcpy (this->elements, other.elements, numUsed * sizeof (ObjectClass*));

        for (int i = numUsed; --i >= 0;)
            if (this->elements[i] != 0)
                this->elements[i]->incReferenceCount();
    }

    /** Copies another array into this one.

        Any existing objects in this array will first be released.
    */
    const ReferenceCountedArray<ObjectClass>& operator= (const ReferenceCountedArray<ObjectClass>& other) throw()
    {
        if (this != &other)
        {
            clear();

            this->granularity = other.granularity;
            this->ensureAllocatedSize (other.numUsed);
            numUsed = other.numUsed;
            memcpy (this->elements, other.elements, numUsed * sizeof (ObjectClass*));
            minimiseStorageOverheads();

            for (int i = numUsed; --i >= 0;)
                if (this->elements[i] != 0)
                    this->elements[i]->incReferenceCount();
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
        while (numUsed > 0)
            if (this->elements [--numUsed] != 0)
                this->elements [numUsed]->decReferenceCount();

        jassert (numUsed == 0);
        this->setAllocatedSize (0);
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
    inline ObjectClass* operator[] (const int index) const throw()
    {
        return (index >= 0 && index < numUsed) ? this->elements [index]
                                               : (ObjectClass*) 0;
    }

    /** Returns a pointer to the object at this index in the array, without checking whether the index is in-range.

        This is a faster and less safe version of operator[] which doesn't check the index passed in, so
        it can be used when you're sure the index if always going to be legal.
    */
    inline ObjectClass* getUnchecked (const int index) const throw()
    {
        jassert (index >= 0 && index < numUsed);
        return this->elements [index];
    }

    /** Returns a pointer to the first object in the array.

        This will return a null pointer if the array's empty.
        @see getLast
    */
    inline ObjectClass* getFirst() const throw()
    {
        return (numUsed > 0) ? this->elements [0]
                             : (ObjectClass*) 0;
    }

    /** Returns a pointer to the last object in the array.

        This will return a null pointer if the array's empty.
        @see getFirst
    */
    inline ObjectClass* getLast() const throw()
    {
        return (numUsed > 0) ? this->elements [numUsed - 1]
                             : (ObjectClass*) 0;
    }

    //==============================================================================
    /** Finds the index of the first occurrence of an object in the array.

        @param objectToLookFor    the object to look for
        @returns                  the index at which the object was found, or -1 if it's not found
    */
    int indexOf (const ObjectClass* const objectToLookFor) const throw()
    {
        ObjectClass** e = this->elements;

        for (int i = numUsed; --i >= 0;)
        {
            if (objectToLookFor == *e)
                return (int) (e - this->elements);

            ++e;
        }

        return -1;
    }

    /** Returns true if the array contains a specified object.

        @param objectToLookFor      the object to look for
        @returns                    true if the object is in the array
    */
    bool contains (const ObjectClass* const objectToLookFor) const throw()
    {
        ObjectClass** e = this->elements;

        for (int i = numUsed; --i >= 0;)
        {
            if (objectToLookFor == *e)
                return true;

            ++e;
        }

        return false;
    }

    /** Appends a new object to the end of the array.

        This will increase the new object's reference count.

        @param newObject       the new object to add to the array
        @see set, insert, addIfNotAlreadyThere, addSorted, addArray
    */
    void add (ObjectClass* const newObject) throw()
    {
        this->ensureAllocatedSize (numUsed + 1);
        this->elements [numUsed++] = newObject;

        if (newObject != 0)
            newObject->incReferenceCount();
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
            if (indexToInsertAt > numUsed)
                indexToInsertAt = numUsed;

            this->ensureAllocatedSize (numUsed + 1);

            ObjectClass** const e = this->elements + indexToInsertAt;
            const int numToMove = numUsed - indexToInsertAt;

            if (numToMove > 0)
                memmove (e + 1, e, numToMove * sizeof (ObjectClass*));

            *e = newObject;

            if (newObject != 0)
                newObject->incReferenceCount();

            ++numUsed;
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
        if (! contains (newObject))
            add (newObject);
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
            if (newObject != 0)
                newObject->incReferenceCount();

            if (indexToChange < numUsed)
            {
                if (this->elements [indexToChange] != 0)
                    this->elements [indexToChange]->decReferenceCount();

                this->elements [indexToChange] = newObject;
            }
            else
            {
                this->ensureAllocatedSize (numUsed + 1);
                this->elements [numUsed++] = newObject;
            }
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
    void addArray (const ReferenceCountedArray<ObjectClass>& arrayToAddFrom,
                   int startIndex = 0,
                   int numElementsToAdd = -1) throw()
    {
        if (startIndex < 0)
        {
            jassertfalse
            startIndex = 0;
        }

        if (numElementsToAdd < 0 || startIndex + numElementsToAdd > arrayToAddFrom.size())
            numElementsToAdd = arrayToAddFrom.size() - startIndex;

        if (numElementsToAdd > 0)
        {
            this->ensureAllocatedSize (numUsed + numElementsToAdd);

            while (--numElementsToAdd >= 0)
                add (arrayToAddFrom.getUnchecked (startIndex++));
        }
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
        insert (findInsertIndexInSortedArray (comparator, this->elements, newObject, 0, numUsed), newObject);
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
        if (indexToRemove >= 0 && indexToRemove < numUsed)
        {
            ObjectClass** const e = this->elements + indexToRemove;

            if (*e != 0)
                (*e)->decReferenceCount();

            --numUsed;
            const int numberToShift = numUsed - indexToRemove;

            if (numberToShift > 0)
                memmove (e, e + 1, numberToShift * sizeof (ObjectClass*));

            if ((numUsed << 1) < this->numAllocated)
                minimiseStorageOverheads();
        }
    }

    /** Removes the first occurrence of a specified object from the array.

        If the item isn't found, no action is taken. If it is found, it is
        removed and has its reference count decreased.

        @param objectToRemove   the object to try to remove
        @see remove, removeRange
    */
    void removeObject (ObjectClass* const objectToRemove)
    {
        remove (indexOf (objectToRemove));
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
        const int start = jlimit (0, numUsed, startIndex);
        const int end   = jlimit (0, numUsed, startIndex + numberToRemove);

        if (end > start)
        {
            int i;
            for (i = start; i < end; ++i)
            {
                if (this->elements[i] != 0)
                {
                    this->elements[i]->decReferenceCount();
                    this->elements[i] = 0; // (in case one of the destructors accesses this array and hits a dangling pointer)
                }
            }

            const int rangeSize = end - start;
            ObjectClass** e = this->elements + start;
            i = numUsed - end;
            numUsed -= rangeSize;

            while (--i >= 0)
            {
                *e = e [rangeSize];
                ++e;
            }

            if ((numUsed << 1) < this->numAllocated)
                minimiseStorageOverheads();
        }
    }

    /** Removes the last n objects from the array.

        The objects that are removed will have their reference counts decreased,
        and may be deleted if not referenced from elsewhere.

        @param howManyToRemove   how many objects to remove from the end of the array
        @see remove, removeObject, removeRange
    */
    void removeLast (int howManyToRemove = 1)
    {
        if (howManyToRemove > numUsed)
            howManyToRemove = numUsed;

        while (--howManyToRemove >= 0)
            remove (numUsed - 1);
    }

    /** Swaps a pair of objects in the array.

        If either of the indexes passed in is out-of-range, nothing will happen,
        otherwise the two objects at these positions will be exchanged.
    */
    void swap (const int index1,
               const int index2) throw()
    {
        if (index1 >= 0 && index1 < numUsed
             && index2 >= 0 && index2 < numUsed)
        {
            swapVariables (this->elements [index1],
                           this->elements [index2]);
        }
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
            if (currentIndex >= 0 && currentIndex < numUsed)
            {
                if (newIndex < 0 || newIndex > numUsed - 1)
                    newIndex = numUsed - 1;

                ObjectClass* const value = this->elements [currentIndex];

                if (newIndex > currentIndex)
                {
                    memmove (this->elements + currentIndex,
                             this->elements + currentIndex + 1,
                             (newIndex - currentIndex) * sizeof (ObjectClass*));
                }
                else
                {
                    memmove (this->elements + newIndex + 1,
                             this->elements + newIndex,
                             (currentIndex - newIndex) * sizeof (ObjectClass*));
                }

                this->elements [newIndex] = value;
            }
        }
    }

    //==============================================================================
    /** Compares this array to another one.

        @returns true only if the other array contains the same objects in the same order
    */
    bool operator== (const ReferenceCountedArray<ObjectClass>& other) const throw()
    {
        if (numUsed != other.numUsed)
            return false;

        for (int i = numUsed; --i >= 0;)
            if (this->elements [i] != other.elements [i])
                return false;

        return true;
    }

    /** Compares this array to another one.

        @see operator==
    */
    bool operator!= (const ReferenceCountedArray<ObjectClass>& other) const throw()
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
        sortArray (comparator, this->elements, 0, size() - 1, retainOrderOfEquivalentItems);
    }

    //==============================================================================
    /** Reduces the amount of storage being used by the array.

        Arrays typically allocate slightly more storage than they need, and after
        removing elements, they may have quite a lot of unused space allocated.
        This method will reduce the amount of allocated storage to a minimum.
    */
    void minimiseStorageOverheads() throw()
    {
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
    }


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    int numUsed;
};


#endif   // __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__
