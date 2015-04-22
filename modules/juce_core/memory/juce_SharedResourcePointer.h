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

#ifndef JUCE_SHAREDRESOURCEPOINTER_H_INCLUDED
#define JUCE_SHAREDRESOURCEPOINTER_H_INCLUDED


//==============================================================================
/**
    A smart-pointer that automatically creates and manages the lifetime of a
    shared static instance of a class.

    The SharedObjectType template type indicates the class to use for the shared
    object - the only requirements on this class are that it must have a public
    default constructor and destructor.

    The SharedResourcePointer offers a pattern that differs from using a singleton or
    static instance of an object, because it uses reference-counting to make sure that
    the underlying shared object is automatically created/destroyed according to the
    number of SharedResourcePointer objects that exist. When the last one is deleted,
    the underlying object is also immediately destroyed. This allows you to use scoping
    to manage the lifetime of a shared resource.

    Note: the construction/deletion of the shared object must not involve any
    code that makes recursive calls to a SharedResourcePointer, or you'll cause
    a deadlock.

    Example:
    @code
    // An example of a class that contains the shared data you want to use.
    struct MySharedData
    {
        // There's no need to ever create an instance of this class directly yourself,
        // but it does need a public constructor that does the initialisation.
        MySharedData()
        {
            sharedStuff = generateHeavyweightStuff();
        }

        Array<SomeKindOfData> sharedStuff;
    };

    struct DataUserClass
    {
        DataUserClass()
        {
            // Multiple instances of the DataUserClass will all have the same
            // shared common instance of MySharedData referenced by their sharedData
            // member variables.
            useSharedStuff (sharedData->sharedStuff);
        }

        // By keeping this pointer as a member variable, the shared resource
        // is guaranteed to be available for as long as the DataUserClass object.
        SharedResourcePointer<MySharedData> sharedData;
    };

    @endcode
 */
template <typename SharedObjectType>
class SharedResourcePointer
{
public:
    /** Creates an instance of the shared object.
        If other SharedResourcePointer objects for this type already exist, then
        this one will simply point to the same shared object that they are already
        using. Otherwise, if this is the first SharedResourcePointer to be created,
        then a shared object will be created automatically.
    */
    SharedResourcePointer()
    {
        SharedObjectHolder& holder = getSharedObjectHolder();
        const SpinLock::ScopedLockType sl (holder.lock);

        if (++(holder.refCount) == 1)
            holder.sharedInstance = new SharedObjectType();

        sharedObject = holder.sharedInstance;
    }

    /** Destructor.
        If no other SharedResourcePointer objects exist, this will also delete
        the shared object to which it refers.
    */
    ~SharedResourcePointer()
    {
        SharedObjectHolder& holder = getSharedObjectHolder();
        const SpinLock::ScopedLockType sl (holder.lock);

        if (--(holder.refCount) == 0)
            holder.sharedInstance = nullptr;
    }

    /** Returns the shared object. */
    operator SharedObjectType*() const noexcept         { return sharedObject; }

    /** Returns the shared object. */
    SharedObjectType& get() const noexcept              { return *sharedObject; }

    /** Returns the object that this pointer references.
        The pointer returned may be zero, of course.
    */
    SharedObjectType& getObject() const noexcept        { return *sharedObject; }

    SharedObjectType* operator->() const noexcept       { return sharedObject; }

private:
    struct SharedObjectHolder  : public ReferenceCountedObject
    {
        SpinLock lock;
        ScopedPointer<SharedObjectType> sharedInstance;
        int refCount;
    };

    static SharedObjectHolder& getSharedObjectHolder() noexcept
    {
        static void* holder [(sizeof (SharedObjectHolder) + sizeof(void*) - 1) / sizeof(void*)] = { 0 };
        return *reinterpret_cast<SharedObjectHolder*> (holder);
    }

    SharedObjectType* sharedObject;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SharedResourcePointer)
};


#endif   // JUCE_SHAREDRESOURCEPOINTER_H_INCLUDED
