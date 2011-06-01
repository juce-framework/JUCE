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

#ifndef __JUCE_WEAKREFERENCE_JUCEHEADER__
#define __JUCE_WEAKREFERENCE_JUCEHEADER__

#include "juce_ReferenceCountedObject.h"


//==============================================================================
/**
    This class acts as a pointer which will automatically become null if the object
    to which it points is deleted.

    To accomplish this, the source object needs to cooperate by performing a couple of simple tasks.
    It must provide a getWeakReference() method and embed a WeakReference::Master object, which stores
    a shared pointer object. It must also clear this master pointer when it's getting deleted.

    E.g.
    @code
    class MyObject
    {
    public:
        MyObject()
        {
            // If you're planning on using your WeakReferences in a multi-threaded situation, you may choose
            // to call getWeakReference() here in the constructor, which will pre-initialise it, avoiding an
            // (extremely unlikely) race condition that could occur if multiple threads overlap while making
            // the first call to getWeakReference().
        }

        ~MyObject()
        {
            // This will zero all the references - you need to call this in your destructor.
            masterReference.clear();
        }

        // Your object must provide a method that looks pretty much identical to this (except
        // for the templated class name, of course).
        const WeakReference<MyObject>::SharedRef& getWeakReference()
        {
            return masterReference (this);
        }

    private:
        // You need to embed one of these inside your object. It can be private.
        WeakReference<MyObject>::Master masterReference;
    };

    // Here's an example of using a pointer..

    MyObject* n = new MyObject();
    WeakReference<MyObject> myObjectRef = n;

    MyObject* pointer1 = myObjectRef;  // returns a valid pointer to 'n'
    delete n;
    MyObject* pointer2 = myObjectRef;  // returns a null pointer
    @endcode

    @see WeakReference::Master
*/
template <class ObjectType, class ReferenceCountingType = ReferenceCountedObject>
class WeakReference
{
public:
    /** Creates a null SafePointer. */
    inline WeakReference() noexcept {}

    /** Creates a WeakReference that points at the given object. */
    WeakReference (ObjectType* const object)  : holder (object != nullptr ? object->getWeakReference() : nullptr) {}

    /** Creates a copy of another WeakReference. */
    WeakReference (const WeakReference& other) noexcept         : holder (other.holder) {}

    /** Copies another pointer to this one. */
    WeakReference& operator= (const WeakReference& other)       { holder = other.holder; return *this; }

    /** Copies another pointer to this one. */
    WeakReference& operator= (ObjectType* const newObject)      { holder = (newObject != nullptr) ? newObject->getWeakReference() : nullptr; return *this; }

    /** Returns the object that this pointer refers to, or null if the object no longer exists. */
    ObjectType* get() const noexcept                            { return holder != nullptr ? holder->get() : nullptr; }

    /** Returns the object that this pointer refers to, or null if the object no longer exists. */
    operator ObjectType*() const noexcept                       { return get(); }

    /** Returns the object that this pointer refers to, or null if the object no longer exists. */
    ObjectType* operator->() noexcept                           { return get(); }

    /** Returns the object that this pointer refers to, or null if the object no longer exists. */
    const ObjectType* operator->() const noexcept               { return get(); }

    /** This returns true if this reference has been pointing at an object, but that object has
        since been deleted.

        If this reference was only ever pointing at a null pointer, this will return false. Using
        operator=() to make this refer to a different object will reset this flag to match the status
        of the reference from which you're copying.
    */
    bool wasObjectDeleted() const noexcept                      { return holder != nullptr && holder->get() == nullptr; }

    bool operator== (ObjectType* const object) const noexcept   { return get() == object; }
    bool operator!= (ObjectType* const object) const noexcept   { return get() != object; }

    //==============================================================================
    /** This class is used internally by the WeakReference class - don't use it directly
        in your code!
        @see WeakReference
    */
    class SharedPointer   : public ReferenceCountingType
    {
    public:
        explicit SharedPointer (ObjectType* const owner_) noexcept : owner (owner_) {}

        inline ObjectType* get() const noexcept     { return owner; }
        void clearPointer() noexcept                { owner = nullptr; }

    private:
        ObjectType* volatile owner;

        JUCE_DECLARE_NON_COPYABLE (SharedPointer);
    };

    typedef ReferenceCountedObjectPtr<SharedPointer> SharedRef;

    //==============================================================================
    /**
        This class is embedded inside an object to which you want to attach WeakReference pointers.
        See the WeakReference class notes for an example of how to use this class.
        @see WeakReference
    */
    class Master
    {
    public:
        Master() noexcept {}

        ~Master()
        {
            // You must remember to call clear() in your source object's destructor! See the notes
            // for the WeakReference class for an example of how to do this.
            jassert (sharedPointer == nullptr || sharedPointer->get() == nullptr);
        }

        /** The first call to this method will create an internal object that is shared by all weak
            references to the object.
            You need to call this from your main object's getWeakReference() method - see the WeakReference
            class notes for an example.
         */
        const SharedRef& operator() (ObjectType* const object)
        {
            if (sharedPointer == nullptr)
            {
                sharedPointer = new SharedPointer (object);
            }
            else
            {
                // You're trying to create a weak reference to an object that has already been deleted!!
                jassert (sharedPointer->get() != nullptr);
            }

            return sharedPointer;
        }

        /** The object that owns this master pointer should call this before it gets destroyed,
            to zero all the references to this object that may be out there. See the WeakReference
            class notes for an example of how to do this.
        */
        void clear()
        {
            if (sharedPointer != nullptr)
                sharedPointer->clearPointer();
        }

    private:
        SharedRef sharedPointer;

        JUCE_DECLARE_NON_COPYABLE (Master);
    };

private:
    SharedRef holder;
};


#endif   // __JUCE_WEAKREFERENCE_JUCEHEADER__
