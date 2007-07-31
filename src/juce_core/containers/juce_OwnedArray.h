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

#ifndef __JUCE_OWNEDARRAY_JUCEHEADER__
#define __JUCE_OWNEDARRAY_JUCEHEADER__

#include "juce_ArrayAllocationBase.h"
#include "juce_ElementComparator.h"
#include "../threads/juce_CriticalSection.h"


//==============================================================================
/** An array designed for holding objects.

    This holds a list of pointers to objects, and will automatically
    delete the objects when they are removed from the array, or when the
    array is itself deleted.

    Declare it in the form:  OwnedArray<MyObjectClass>

    ..and then add new objects, e.g.   myOwnedArray.add (new MyObjectClass());

    After adding objects, they are 'owned' by the array and will be deleted when
    removed or replaced.

    To make all the array's methods thread-safe, pass in "CriticalSection" as the templated
    TypeOfCriticalSectionToUse parameter, instead of the default DummyCriticalSection.

    @see Array, ReferenceCountedArray, StringArray, CriticalSection
*/
template <class ObjectClass,
          class TypeOfCriticalSectionToUse = DummyCriticalSection>

class OwnedArray   : private ArrayAllocationBase <ObjectClass*>
{
public:
    //==============================================================================
    /** Creates an empty array.

        @param granularity  this is the size of increment by which the internal storage
        used by the array will grow. Only change it from the default if you know the
        array is going to be very big and needs to be able to grow efficiently.

        @see ArrayAllocationBase
    */
    OwnedArray (const int granularity = juceDefaultArrayGranularity) throw()
        : ArrayAllocationBase <ObjectClass*> (granularity),
          numUsed (0)
    {
    }

    /** Deletes the array and also deletes any objects inside it.

        To get rid of the array without deleting its objects, use its
        clear (false) method before deleting it.
    */
    ~OwnedArray()
    {
        clear (true);
    }

    //==============================================================================
    /** Clears the array, optionally deleting the objects inside it first. */
    void clear (const bool deleteObjects = true)
    {
        lock.enter();

        if (deleteObjects)
        {
            while (numUsed > 0)
                delete this->elements [--numUsed];
        }

        this->setAllocatedSize (0);
        numUsed = 0;
        lock.exit();
    }

    //==============================================================================
    /** Returns the number of items currently in the array.
        @see operator[]
    */
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
        lock.enter();
        ObjectClass* const result = (index >= 0 && index < numUsed) ? this->elements [index]
                                                                    : (ObjectClass*) 0;
        lock.exit();

