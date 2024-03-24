/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Automatically locks and unlocks a ReadWriteLock object.

    Use one of these as a local variable to control access to a ReadWriteLock.

    e.g. @code

    ReadWriteLock myLock;

    for (;;)
    {
        const ScopedReadLock myScopedLock (myLock);
        // myLock is now locked

        ...do some stuff...

        // myLock gets unlocked here.
    }
    @endcode

    @see ReadWriteLock, ScopedWriteLock

    @tags{Core}
*/
class JUCE_API  ScopedReadLock
{
public:
    //==============================================================================
    /** Creates a ScopedReadLock.

        As soon as it is created, this will call ReadWriteLock::enterRead(), and
        when the ScopedReadLock object is deleted, the ReadWriteLock will
        be unlocked.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline explicit ScopedReadLock (const ReadWriteLock& lock) noexcept   : lock_ (lock) { lock.enterRead(); }

    /** Destructor.

        The ReadWriteLock's exitRead() method will be called when the destructor is called.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    inline ~ScopedReadLock() noexcept                                     { lock_.exitRead(); }


private:
    //==============================================================================
    const ReadWriteLock& lock_;

    JUCE_DECLARE_NON_COPYABLE (ScopedReadLock)
};

//==============================================================================
/**
    Automatically locks and unlocks a ReadWriteLock object.

    Use one of these as a local variable to control access to a ReadWriteLock.

    e.g. @code

    ReadWriteLock myLock;

    for (;;)
    {
        const ScopedTryReadLock myScopedTryLock (myLock);

        // Unlike using a ScopedReadLock, this may fail to actually get the lock, so you
        // should test this with the isLocked() method before doing your thread-unsafe
        // action.

        if (myScopedTryLock.isLocked())
        {
            ...do some stuff...
        }
        else
        {
            ..our attempt at locking failed because a write lock has already been issued..
        }

        // myLock gets unlocked here (if it was locked).
    }
    @endcode

    @see ReadWriteLock, ScopedTryWriteLock

    @tags{Core}
*/
class JUCE_API  ScopedTryReadLock
{
public:
    //==============================================================================
    /** Creates a ScopedTryReadLock and calls ReadWriteLock::tryEnterRead() as soon as it is
        created. When the ScopedTryReadLock object is destructed, the ReadWriteLock will be unlocked
        (if it was successfully acquired).

        Make sure this object is created and destructed by the same thread, otherwise there are no
        guarantees what will happen! Best just to use it as a local stack object, rather than creating
        one with the new() operator.
    */
    explicit ScopedTryReadLock (ReadWriteLock& lockIn)
        : ScopedTryReadLock (lockIn, true) {}

    /** Creates a ScopedTryReadLock.

        If acquireLockOnInitialisation is true then as soon as it is created, this will call
        ReadWriteLock::tryEnterRead(), and when the ScopedTryReadLock object is destructed, the
        ReadWriteLock will be unlocked (if it was successfully acquired).

        Make sure this object is created and destructed by the same thread, otherwise there are no
        guarantees what will happen! Best just to use it as a local stack object, rather than creating
        one with the new() operator.
    */
    ScopedTryReadLock (ReadWriteLock& lockIn, bool acquireLockOnInitialisation) noexcept
        : lock (lockIn), lockWasSuccessful (acquireLockOnInitialisation && lock.tryEnterRead()) {}

    /** Destructor.

        The ReadWriteLock's exitRead() method will be called when the destructor is called.

        Make sure this object is created and destructed by the same thread, otherwise there are no
        guarantees what will happen!
    */
    ~ScopedTryReadLock() noexcept                   { if (lockWasSuccessful) lock.exitRead(); }

    /** Returns true if the mutex was successfully locked. */
    bool isLocked() const noexcept                  { return lockWasSuccessful; }

    /** Retry gaining the lock by calling tryEnter on the underlying lock. */
    bool retryLock() noexcept                       { return lockWasSuccessful = lock.tryEnterRead(); }

private:
    //==============================================================================
    ReadWriteLock& lock;
    bool lockWasSuccessful;

    JUCE_DECLARE_NON_COPYABLE (ScopedTryReadLock)
};

} // namespace juce
