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

#ifndef __JUCE_SCOPEDREADLOCK_JUCEHEADER__
#define __JUCE_SCOPEDREADLOCK_JUCEHEADER__

#include "juce_ReadWriteLock.h"


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


#endif   // __JUCE_SCOPEDREADLOCK_JUCEHEADER__
