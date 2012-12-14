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

#ifndef __JUCE_SCOPEDLOCK_JUCEHEADER__
#define __JUCE_SCOPEDLOCK_JUCEHEADER__


//==============================================================================
/**
    Automatically locks and unlocks a mutex object.

    Use one of these as a local variable to provide RAII-based locking of a mutex.

    The templated class could be a CriticalSection, SpinLock, or anything else that
    provides enter() and exit() methods.

    e.g. @code
    CriticalSection myCriticalSection;

    for (;;)
    {
        const GenericScopedLock<CriticalSection> myScopedLock (myCriticalSection);
        // myCriticalSection is now locked

        ...do some stuff...

        // myCriticalSection gets unlocked here.
    }
    @endcode

    @see GenericScopedUnlock, CriticalSection, SpinLock, ScopedLock, ScopedUnlock
*/
template <class LockType>
class GenericScopedLock
{
public:
    //==============================================================================
    /** Creates a GenericScopedLock.

        As soon as it is created, this will acquire the lock, and when the GenericScopedLock
        object is deleted, the lock will be released.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline explicit GenericScopedLock (const LockType& lock) noexcept : lock_ (lock)     { lock.enter(); }

    /** Destructor.
        The lock will be released when the destructor is called.
        Make sure this object is created and deleted by the same thread, otherwise there are
        no guarantees what will happen!
    */
    inline ~GenericScopedLock() noexcept                                                 { lock_.exit(); }

private:
    //==============================================================================
    const LockType& lock_;

    JUCE_DECLARE_NON_COPYABLE (GenericScopedLock)
};


//==============================================================================
/**
    Automatically unlocks and re-locks a mutex object.

    This is the reverse of a GenericScopedLock object - instead of locking the mutex
    for the lifetime of this object, it unlocks it.

    Make sure you don't try to unlock mutexes that aren't actually locked!

    e.g. @code

    CriticalSection myCriticalSection;

    for (;;)
    {
        const GenericScopedLock<CriticalSection> myScopedLock (myCriticalSection);
        // myCriticalSection is now locked

        ... do some stuff with it locked ..

        while (xyz)
        {
            ... do some stuff with it locked ..

            const GenericScopedUnlock<CriticalSection> unlocker (myCriticalSection);

            // myCriticalSection is now unlocked for the remainder of this block,
            // and re-locked at the end.

            ...do some stuff with it unlocked ...
        }

        // myCriticalSection gets unlocked here.
    }
    @endcode

    @see GenericScopedLock, CriticalSection, ScopedLock, ScopedUnlock
*/
template <class LockType>
class GenericScopedUnlock
{
public:
    //==============================================================================
    /** Creates a GenericScopedUnlock.

        As soon as it is created, this will unlock the CriticalSection, and
        when the ScopedLock object is deleted, the CriticalSection will
        be re-locked.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline explicit GenericScopedUnlock (const LockType& lock) noexcept : lock_ (lock)   { lock.exit(); }

    /** Destructor.

        The CriticalSection will be unlocked when the destructor is called.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    inline ~GenericScopedUnlock() noexcept                                               { lock_.enter(); }


private:
    //==============================================================================
    const LockType& lock_;

    JUCE_DECLARE_NON_COPYABLE (GenericScopedUnlock)
};


//==============================================================================
/**
    Automatically locks and unlocks a mutex object.

    Use one of these as a local variable to provide RAII-based locking of a mutex.

    The templated class could be a CriticalSection, SpinLock, or anything else that
    provides enter() and exit() methods.

    e.g. @code

    CriticalSection myCriticalSection;

    for (;;)
    {
        const GenericScopedTryLock<CriticalSection> myScopedTryLock (myCriticalSection);

        // Unlike using a ScopedLock, this may fail to actually get the lock, so you
        // should test this with the isLocked() method before doing your thread-unsafe
        // action..
        if (myScopedTryLock.isLocked())
        {
           ...do some stuff...
        }
        else
        {
            ..our attempt at locking failed because another thread had already locked it..
        }

        // myCriticalSection gets unlocked here (if it was locked)
    }
    @endcode

    @see CriticalSection::tryEnter, GenericScopedLock, GenericScopedUnlock
*/
template <class LockType>
class GenericScopedTryLock
{
public:
    //==============================================================================
    /** Creates a GenericScopedTryLock.

        As soon as it is created, this will attempt to acquire the lock, and when the
        GenericScopedTryLock is deleted, the lock will be released (if the lock was
        successfully acquired).

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline explicit GenericScopedTryLock (const LockType& lock) noexcept
        : lock_ (lock), lockWasSuccessful (lock.tryEnter()) {}

    /** Destructor.

        The mutex will be unlocked (if it had been successfully locked) when the
        destructor is called.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    inline ~GenericScopedTryLock() noexcept         { if (lockWasSuccessful) lock_.exit(); }

    /** Returns true if the mutex was successfully locked. */
    bool isLocked() const noexcept                  { return lockWasSuccessful; }

private:
    //==============================================================================
    const LockType& lock_;
    const bool lockWasSuccessful;

    JUCE_DECLARE_NON_COPYABLE (GenericScopedTryLock)
};


#endif   // __JUCE_SCOPEDLOCK_JUCEHEADER__
