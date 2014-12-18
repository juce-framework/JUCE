/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_WEAKREFERENCE_H_INCLUDED
#define JUCE_WEAKREFERENCE_H_INCLUDED


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

        ~Master() noexcept
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
        void clear() noexcept
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


#endif   // JUCE_WEAKREFERENCE_H_INCLUDED
