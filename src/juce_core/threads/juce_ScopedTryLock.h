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

#ifndef __JUCE_SCOPEDTRYLOCK_JUCEHEADER__
#define __JUCE_SCOPEDTRYLOCK_JUCEHEADER__

#include "juce_CriticalSection.h"


//==============================================================================
/**
    Automatically tries to lock and unlock a CriticalSection object.

    Use one of these as a local variable to control access to a CriticalSection.

    e.g. @code

    CriticalSection myCriticalSection;

    for (;;)
    {
        const ScopedTryLock myScopedTryLock (myCriticalSection);

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

    @see CriticalSection::tryEnter, ScopedLock, ScopedUnlock, ScopedReadLock
*/
class JUCE_API  ScopedTryLock
{
public:
    //==============================================================================
    /** Creates a ScopedTryLock.

        As soon as it is created, this will try to lock the CriticalSection, and
        when the ScopedTryLock object is deleted, the CriticalSection will
        be unlocked if the lock was successful.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline ScopedTryLock (const CriticalSection& lock) throw()      : lock_ (lock), lockWasSuccessful (lock.tryEnter()) {}

    /** Destructor.

        The CriticalSection will be unlocked (if locked) when the destructor is called.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    inline ~ScopedTryLock() throw()                                 { if (lockWasSuccessful) lock_.exit(); }

    /** Lock state

    @return True if the CriticalSection is locked.
    */
    bool isLocked() const throw()                                   { return lockWasSuccessful; }

private:
    //==============================================================================
    const CriticalSection& lock_;
    const bool lockWasSuccessful;

    ScopedTryLock (const ScopedTryLock&);
    const ScopedTryLock& operator= (const ScopedTryLock&);
};


#endif   // __JUCE_SCOPEDTRYLOCK_JUCEHEADER__
