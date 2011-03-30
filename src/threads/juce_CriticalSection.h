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

#ifndef __JUCE_CRITICALSECTION_JUCEHEADER__
#define __JUCE_CRITICALSECTION_JUCEHEADER__

#ifndef DOXYGEN
 class ScopedLock;
 class ScopedUnlock;
#endif


//==============================================================================
/**
    A mutex class.

    A CriticalSection acts as a re-entrant mutex lock. The best way to lock and unlock
    one of these is by using RAII in the form of a local ScopedLock object - have a look
    through the codebase for many examples of how to do this.

    @see ScopedLock, SpinLock, Thread, InterProcessLock
*/
class JUCE_API  CriticalSection
{
public:
    //==============================================================================
    /** Creates a CriticalSection object. */
    CriticalSection() throw();

    /** Destructor.
        If the critical section is deleted whilst locked, any subsequent behaviour
        is unpredictable.
    */
    ~CriticalSection() throw();

    //==============================================================================
    /** Acquires the lock.

        If the lock is already held by the caller thread, the method returns immediately.
        If the lock is currently held by another thread, this will wait until it becomes free.
        Remember that it's highly recommended that you never use this method, but use a ScopedLock
        to manage the locking instead.

        @see exit, tryEnter, ScopedLock
    */
    void enter() const throw();

    /** Attempts to lock this critical section without blocking.

        This method behaves identically to CriticalSection::enter, except that the caller thread
        does not wait if the lock is currently held by another thread but returns false immediately.

        @returns false if the lock is currently held by another thread, true otherwise.
        @see enter
    */
    bool tryEnter() const throw();

    /** Releases the lock.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enter() method has been called multiple times by the thread, each
        call must be matched by a call to exit() before other threads will be allowed
        to take over the lock.

        @see enter, ScopedLock
    */
    void exit() const throw();


    //==============================================================================
    /** Provides the type of scoped lock to use with this type of critical section object. */
    typedef ScopedLock      ScopedLockType;

    /** Provides the type of scoped unlocker to use with this type of critical section object. */
    typedef ScopedUnlock    ScopedUnlockType;


private:
    //==============================================================================
   #if JUCE_WINDOWS
    // To avoid including windows.h in the public JUCE headers, we'll just allocate a
    // block of memory here that's big enough to be used internally as a windows critical
    // section structure.
    #if JUCE_64BIT
     uint8 internal [44];
    #else
     uint8 internal [24];
    #endif
   #else
    mutable pthread_mutex_t internal;
   #endif

    JUCE_DECLARE_NON_COPYABLE (CriticalSection);
};


//==============================================================================
/**
    A class that can be used in place of a real CriticalSection object, but which
    doesn't perform any locking.

    This is currently used by some templated classes, and most compilers should
    manage to optimise it out of existence.

    @see CriticalSection, Array, OwnedArray, ReferenceCountedArray
*/
class JUCE_API  DummyCriticalSection
{
public:
    inline DummyCriticalSection() throw()       {}
    inline ~DummyCriticalSection() throw()      {}

    inline void enter() const throw()           {}
    inline bool tryEnter() const throw()        { return true; }
    inline void exit() const throw()            {}

    //==============================================================================
    /** A dummy scoped-lock type to use with a dummy critical section. */
    struct ScopedLockType
    {
        ScopedLockType (const DummyCriticalSection&) throw() {}
    };

    /** A dummy scoped-unlocker type to use with a dummy critical section. */
    typedef ScopedLockType ScopedUnlockType;

private:
    JUCE_DECLARE_NON_COPYABLE (DummyCriticalSection);
};


//==============================================================================
/**
    A simple spin-lock class that can be used as a simple, low-overhead mutex for
    uncontended situations.

    Note that unlike a CriticalSection, this type of lock is not re-entrant, and may
    be less efficient when used it a highly contended situation, but it's very small and
    requires almost no initialisation.
    It's most appropriate for simple situations where you're only going to hold the
    lock for a very brief time.

    @see CriticalSection
*/
class JUCE_API  SpinLock
{
public:
    inline SpinLock() throw() {}
    inline ~SpinLock() throw() {}

    void enter() const throw();
    bool tryEnter() const throw();

    inline void exit() const throw()
    {
        jassert (lock.value == 1); // Agh! Releasing a lock that isn't currently held!
        lock = 0;
    }

    //==============================================================================
    /** A scoped-lock type to use with a SpinLock. */
    class ScopedLockType
    {
    public:
        inline explicit ScopedLockType (const SpinLock& lock_) throw() : lock (lock_)   { lock_.enter(); }
        inline ~ScopedLockType() throw()                                                { lock.exit(); }

    private:
        //==============================================================================
        const SpinLock& lock;
        JUCE_DECLARE_NON_COPYABLE (ScopedLockType);
    };

    //==============================================================================
    /** A scoped-unlocker type to use with a SpinLock. */
    class ScopedUnlockType
    {
    public:
        inline explicit ScopedUnlockType (const SpinLock& lock_) throw() : lock (lock_) { lock_.exit(); }
        inline ~ScopedUnlockType() throw()                                              { lock.enter(); }

    private:
        //==============================================================================
        const SpinLock& lock;
        JUCE_DECLARE_NON_COPYABLE (ScopedUnlockType);
    };

private:
    //==============================================================================
    mutable Atomic<int> lock;
    JUCE_DECLARE_NON_COPYABLE (SpinLock);
};


#endif   // __JUCE_CRITICALSECTION_JUCEHEADER__
