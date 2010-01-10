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

#ifndef __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__
#define __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__

#include "../core/juce_Atomic.h"


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
    p = 0;
    p2->foo();
    @endcode

    Once a new ReferenceCountedObject has been assigned to a pointer, be
    careful not to delete the object manually.

    @see ReferenceCountedObjectPtr, ReferenceCountedArray
*/
class JUCE_API  ReferenceCountedObject
{
public:
    //==============================================================================
    /** Increments the object's reference count.

        This is done automatically by the smart pointer, but is public just
        in case it's needed for nefarious purposes.
    */
    inline void incReferenceCount() throw()
    {
        Atomic::increment (refCounts);

        jassert (refCounts > 0);
    }

    /** Decreases the object's reference count.

        If the count gets to zero, the object will be deleted.
    */
    inline void decReferenceCount() throw()
    {
        jassert (refCounts > 0);

        if (Atomic::decrementAndReturn (refCounts) == 0)
            delete this;
    }

    /** Returns the object's current reference count. */
    inline int getReferenceCount() const throw()
    {
        return refCounts;
    }


protected:
    //==============================================================================
    /** Creates the reference-counted object (with an initial ref count of zero). */
    ReferenceCountedObject()
        : refCounts (0)
    {
    }

    /** Destructor. */
    virtual ~ReferenceCountedObject()
    {
        // it's dangerous to delete an object that's still referenced by something else!
        jassert (refCounts == 0);
    }

private:
    //==============================================================================
    int refCounts;
};



//==============================================================================
/**
    Used to point to an object of type ReferenceCountedObject.

    It's wise to use a typedef instead of typing out the templated name
    each time - e.g.

    typedef ReferenceCountedObjectPtr<MyClass> MyClassPtr;

    @see ReferenceCountedObject, ReferenceCountedObjectArray
*/
template <class ReferenceCountedObjectClass>
class ReferenceCountedObjectPtr
{
public:
    //==============================================================================
    /** Creates a pointer to a null object. */
    inline ReferenceCountedObjectPtr() throw()
        : referencedObject (0)
    {
    }

    /** Creates a pointer to an object.

        This will increment the object's reference-count if it is non-null.
    */
    inline ReferenceCountedObjectPtr (ReferenceCountedObjectClass* const refCountedObject) throw()
        : referencedObject (refCountedObject)
    {
        if (refCountedObject != 0)
            refCountedObject->incReferenceCount();
    }

    /** Copies another pointer.

        This will increment the object's reference-count (if it is non-null).
    */
    inline ReferenceCountedObjectPtr (const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& other) throw()
        : referencedObject (other.referencedObject)
    {
        if (referencedObject != 0)
            referencedObject->incReferenceCount();
    }

    /** Changes this pointer to point at a different object.

        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& operator= (const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& other)
    {
        ReferenceCountedObjectClass* const newObject = other.referencedObject;

        if (newObject != referencedObject)
        {
            if (newObject != 0)
                newObject->incReferenceCount();

            ReferenceCountedObjectClass* const oldObject = referencedObject;
            referencedObject = newObject;

            if (oldObject != 0)
                oldObject->decReferenceCount();
        }

        return *this;
    }

    /** Changes this pointer to point at a different object.

        The reference count of the old object is decremented, and it might be
        deleted if it hits zero. The new object's count is incremented.
    */
    const ReferenceCountedObjectPtr<ReferenceCountedObjectClass>& operator= (ReferenceCountedObjectClass* const newObject)
    {
        if (referencedObject != newObject)
        {
            if (newObject != 0)
                newObject->incReferenceCount();

            ReferenceCountedObjectClass* const oldObject = referencedObject;
            referencedObject = newObject;

            if (oldObject != 0)
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
        if (referencedObject != 0)
            referencedObject->decReferenceCount();
    }

    /** Returns the object that this pointer references.

        The pointer returned may be zero, of course.
    */
    inline operator ReferenceCountedObjectClass*() const throw()
    {
        return referencedObject;
    }

    /** Returns true if this pointer refers to the given object. */
    inline bool operator== (ReferenceCountedObjectClass* const object) const throw()
    {
        return referencedObject == object;
    }

    /** Returns true if this pointer doesn't refer to the given object. */
    inline bool operator!= (ReferenceCountedObjectClass* const object) const throw()
    {
        return referencedObject != object;
    }

    // the -> operator is called on the referenced object
    inline ReferenceCountedObjectClass* operator->() const throw()
    {
        return referencedObject;
    }


private:
    //==============================================================================
    ReferenceCountedObjectClass* referencedObject;
};


#endif   // __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__
