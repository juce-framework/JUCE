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

#ifndef __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__
#define __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__

#include "juce_Atomic.h"


//==============================================================================
/**
    Adds reference-counting to an object.

    To add reference-counting to a class, derive it from this class, and
    use the ReferenceCountedObjectPtr class to point to it.

    e.g. @code
    class MyClass : public ReferenceCountedObject
    {
        void foo();

        // This is a neat way of declaring a typedef for a pointer class,
        // rather than typing out the full templated name each time..
        typedef ReferenceCountedObjectPtr<MyClass> Ptr;
    };

    MyClass::Ptr p = new MyClass();
    MyClass::Ptr p2 = p;
    p = nullptr;
    p2->foo();
    @endcode

    Once a new ReferenceCountedObject has been assigned to a pointer, be
    careful not to delete the object manually.

    This class uses an Atomic<int> value to hold the reference count, so that it
    the pointers can be passed between threads safely. For a faster but non-thread-safe
    version, use SingleThreadedReferenceCountedObject instead.

    @see ReferenceCountedObjectPtr, ReferenceCountedArray, SingleThreadedReferenceCountedObject
*/
class JUCE_API  ReferenceCountedObject
{
public:
    //==============================================================================
    /** Increments the object's reference count.

        This is done automatically by the smart pointer, but is public just
        in case it's needed for nefarious purposes.
    */
    inline void incReferenceCount() noexcept
    {
        ++refCount;
    }

    /** Decreases the object's reference count.

        If the count gets to zero, the object will be deleted.
    */
    inline void decReferenceCount() noexcept
    {
        jassert (getReferenceCount() > 0);

        if (--refCount == 0)
            delete this;
    }

    /** Returns the object's current reference count. */
    inline int getReferenceCount() const noexcept       { return refCount.get(); }


protected:
    //==============================================================================
    /** Creates the reference-counted object (with an initial ref count of zero). */
    ReferenceCountedObject()
    {
    }

    /** Destructor. */
    virtual ~ReferenceCountedObject()
    {
        // it's dangerous to delete an object that's still referenced by something else!
        jassert (getReferenceCount() == 0);
    }

    /** Resets the reference count to zero without deleting the object.
        You should probably never need to use this!
    */
    void resetReferenceCount() noexcept
    {
        refCount = 0;
    }

private:
    //==============================================================================
    Atomic <int> refCount;

    JUCE_DECLARE_NON_COPYABLE (ReferenceCountedObject)
};


//==============================================================================
/**
    Adds reference-counting to an object.

    This is efectively a version of the ReferenceCountedObject class, but which
    uses a non-atomic counter, and so is not thread-safe (but which will be more
    efficient).
    For more details on how to use it, see the ReferenceCountedObject class notes.

    @see ReferenceCountedObject, ReferenceCountedObjectPtr, ReferenceCountedArray
*/
class JUCE_API  SingleThreadedReferenceCountedObject
{
public:
    //==============================================================================
    /** Increments the object's reference count.

        This is done automatically by the smart pointer, but is public just
        in case it's needed for nefarious purposes.
    */
    inline void incReferenceCount() noexcept
    {
        ++refCount;
    }

    /** Decreases the object's reference count.

        If the count gets to zero, the object will be deleted.
    */
    inline void decReferenceCount() noexcept
    {
        jassert (getReferenceCount() > 0);

        if (--refCount == 0)
            delete this;
    }

    /** Returns the object's current reference count. */
    inline int getReferenceCount() const noexcept       { return refCount; }


protected:
    //==============================================================================
    /** Creates the reference-counted object (with an initial ref count of zero). */
    SingleThreadedReferenceCountedObject() : refCount (0)  {}

    /** Destructor. */
    virtual ~SingleThreadedReferenceCountedObject()
    {
        // it's dangerous to delete an object that's still referenced by something else!
        jassert (getReferenceCount() == 0);
    }

private:
    //==============================================================================
    int refCount;

    JUCE_DECLARE_NON_COPYABLE (SingleThreadedReferenceCountedObject)
};


//==============================================================================
/**
    A smart-pointer class which points to a reference-counted object.

    The template parameter specifies the class of the object you want to point to - the easiest
    way to make a class reference-countable is to simply make it inherit from ReferenceCountedObject,
    but if you need to, you could roll your own reference-countable class by implementing a pair of
    mathods called incReferenceCount() and decReferenceCount().

    When using this class, you'll probably want to create a typedef to abbreviate the full
    templated name - e.g.
    @code typedef ReferenceCountedObjectPtr<MyClass> MyClassPtr;@endcode

    @see ReferenceCountedObject, ReferenceCountedObjectArray
*/
template <class ReferenceCountedObjectClass>
class ReferenceCountedObjectPtr
{
public:
    /** The class being referenced by this pointer. */
    typedef ReferenceCountedObjectClass ReferencedType;

    //==============================================================================
    /** Creates a pointer to a null object. */
    inline ReferenceCountedObjectPtr() noexcept
        : referencedObject (nullptr)
    {
    }

