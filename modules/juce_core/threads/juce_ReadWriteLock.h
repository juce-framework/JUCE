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

#ifndef JUCE_READWRITELOCK_H_INCLUDED
#define JUCE_READWRITELOCK_H_INCLUDED


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
        If the object is deleted whilst locked, any subsequent behaviour is undefined.
    */
    ~ReadWriteLock() noexcept;

    //==============================================================================
    /** Locks this object for reading.

        Multiple threads can simultaneously lock the object for reading, but if another
        thread has it locked for writing, then this will block until it releases the lock.

        @see exitRead, ScopedReadLock
    */
    void enterRead() const noexcept;

    /** Tries to lock this object for reading.

        Multiple threads can simultaneously lock the object for reading, but if another
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


#endif   // JUCE_READWRITELOCK_H_INCLUDED
