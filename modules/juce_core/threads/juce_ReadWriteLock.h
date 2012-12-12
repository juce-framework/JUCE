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

#ifndef __JUCE_READWRITELOCK_JUCEHEADER__
#define __JUCE_READWRITELOCK_JUCEHEADER__

#include "juce_CriticalSection.h"
#include "juce_SpinLock.h"
#include "juce_WaitableEvent.h"
#include "juce_Thread.h"
#include "../containers/juce_Array.h"


//==============================================================================
/**
    A critical section that allows multiple simultaneous readers.

    Features of this type of lock are:

    - Multiple readers can hold the lock at the same time, but only one writer
      can hold it at once.
    - Writers trying to gain the lock will be blocked until all readers and writers
      have released it
    - Readers trying to gain the lock while a writer is waiting to acquire it will be
      blocked until the writer has obtained and released it
    - If a thread already has a read lock and tries to obtain a write lock, it will succeed if
      there are no other readers
    - If a thread already has the write lock and tries to obtain a read lock, this will succeed.
    - Recursive locking is supported.

    @see ScopedReadLock, ScopedWriteLock, CriticalSection
*/
class JUCE_API  ReadWriteLock
{
public:
    //==============================================================================
    /**
        Creates a ReadWriteLock object.
    */
    ReadWriteLock() noexcept;

    /** Destructor.

        If the object is deleted whilst locked, any subsequent behaviour
        is unpredictable.
    */
    ~ReadWriteLock() noexcept;

    //==============================================================================
    /** Locks this object for reading.

        Multiple threads can simulaneously lock the object for reading, but if another
        thread has it locked for writing, then this will block until it releases the
        lock.

        @see exitRead, ScopedReadLock
    */
    void enterRead() const noexcept;

    /** Tries to lock this object for reading.

        Multiple threads can simulaneously lock the object for reading, but if another
        thread has it locked for writing, then this will fail and return false.

        @returns true if the lock is successfully gained.
        @see exitRead, ScopedReadLock
    */
    bool tryEnterRead() const noexcept;

    /** Releases the read-lock.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enterRead() method has been called multiple times by the thread, each
        call must be matched by a call to exitRead() before other threads will be allowed
        to take over the lock.

        @see enterRead, ScopedReadLock
    */
    void exitRead() const noexcept;

    //==============================================================================
    /** Locks this object for writing.

        This will block until any other threads that have it locked for reading or
        writing have released their lock.

        @see exitWrite, ScopedWriteLock
    */
    void enterWrite() const noexcept;

    /** Tries to lock this object for writing.

        This is like enterWrite(), but doesn't block - it returns true if it manages
        to obtain the lock.

        @returns true if the lock is successfully gained.
        @see enterWrite
    */
    bool tryEnterWrite() const noexcept;

    /** Releases the write-lock.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enterWrite() method has been called multiple times by the thread, each
        call must be matched by a call to exit() before other threads will be allowed
        to take over the lock.

        @see enterWrite, ScopedWriteLock
    */
    void exitWrite() const noexcept;


private:
    //==============================================================================
    SpinLock accessLock;
    WaitableEvent waitEvent;
    mutable int numWaitingWriters, numWriters;
    mutable Thread::ThreadID writerThreadId;

    struct ThreadRecursionCount
    {
        Thread::ThreadID threadID;
        int count;
    };

    mutable Array <ThreadRecursionCount> readerThreads;

    JUCE_DECLARE_NON_COPYABLE (ReadWriteLock)
};


#endif   // __JUCE_READWRITELOCK_JUCEHEADER__