    /** Creates a pointer to an object.

        This will increment the object's reference-count if it is non-null.
    */
    inline ReferenceCountedObjectPtr (ReferenceCountedObjectClass* const refCountedObject) noexcept
        : referencedObject (refCountedObject)
    {
        if (refCountedObject != nullptr)
            refCountedObject->incReferenceCount();
    }

    /** Copies another pointer.
        This will increment the object's reference-count (if it is non-null).
    */
    inline ReferenceCountedObjectPtr (const ReferenceCountedObjectPtr& other) noexcept
        : referencedObject (other.referencedObject)
    {
        if (referencedObject != nullptr)
            referencedObject->incReferenceCount();
    }

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    /** Takes-over the object from another pointer. */
    inline ReferenceCountedObjectPtr (ReferenceCountedObjectPtr&& other) noexcept
        : referencedObject (other.referencedObject)
    {
        other.referencedObject = nullptr;
    }
   #endif

    /** Copies another pointer.
        This will increment the object's reference-count (if it is non-null).
    */
    template <class DerivedClass>
    inline ReferenceCountedObjectPtr (const ReferenceCountedObjectPtr<DerivedClass>& other) noexcept
        : referencedObject (static_cast <ReferenceCountedObjectClass*> (other.get()))
    {
        if (referencedObject != nullptr)
            referencedObject->incReferenceCount();
    }

    /** Changes this pointer to point at a different object.

        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    ReferenceCountedObjectPtr& operator= (const ReferenceCountedObjectPtr& other)
    {
        return operator= (other.referencedObject);
    }

    /** Changes this pointer to point at a different object.

        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    template <class DerivedClass>
    ReferenceCountedObjectPtr& operator= (const ReferenceCountedObjectPtr<DerivedClass>& other)
    {
        return operator= (static_cast <ReferenceCountedObjectClass*> (other.get()));
    }

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    /** Takes-over the object from another pointer. */
    ReferenceCountedObjectPtr& operator= (ReferenceCountedObjectPtr&& other)
    {
        std::swap (referencedObject, other.referencedObject);
        return *this;
    }
   #endif

    /** Changes this pointer to point at a different object.

        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    ReferenceCountedObjectPtr& operator= (ReferenceCountedObjectClass* const newObject)
    {
        if (referencedObject != newObject)
        {
            if (newObject != nullptr)
                newObject->incReferenceCount();

            ReferenceCountedObjectClass* const oldObject = referencedObject;
            referencedObject = newObject;

            if (oldObject != nullptr)
                oldObject->decReferenceCount();
        }

        return *this;
    }

    /** Destructor.

        This will decrement the object's reference-count, and may delete it if it
        gets to zero.
    */
    inline ~ReferenceCountedObjectPtr()
    {
        if (referencedObject != nullptr)
            referencedObject->decReferenceCount();
    }

    /** Returns the object that this pointer references.
        The pointer returned may be zero, of course.
    */
    inline operator ReferenceCountedObjectClass*() const noexcept
    {
        return referencedObject;
    }

    // the -> operator is called on the referenced object
    inline ReferenceCountedObjectClass* operator->() const noexcept
    {
        return referencedObject;
    }

    /** Returns the object that this pointer references.
        The pointer returned may be zero, of course.
    */
    inline ReferenceCountedObjectClass* get() const noexcept
    {
        return referencedObject;
    }

    /** Returns the object that this pointer references.
        The pointer returned may be zero, of course.
    */
    inline ReferenceCountedObjectClass* getObject() const noexcept
    {
        return referencedObject;
    }

private:
    //==============================================================================
    ReferenceCountedObjectClass* referencedObject;
};


/** Compares two ReferenceCountedObjectPointers. */
template <class ReferenceCountedObjectClass>
bool operator== (const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& object1, ReferenceCountedObjectClass* const object2) noexcept
{
    return object1.get() == object2;
}

/** Compares two ReferenceCountedObjectPointers. */
template <class ReferenceCountedObjectClass>
bool operator== (const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& object1, const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& object2) noexcept
{
    return object1.get() == object2.get();
}

/** Compares two ReferenceCountedObjectPointers. */
template <class ReferenceCountedObjectClass>
bool operator== (ReferenceCountedObjectClass* object1, ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& object2) noexcept
{
    return object1 == object2.get();
}

/** Compares two ReferenceCountedObjectPointers. */
template <class ReferenceCountedObjectClass>
bool operator!= (const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& object1, const ReferenceCountedObjectClass* object2) noexcept
{
    return object1.get() != object2;
}

/** Compares two ReferenceCountedObjectPointers. */
template <class ReferenceCountedObjectClass>
bool operator!= (const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& object1, ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& object2) noexcept
{
    return object1.get() != object2.get();
}

/** Compares two ReferenceCountedObjectPointers. */
template <class ReferenceCountedObjectClass>
bool operator!= (ReferenceCountedObjectClass* object1, ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& object2) noexcept
{
    return object1 != object2.get();
}


#endif   // __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__
