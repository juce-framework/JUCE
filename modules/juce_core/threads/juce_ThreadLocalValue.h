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

#ifndef __JUCE_THREADLOCALVALUE_JUCEHEADER__
#define __JUCE_THREADLOCALVALUE_JUCEHEADER__


//==============================================================================
/**
    Provides cross-platform support for thread-local objects.

    This class holds an internal list of objects of the templated type, keeping
    an instance for each thread that requests one. The first time a thread attempts
    to access its value, an object is created and added to the list for that thread.

    The templated class for your value could be a primitive type, or any class that
    has a default constructor.

    Once a thread has accessed its object, that object will not be deleted until the
    ThreadLocalValue object itself is deleted, even if its thread exits before that.
    But, because thread ID numbers are used to identify threads, and OSes often re-use
    these ID numbers, value objects will often be implicitly re-used by new threads whose
    ID number is the same as one that was used by an earlier thread.
*/
template <typename Type>
class ThreadLocalValue
{
public:
    /** */
    ThreadLocalValue() noexcept
    {
    }

    /** Destructor.
        When this object is deleted, all the value objects for all threads will be deleted.
    */
    ~ThreadLocalValue()
    {
        for (ObjectHolder* o = first.value; o != nullptr;)
        {
            ObjectHolder* const next = o->next;
            delete o;
            o = next;
        }
    }

    /** Returns a reference to this thread's instance of the value.
        Note that the first time a thread tries to access the value, an instance of the
        value object will be created - so if your value's class has a non-trivial
        constructor, be aware that this method could invoke it.
    */
    Type& operator*() const noexcept    { return get(); }

    /** Returns a pointer to this thread's instance of the value.
        Note that the first time a thread tries to access the value, an instance of the
        value object will be created - so if your value's class has a non-trivial
        constructor, be aware that this method could invoke it.
    */
    operator Type*() const noexcept     { return &get(); }

    /** Accesses a method or field of the value object.
        Note that the first time a thread tries to access the value, an instance of the
        value object will be created - so if your value's class has a non-trivial
        constructor, be aware that this method could invoke it.
    */
    Type* operator->() const noexcept   { return &get(); }

    /** Returns a reference to this thread's instance of the value.
        Note that the first time a thread tries to access the value, an instance of the
        value object will be created - so if your value's class has a non-trivial
        constructor, be aware that this method could invoke it.
    */
    Type& get() const noexcept
    {
        const Thread::ThreadID threadId = Thread::getCurrentThreadId();

        for (ObjectHolder* o = first.get(); o != nullptr; o = o->next)
            if (o->threadId == threadId)
                return o->object;

        ObjectHolder* const newObject = new ObjectHolder (threadId);

        do
        {
            newObject->next = first.get();
        }
        while (! first.compareAndSetBool (newObject, newObject->next));

        return newObject->object;
    }

private:
    //==============================================================================
    struct ObjectHolder
    {
        ObjectHolder (const Thread::ThreadID& threadId_)
            : threadId (threadId_), object()
        {}

        const Thread::ThreadID threadId;
        ObjectHolder* next;
        Type object;

        JUCE_DECLARE_NON_COPYABLE (ObjectHolder);
    };

    mutable Atomic<ObjectHolder*> first;

    JUCE_DECLARE_NON_COPYABLE (ThreadLocalValue);
};


#endif   // __JUCE_THREADLOCALVALUE_JUCEHEADER__