        return result;
    }

    /** Returns a pointer to the object at this index in the array, without checking whether the index is in-range.

        This is a faster and less safe version of operator[] which doesn't check the index passed in, so
        it can be used when you're sure the index if always going to be legal.
    */
    inline ObjectClass* getUnchecked (const int index) const throw()
    {
        lock.enter();
        jassert (index >= 0 && index < numUsed);
        ObjectClass* const result = this->elements [index];
        lock.exit();

        return result;
    }

    /** Returns a pointer to the first object in the array.

        This will return a null pointer if the array's empty.
        @see getLast
    */
    inline ObjectClass* getFirst() const throw()
    {
        lock.enter();
        ObjectClass* const result = (numUsed > 0) ? this->elements [0]
                                                  : (ObjectClass*) 0;
        lock.exit();
        return result;
    }

    /** Returns a pointer to the last object in the array.

        This will return a null pointer if the array's empty.
        @see getFirst
    */
    inline ObjectClass* getLast() const throw()
    {
        lock.enter();
        ObjectClass* const result = (numUsed > 0) ? this->elements [numUsed - 1]
                                                  : (ObjectClass*) 0;
        lock.exit();

        return result;
    }

    //==============================================================================
    /** Finds the index of an object which might be in the array.

        @param objectToLookFor    the object to look for
        @returns                  the index at which the object was found, or -1 if it's not found
    */
    int indexOf (const ObjectClass* const objectToLookFor) const throw()
    {
        int result = -1;

        lock.enter();
        ObjectClass* const* e = this->elements;

        for (int i = numUsed; --i >= 0;)
        {
            if (objectToLookFor == *e)
            {
                result = (int) (e - this->elements);
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

        ObjectClass* const* e = this->elements;
        int i = numUsed;

        while (i >= 4)
        {
            if (objectToLookFor == *e
                 || objectToLookFor == *++e
                 || objectToLookFor == *++e
                 || objectToLookFor == *++e)
            {
                lock.exit();
                return true;
            }

            i -= 4;
            ++e;
        }

        while (i > 0)
        {
            if (objectToLookFor == *e)
            {
                lock.exit();
                return true;
            }

            --i;
            ++e;
        }

        lock.exit();
        return false;
    }

    //==============================================================================
    /** Appends a new object to the end of the array.

        Note that the this object will be deleted by the OwnedArray when it
        is removed, so be careful not to delete it somewhere else.

        Also be careful not to add the same object to the array more than once,
        as this will obviously cause deletion of dangling pointers.

        @param newObject       the new object to add to the array
        @see set, insert, addIfNotAlreadyThere, addSorted
    */
    void add (const ObjectClass* const newObject) throw()
    {
        lock.enter();
        this->ensureAllocatedSize (numUsed + 1);
        this->elements [numUsed++] = const_cast <ObjectClass*> (newObject);
        lock.exit();
    }

    /** Inserts a new object into the array at the given index.

        Note that the this object will be deleted by the OwnedArray when it
        is removed, so be careful not to delete it somewhere else.

        If the index is less than 0 or greater than the size of the array, the
        element will be added to the end of the array.
        Otherwise, it will be inserted into the array, moving all the later elements
        along to make room.

        Be careful not to add the same object to the array more than once,
        as this will obviously cause deletion of dangling pointers.

        @param indexToInsertAt      the index at which the new element should be inserted
        @param newObject            the new object to add to the array
        @see add, addSorted, addIfNotAlreadyThere, set
    */
    void insert (int indexToInsertAt,
                 const ObjectClass* const newObject) throw()
    {
        if (indexToInsertAt >= 0)
        {
            lock.enter();

            if (indexToInsertAt > numUsed)
                indexToInsertAt = numUsed;

            this->ensureAllocatedSize (numUsed + 1);

            ObjectClass** const e = this->elements + indexToInsertAt;
            const int numToMove = numUsed - indexToInsertAt;

            if (numToMove > 0)
                memmove (e + 1, e, numToMove * sizeof (ObjectClass*));

            *e = const_cast <ObjectClass*> (newObject);
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
    void addIfNotAlreadyThere (const ObjectClass* const newObject) throw()
    {
        lock.enter();

        if (! contains (newObject))
            add (newObject);

        lock.exit();
    }

    /** Replaces an object in the array with a different one.

        If the index is less than zero, this method does nothing.
        If the index is beyond the end of the array, the new object is added to the end of the array.

        Be careful not to add the same object to the array more than once,
        as this will obviously cause deletion of dangling pointers.

        @param indexToChange        the index whose value you want to change
        @param newObject            the new value to set for this index.
        @param deleteOldElement     whether to delete the object that's being replaced with the new one
        @see add, insert, remove
    */
    void set (const int indexToChange,
              const ObjectClass* const newObject,
              const bool deleteOldElement = true)
    {
        if (indexToChange >= 0)
        {
            ObjectClass* toDelete = 0;
            lock.enter();

            if (indexToChange < numUsed)
            {
                if (deleteOldElement)
                    toDelete = this->elements [indexToChange];

                if (toDelete == newObject)
                    toDelete = 0;
                else
                    this->elements [indexToChange] = const_cast <ObjectClass*> (newObject);
            }
            else
            {
                this->ensureAllocatedSize (numUsed + 1);
                this->elements [numUsed++] = const_cast <ObjectClass*> (newObject);
            }

            lock.exit();

            delete toDelete;
        }
    }

    /** Inserts a new object into the array assuming that the array is sorted.

        This will use a comparator to find the position at which the new object
        should go. If the array isn't sorted, the behaviour of this
        method will be unpredictable.

        @param comparator   the comparator to use to compare the elements - see the sort method
                            for details about this object's structure
        @param newObject    the new object to insert to the array
        @see add, sort, indexOfSorted
    */
    template <class ElementComparator>
    void addSorted (ElementComparator& comparator,
                    ObjectClass* const newObject) throw()
    {
        (void) comparator;  // if you pass in an object with a static compareElements() method, this
                            // avoids getting warning messages about the parameter being unused
        lock.enter();
        insert (findInsertIndexInSortedArray (comparator, this->elements, newObject, 0, numUsed), newObject);
        lock.exit();
    }

    /** Finds the index of an object in the array, assuming that the array is sorted.

        This will use a comparator to do a binary-chop to find the index of the given
        element, if it exists. If the array isn't sorted, the behaviour of this
        method will be unpredictable.

        @param comparator           the comparator to use to compare the elements - see the sort()
                                    method for details about the form this object should take
        @param objectToLookFor      the object to search for
        @returns                    the index of the element, or -1 if it's not found
        @see addSorted, sort
    */
    template <class ElementComparator>
    int indexOfSorted (ElementComparator& comparator,
                       const ObjectClass* const objectToLookFor) const throw()
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
            else if (comparator.compareElements (objectToLookFor, this->elements [start]) == 0)
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
                else if (comparator.compareElements (objectToLookFor, this->elements [halfway]) >= 0)
                    start = halfway;
                else
                    end = halfway;
            }
        }
    }

    //==============================================================================
    /** Removes an object from the array.

        This will remove the object at a given index (optionally also
        deleting it) and move back all the subsequent objects to close the gap.
        If the index passed in is out-of-range, nothing will happen.

        @param indexToRemove    the index of the element to remove
        @param deleteObject     whether to delete the object that is removed
        @see removeObject, removeRange
    */
    void remove (const int indexToRemove,
                 const bool deleteObject = true)
    {
        lock.enter();
        ObjectClass* toDelete = 0;

        if (indexToRemove >= 0 && indexToRemove < numUsed)
        {
            ObjectClass** const e = this->elements + indexToRemove;

            if (deleteObject)
                toDelete = *e;

            --numUsed;
            const int numToShift = numUsed - indexToRemove;

            if (numToShift > 0)
                memmove (e, e + 1, numToShift * sizeof (ObjectClass*));

            if ((numUsed << 1) < this->numAllocated)
                minimiseStorageOverheads();
        }

        lock.exit();

        delete toDelete;
    }

    /** Removes a specified object from the array.

        If the item isn't found, no action is taken.

        @param objectToRemove   the object to try to remove
        @param deleteObject     whether to delete the object (if it's found)
        @see remove, removeRange
    */
    void removeObject (const ObjectClass* const objectToRemove,
                       const bool deleteObject = true)
    {
        lock.enter();
        ObjectClass** e = this->elements;

        for (int i = numUsed; --i >= 0;)
        {
            if (objectToRemove == *e)
            {
                remove ((int) (e - this->elements), deleteObject);
                break;
            }

            ++e;
        }

        lock.exit();
    }

    /** Removes a range of objects from the array.

        This will remove a set of objects, starting from the given index,
        and move any subsequent elements down to close the gap.

        If the range extends beyond the bounds of the array, it will
        be safely clipped to the size of the array.

        @param startIndex       the index of the first object to remove
        @param numberToRemove   how many objects should be removed
        @param deleteObjects    whether to delete the objects that get removed
        @see remove, removeObject
    */
    void removeRange (int startIndex,
                      const int numberToRemove,
                      const bool deleteObjects = true)
    {
        lock.enter();
        const int endIndex = jlimit (0, numUsed, startIndex + numberToRemove);
        startIndex = jlimit (0, numUsed, startIndex);

        if (endIndex > startIndex)
        {
            if (deleteObjects)
            {
                for (int i = startIndex; i < endIndex; ++i)
                {
                    delete this->elements [i];
                    this->elements [i] = 0; // (in case one of the destructors accesses this array and hits a dangling pointer)
                }
            }

            const int rangeSize = endIndex - startIndex;
            ObjectClass** e = this->elements + startIndex;
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

    /** Removes the last n objects from the array.

        @param howManyToRemove   how many objects to remove from the end of the array
        @param deleteObjects     whether to also delete the objects that are removed
        @see remove, removeObject, removeRange
    */
    void removeLast (int howManyToRemove = 1,
                     const bool deleteObjects = true)
    {
        lock.enter();

        if (howManyToRemove >= numUsed)
        {
            clear (deleteObjects);
        }
        else
        {
            while (--howManyToRemove >= 0)
                remove (numUsed - 1, deleteObjects);
        }

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

        if (index1 >= 0 && index1 < numUsed
             && index2 >= 0 && index2 < numUsed)
        {
            swapVariables (this->elements [index1],
                           this->elements [index2]);
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

            if (currentIndex >= 0 && currentIndex < numUsed)
            {
                if (newIndex < 0 || newIndex >= numUsed)
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

            lock.exit();
        }
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
        swapVariables <ObjectClass**> (this->elements, otherArray.elements);
        swapVariables <int> (this->numAllocated, otherArray.numAllocated);
        otherArray.lock.exit();
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
        @see sortArray, indexOfSorted
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

    // disallow copy constructor and assignment
    OwnedArray (const OwnedArray&);
    const OwnedArray& operator= (const OwnedArray&);
};


#endif   // __JUCE_OWNEDARRAY_JUCEHEADER__
