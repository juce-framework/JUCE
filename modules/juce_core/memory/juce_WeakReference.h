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
    It must embed a WeakReference::Master object, which stores a shared pointer object, and must clear
    this master pointer in its destructor.

    E.g.
    @code
    class MyObject
    {
    public:
        MyObject()
        {
            // If you're planning on using your WeakReferences in a multi-threaded situation, you may choose
            // to create a WeakReference to the object here in the constructor, which will pre-initialise the
            // embedded object, avoiding an (extremely unlikely) race condition that could occur if multiple
            // threads overlap while creating the first WeakReference to it.
        }

        ~MyObject()
        {
            // This will zero all the references - you need to call this in your destructor.
            masterReference.clear();
        }

    private:
        // You need to embed a variable of this type, with the name "masterReference" inside your object. If the
        // variable is not public, you should make your class a friend of WeakReference<MyObject> so that the
        // WeakReference class can access it.
        WeakReference<MyObject>::Master masterReference;
        friend class WeakReference<MyObject>;
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
    WeakReference (ObjectType* const object)  : holder (getRef (object)) {}

    /** Creates a copy of another WeakReference. */
    WeakReference (const WeakReference& other) noexcept         : holder (other.holder) {}

    /** Copies another pointer to this one. */
    WeakReference& operator= (const WeakReference& other)       { holder = other.holder; return *this; }

    /** Copies another pointer to this one. */
    WeakReference& operator= (ObjectType* const newObject)      { holder = getRef (newObject); return *this; }

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    WeakReference (WeakReference&& other) noexcept              : holder (static_cast <SharedRef&&> (other.holder)) {}
    WeakReference& operator= (WeakReference&& other) noexcept   { holder = static_cast <SharedRef&&> (other.holder); return *this; }
   #endif

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
        explicit SharedPointer (ObjectType* const obj) noexcept : owner (obj) {}

        inline ObjectType* get() const noexcept     { return owner; }
        void clearPointer() noexcept                { owner = nullptr; }

    private:
        ObjectType* volatile owner;

        JUCE_DECLARE_NON_COPYABLE (SharedPointer)
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
        */
        SharedPointer* getSharedPointer (ObjectType* const object)
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

        JUCE_DECLARE_NON_COPYABLE (Master)
    };

private:
    SharedRef holder;

    static inline SharedPointer* getRef (ObjectType* const o)
    {
        return (o != nullptr) ? o->masterReference.getSharedPointer (o) : nullptr;
    }
};


#endif   // __JUCE_WEAKREFERENCE_JUCEHEADER__
