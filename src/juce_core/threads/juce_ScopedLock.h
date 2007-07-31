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

#ifndef __JUCE_SCOPEDLOCK_JUCEHEADER__
#define __JUCE_SCOPEDLOCK_JUCEHEADER__

#include "juce_CriticalSection.h"


//==============================================================================
/**
    Automatically locks and unlocks a CriticalSection object.

    Use one of these as a local variable to control access to a CriticalSection.

    e.g. @code

    CriticalSection myCriticalSection;

    for (;;)
    {
        const ScopedLock myScopedLock (myCriticalSection);
        // myCriticalSection is now locked

        ...do some stuff...

        // myCriticalSection gets unlocked here.
    }
    @endcode

    @see CriticalSection, ScopedUnlock
*/
class JUCE_API  ScopedLock
{
public:
    //==============================================================================
    /** Creates a ScopedLock.

        As soon as it is created, this will lock the CriticalSection, and
        when the ScopedLock object is deleted, the CriticalSection will
        be unlocked.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline ScopedLock (const CriticalSection& lock) throw()     : lock_ (lock) { lock.enter(); }

    /** Destructor.

        The CriticalSection will be unlocked when the destructor is called.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    inline ~ScopedLock() throw()                                { lock_.exit(); }


private:
    //==============================================================================
    const CriticalSection& lock_;

    ScopedLock (const ScopedLock&);
    const ScopedLock& operator= (const ScopedLock&);
};


//==============================================================================
/**
    Automatically unlocks and re-locks a CriticalSection object.

    This is the reverse of a ScopedLock object - instead of locking the critical
    section for the lifetime of this object, it unlocks it.

    Make sure you don't try to unlock critical sections that aren't actually locked!

    e.g. @code

    CriticalSection myCriticalSection;

    for (;;)
    {
        const ScopedLock myScopedLock (myCriticalSection);
        // myCriticalSection is now locked

        ... do some stuff with it locked ..

        while (xyz)
        {
            ... do some stuff with it locked ..

            const ScopedUnlock unlocker (myCriticalSection);

            // myCriticalSection is now unlocked for the remainder of this block,
            // and re-locked at the end.

            ...do some stuff with it unlocked ...
        }

        // myCriticalSection gets unlocked here.
    }
    @endcode

    @see CriticalSection, ScopedLock
*/
class ScopedUnlock
{
public:
    //==============================================================================
    /** Creates a ScopedUnlock.

        As soon as it is created, this will unlock the CriticalSection, and
        when the ScopedLock object is deleted, the CriticalSection will
        be re-locked.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline ScopedUnlock (const CriticalSection& lock) throw()     : lock_ (lock) { lock.exit(); }

    /** Destructor.

        The CriticalSection will be unlocked when the destructor is called.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    inline ~ScopedUnlock() throw()                                { lock_.enter(); }


private:
    //==============================================================================
    const CriticalSection& lock_;

    ScopedUnlock (const ScopedLock&);
    const ScopedUnlock& operator= (const ScopedUnlock&);
};



#endif   // __JUCE_SCOPEDLOCK_JUCEHEADER__
