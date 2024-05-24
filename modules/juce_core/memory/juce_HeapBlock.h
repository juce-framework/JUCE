/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if ! (DOXYGEN || JUCE_EXCEPTIONS_DISABLED)
namespace HeapBlockHelper
{
    template <bool shouldThrow>
    struct ThrowOnFail          { static void checkPointer (void*) {} };

    template <>
    struct ThrowOnFail<true>    { static void checkPointer (void* data) { if (data == nullptr) throw std::bad_alloc(); } };
}
#endif

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
        int* temp = (int*) malloc (1024 * sizeof (int));
        memcpy (temp, xyz, 1024 * sizeof (int));
        free (temp);
        temp = (int*) calloc (2048 * sizeof (int));
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (int));
        free (temp);
    @endcode

    ..you could just write this:
    @code
        HeapBlock<int> temp (1024);
        memcpy (temp, xyz, 1024 * sizeof (int));
        temp.jcalloc (2048);
        temp[0] = 1234;
        memcpy (foobar, temp, 2048 * sizeof (int));
    @endcode

    The class is extremely lightweight, containing only a pointer to the
    data, and exposes malloc/realloc/calloc/free methods that do the same jobs
    as their less object-oriented counterparts. Despite adding safety, you probably
    won't sacrifice any performance by using this in place of normal pointers.

    The throwOnFailure template parameter can be set to true if you'd like the class
    to throw a std::bad_alloc exception when an allocation fails. If this is false,
    then a failed allocation will just leave the heapblock with a null pointer (assuming
    that the system's malloc() function doesn't throw).

    @see Array, OwnedArray, MemoryBlock

    @tags{Core}
*/
template <class ElementType, bool throwOnFailure = false>
class HeapBlock
{
private:
    template <class OtherElementType>
    using AllowConversion = std::enable_if_t<std::is_base_of_v<std::remove_pointer_t<ElementType>,
                                                               std::remove_pointer_t<OtherElementType>>>;

public:
    //==============================================================================
    /** Creates a HeapBlock which is initially just a null pointer.

        After creation, you can resize the array using the malloc(), calloc(),
        or realloc() methods.
    */
    HeapBlock() = default;

    /** Creates a HeapBlock containing a number of elements.

        The contents of the block are undefined, as it will have been created by a
        malloc call.

        If you want an array of zero values, you can use the calloc() method or the
        other constructor that takes an InitialisationState parameter.
    */
    template <typename SizeType, std::enable_if_t<std::is_convertible_v<SizeType, int>, int> = 0>
    explicit HeapBlock (SizeType numElements)
        : data (mallocWrapper (static_cast<size_t> (numElements) * sizeof (ElementType)))
    {
    }

    /** Creates a HeapBlock containing a number of elements.

        The initialiseToZero parameter determines whether the new memory should be cleared,
        or left uninitialised.
    */
    template <typename SizeType, std::enable_if_t<std::is_convertible_v<SizeType, int>, int> = 0>
    HeapBlock (SizeType numElements, bool initialiseToZero)
        : data (initialiseToZero ? callocWrapper (static_cast<size_t> (numElements), sizeof (ElementType))
                                 : mallocWrapper (static_cast<size_t> (numElements) * sizeof (ElementType)))
    {
    }

    /** Destructor.
        This will free the data, if any has been allocated.
    */
    ~HeapBlock()
    {
        std::free (data);
    }

    /** Move constructor */
    HeapBlock (HeapBlock&& other) noexcept
        : data (other.data)
    {
        other.data = nullptr;
    }

    /** Move assignment operator */
    HeapBlock& operator= (HeapBlock&& other) noexcept
    {
        std::swap (data, other.data);
        return *this;
    }

