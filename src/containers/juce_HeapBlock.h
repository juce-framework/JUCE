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

#ifndef __JUCE_HEAPBLOCK_JUCEHEADER__
#define __JUCE_HEAPBLOCK_JUCEHEADER__


//==============================================================================
/**
    Very simple container class to hold a pointer to some data on the heap.

    When you need to allocate some heap storage for something, always try to use
    this class instead of allocating the memory directly using malloc/free.

    A HeapBlock<char> object can be treated in pretty much exactly the same way
    as an char*, but as long as you allocate it on the stack or as a class member,
    it's almost impossible for it to leak memory.

    It also makes your code much more concise and readable than doing the same thing
    using direct allocations,

    E.g. instead of this:
    @code
        int* temp = (int*) juce_malloc (1024 * sizeof (int));
        memcpy (temp, xyz, 1024 * sizeof (int));
        juce_free (temp);
        temp = (int*) juce_calloc (2048 * sizeof (int));
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (int));
        juce_free (temp);
    @endcode

    ..you could just write this:
    @code
        HeapBlock <int> temp (1024);
        memcpy (temp, xyz, 1024 * sizeof (int));
        temp.calloc (2048);
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (int));
    @endcode

    The class is extremely lightweight, containing only a pointer to the
    data, and exposes malloc/realloc/calloc/free methods that do the same jobs
    as their less object-oriented counterparts. Despite adding safety, you probably
    won't sacrifice any performance by using this in place of normal pointers.

    @see Array, OwnedArray, MemoryBlock
*/
template <class ElementType>
class HeapBlock
{
public:
    //==============================================================================
    /** Creates a HeapBlock which is initially just a null pointer.

        After creation, you can resize the array using the malloc(), calloc(),
        or realloc() methods.
    */
    HeapBlock()  : data (0)
    {
    }

    /** Creates a HeapBlock containing a number of elements.

        The contents of the block are undefined, as it will have been created by a
        malloc call.

        If you want an array of zero values, you can use the calloc() method instead.
    */
    HeapBlock (const size_t numElements)
        : data ((ElementType*) ::juce_malloc (numElements * sizeof (ElementType)))
    {
    }

    /** Destructor.

        This will free the data, if any has been allocated.
    */
    ~HeapBlock()
    {
        ::juce_free (data);
    }

    //==============================================================================
    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator ElementType*() const                                    { return data; }

    /** Returns a void pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator void*() const                                           { return (void*) data; }

    /** Lets you use indirect calls to the first element in the array.
        Obviously this will cause problems if the array hasn't been initialised, because it'll
        be referencing a null pointer.
    */
    inline ElementType* operator->() const                                  { return data; }

    /** Returns a pointer to the data by casting it to any type you need.
    */
    template <class CastType>
    inline operator CastType*() const                                       { return (CastType*) data; }

    /** Returns a reference to one of the data elements.
        Obviously there's no bounds-checking here, as this object is just a dumb pointer and
        has no idea of the size it currently has allocated.
    */
    template <typename IndexType>
    inline ElementType& operator[] (IndexType index) const                  { return data [index]; }

    /** Returns a pointer to a data element at an offset from the start of the array.
        This is the same as doing pointer arithmetic on the raw pointer itself.
    */
    template <typename IndexType>
    inline ElementType* operator+ (IndexType index) const                   { return data + index; }

    /** Returns a reference to the raw data pointer.
        Beware that the pointer returned here will become invalid as soon as you call
        any of the allocator methods on this object!
    */
    inline ElementType** operator&() const                                  { return (ElementType**) &data; }

    //==============================================================================
    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator== (const ElementType* const otherPointer) const    { return otherPointer == data; }

    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator!= (const ElementType* const otherPointer) const    { return otherPointer != data; }

    //==============================================================================
    /** Allocates a specified amount of memory.

        This uses the normal malloc to allocate an amount of memory for this object.
        Any previously allocated memory will be freed by this method.

        The number of bytes allocated will be (newNumElements * elementSize). Normally
        you wouldn't need to specify the second parameter, but it can be handy if you need
        to allocate a size in bytes rather than in terms of the number of elements.

        The data that is allocated will be freed when this object is deleted, or when you
        call free() or any of the allocation methods.
    */
    void malloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        ::juce_free (data);
        data = (ElementType*) ::juce_malloc (newNumElements * elementSize);
    }

    /** Allocates a specified amount of memory and clears it.
        This does the same job as the malloc() method, but clears the memory that it allocates.
    */
    void calloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        ::juce_free (data);
        data = (ElementType*) ::juce_calloc (newNumElements * elementSize);
    }

    /** Allocates a specified amount of memory and optionally clears it.
        This does the same job as either malloc() or calloc(), depending on the
        initialiseToZero parameter.
    */
    void allocate (const size_t newNumElements, const bool initialiseToZero)
    {
        ::juce_free (data);

        if (initialiseToZero)
            data = (ElementType*) ::juce_calloc (newNumElements * sizeof (ElementType));
        else
            data = (ElementType*) ::juce_malloc (newNumElements * sizeof (ElementType));
    }

    /** Re-allocates a specified amount of memory.

        The semantics of this method are the same as malloc() and calloc(), but it
        uses realloc() to keep as much of the existing data as possible.
    */
    void realloc (const size_t newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        if (data == 0)
            data = (ElementType*) ::juce_malloc (newNumElements * elementSize);
        else
            data = (ElementType*) ::juce_realloc (data, newNumElements * elementSize);
    }

    /** Frees any currently-allocated data.
        This will free the data and reset this object to be a null pointer.
    */
    void free()
    {
        ::juce_free (data);
        data = 0;
    }

    /** Swaps this object's data with the data of another HeapBlock.
        The two objects simply exchange their data pointers.
    */
    void swapWith (HeapBlock <ElementType>& other)
    {
        swapVariables (data, other.data);
    }


private:
    //==============================================================================
    ElementType* data;

    HeapBlock (const HeapBlock&);
    const HeapBlock& operator= (const HeapBlock&);
};


#endif   // __JUCE_HEAPBLOCK_JUCEHEADER__
