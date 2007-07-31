/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_READWRITELOCK_JUCEHEADER__
#define __JUCE_READWRITELOCK_JUCEHEADER__

#include "juce_CriticalSection.h"
#include "juce_WaitableEvent.h"
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
    ReadWriteLock() throw();

    /** Destructor.

        If the object is deleted whilst locked, any subsequent behaviour
        is unpredictable.
    */
    ~ReadWriteLock() throw();

    //==============================================================================
    /** Locks this object for reading.

        Multiple threads can simulaneously lock the object for reading, but if another
        thread has it locked for writing, then this will block until it releases the
        lock.

        @see exitRead, ScopedReadLock
    */
    void enterRead() const throw();

    /** Releases the read-lock.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enterRead() method has been called multiple times by the thread, each
        call must be matched by a call to exitRead() before other threads will be allowed
        to take over the lock.

        @see enterRead, ScopedReadLock
    */
    void exitRead() const throw();

    //==============================================================================
    /** Locks this object for writing.

        This will block until any other threads that have it locked for reading or
        writing have released their lock.

        @see exitWrite, ScopedWriteLock
    */
    void enterWrite() const throw();

    /** Releases the write-lock.

        If the caller thread hasn't got the lock, this can have unpredictable results.

        If the enterWrite() method has been called multiple times by the thread, each
        call must be matched by a call to exit() before other threads will be allowed
        to take over the lock.

        @see enterWrite, ScopedWriteLock
    */
    void exitWrite() const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    CriticalSection accessLock;
    WaitableEvent waitEvent;
    mutable int numWaitingWriters, numWriters;
    mutable int writerThreadId;
    mutable Array <int> readerThreads;

    ReadWriteLock (const ReadWriteLock&);
    const ReadWriteLock& operator= (const ReadWriteLock&);
};


#endif   // __JUCE_READWRITELOCK_JUCEHEADER__