    /** Converting move constructor.
        Only enabled if this is a HeapBlock<Base*> and the other object is a HeapBlock<Derived*>,
        where std::is_base_of_v<Base, Derived> == true.
    */
    template <class OtherElementType, bool otherThrowOnFailure, typename = AllowConversion<OtherElementType>>
    HeapBlock (HeapBlock<OtherElementType, otherThrowOnFailure>&& other) noexcept
        : data (reinterpret_cast<ElementType*> (other.data))
    {
        other.data = nullptr;
    }

    /** Converting move assignment operator.
        Only enabled if this is a HeapBlock<Base*> and the other object is a HeapBlock<Derived*>,
        where std::is_base_of_v<Base, Derived> == true.
    */
    template <class OtherElementType, bool otherThrowOnFailure, typename = AllowConversion<OtherElementType>>
    HeapBlock& operator= (HeapBlock<OtherElementType, otherThrowOnFailure>&& other) noexcept
    {
        jfree();
        data = reinterpret_cast<ElementType*> (other.data);
        other.data = nullptr;
        return *this;
    }

    //==============================================================================
    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator ElementType*() const noexcept                            { return data; }

    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline ElementType* get() const noexcept                                 { return data; }

    /** Returns a raw pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline ElementType* getData() const noexcept                             { return data; }

    /** Returns a void pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator void*() const noexcept                                   { return static_cast<void*> (data); }

    /** Returns a void pointer to the allocated data.
        This may be a null pointer if the data hasn't yet been allocated, or if it has been
        freed by calling the free() method.
    */
    inline operator const void*() const noexcept                             { return static_cast<const void*> (data); }

    /** Lets you use indirect calls to the first element in the array.
        Obviously this will cause problems if the array hasn't been initialised, because it'll
        be referencing a null pointer.
    */
    inline ElementType* operator->() const  noexcept                         { return data; }

    /** Returns a reference to one of the data elements.
        Obviously there's no bounds-checking here, as this object is just a dumb pointer and
        has no idea of the size it currently has allocated.
    */
    template <typename IndexType>
    ElementType& operator[] (IndexType index) const noexcept                 { return data [index]; }

    /** Returns a pointer to a data element at an offset from the start of the array.
        This is the same as doing pointer arithmetic on the raw pointer itself.
    */
    template <typename IndexType>
    ElementType* operator+ (IndexType index) const noexcept                  { return data + index; }

    //==============================================================================
    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator== (const ElementType* otherPointer) const noexcept  { return otherPointer == data; }

    /** Compares the pointer with another pointer.
        This can be handy for checking whether this is a null pointer.
    */
    inline bool operator!= (const ElementType* otherPointer) const noexcept  { return otherPointer != data; }

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
    template <typename SizeType>
    void jmalloc (SizeType newNumElements, size_t elementSize = sizeof (ElementType))
    {
        std::free (data);
        data = mallocWrapper (static_cast<size_t> (newNumElements) * elementSize);
    }

    /** Allocates a specified amount of memory and clears it.
        This does the same job as the malloc() method, but clears the memory that it allocates.
    */
    template <typename SizeType>
    void jcalloc (SizeType newNumElements, const size_t elementSize = sizeof (ElementType))
    {
        std::free (data);
        data = callocWrapper (static_cast<size_t> (newNumElements), elementSize);
    }

    /** Allocates a specified amount of memory and optionally clears it.
        This does the same job as either malloc() or calloc(), depending on the
        initialiseToZero parameter.
    */
    template <typename SizeType>
    void allocate (SizeType newNumElements, bool initialiseToZero)
    {
        std::free (data);
        data = initialiseToZero ? callocWrapper (static_cast<size_t> (newNumElements), sizeof (ElementType))
                                : mallocWrapper (static_cast<size_t> (newNumElements) * sizeof (ElementType));
    }

    /** Re-allocates a specified amount of memory.

        The semantics of this method are the same as malloc() and calloc(), but it
        uses realloc() to keep as much of the existing data as possible.
    */
    template <typename SizeType>
    void jrealloc (SizeType newNumElements, size_t elementSize = sizeof (ElementType))
    {
        data = reallocWrapper (data, static_cast<size_t> (newNumElements) * elementSize);
    }

    /** Frees any currently-allocated data.
        This will free the data and reset this object to be a null pointer.
    */
    void jfree() noexcept
    {
        std::free (data);
        data = nullptr;
    }

#ifndef JUCE_NO_DEPRECATED_HEAPBLOCK_METHODS

    template <typename SizeType>
    [[deprecated]] void malloc(SizeType newNumElements, size_t elementSize = sizeof(ElementType)) {
        return jmalloc(newNumElements, elementSize);
    }

    /** Allocates a specified amount of memory and clears it.
        This does the same job as the malloc() method, but clears the memory that it
       allocates.
    */
    template <typename SizeType>
    [[deprecated]] void calloc(SizeType newNumElements,
                 const size_t elementSize = sizeof(ElementType)) {
        return jcalloc(newNumElements, elementSize);
    }

    /** Re-allocates a specified amount of memory.

        The semantics of this method are the same as malloc() and calloc(), but it
        uses realloc() to keep as much of the existing data as possible.
    */
    template <typename SizeType>
    [[deprecated]] void realloc(SizeType newNumElements, size_t elementSize = sizeof(ElementType)) {
        return jrealloc(newNumElements, elementSize);
    }

    /** Frees any currently-allocated data.
        This will free the data and reset this object to be a null pointer.
    */
    [[deprecated]] void free() noexcept {
        jfree();
    }

#endif

    /** Swaps this object's data with the data of another HeapBlock.
        The two objects simply exchange their data pointers.
    */
    template <bool otherBlockThrows>
    void swapWith (HeapBlock<ElementType, otherBlockThrows>& other) noexcept
    {
        std::swap (data, other.data);
    }

    /** This fills the block with zeros, up to the number of elements specified.
        Since the block has no way of knowing its own size, you must make sure that the number of
        elements you specify doesn't exceed the allocated size.
    */
    template <typename SizeType>
    void clear (SizeType numElements) noexcept
    {
        zeromem (data, sizeof (ElementType) * static_cast<size_t> (numElements));
    }

    /** This typedef can be used to get the type of the heapblock's elements. */
    using Type = ElementType;

private:
    //==============================================================================
    // Calls to malloc, calloc and realloc with zero size have implementation-defined
    // behaviour where either nullptr or a non-null pointer is returned.
    template <typename Functor>
    static ElementType* wrapper (size_t size, Functor&& f)
    {
        if (size == 0)
            return nullptr;

        auto* memory = static_cast<ElementType*> (f());

       #if JUCE_EXCEPTIONS_DISABLED
        jassert (memory != nullptr); // without exceptions, you'll need to find a better way to handle this failure case.
       #else
        HeapBlockHelper::ThrowOnFail<throwOnFailure>::checkPointer (memory);
       #endif

        return memory;
    }

    static ElementType* mallocWrapper (size_t size)
    {
        return wrapper (size, [size] { return std::malloc (size); });
    }

    static ElementType* callocWrapper (size_t num, size_t size)
    {
        return wrapper (num * size, [num, size] { return std::calloc (num, size); });
    }

    static ElementType* reallocWrapper (void* ptr, size_t newSize)
    {
        return wrapper (newSize, [ptr, newSize] { return std::realloc (ptr, newSize); });
    }

    template <class OtherElementType, bool otherThrowOnFailure>
    friend class HeapBlock;

    //==============================================================================
    ElementType* data = nullptr;

   #if ! (defined (JUCE_DLL) || defined (JUCE_DLL_BUILD))
    JUCE_DECLARE_NON_COPYABLE (HeapBlock)
    JUCE_PREVENT_HEAP_ALLOCATION // Creating a 'new HeapBlock' would be missing the point!
   #endif
};

} // namespace juce
