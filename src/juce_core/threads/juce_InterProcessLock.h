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

#ifndef __JUCE_INTERPROCESSLOCK_JUCEHEADER__
#define __JUCE_INTERPROCESSLOCK_JUCEHEADER__

#include "../text/juce_String.h"


//==============================================================================
/**
    Acts as a critical section which processes can use to block each other.

    @see CriticalSection
*/
class JUCE_API  InterProcessLock
{
public:
    //==============================================================================
    /** Creates a lock object.

        @param name     a name that processes will use to identify this lock object
    */
    InterProcessLock (const String& name) throw();

    /** Destructor.

        This will also release the lock if it's currently held by this process.
    */
    ~InterProcessLock() throw();

    //==============================================================================
    /** Attempts to lock the critical section.

        @param timeOutMillisecs     how many milliseconds to wait if the lock
                                    is already held by another process - a value of
                                    0 will return immediately, negative values will wait
                                    forever
        @returns    true if the lock could be gained within the timeout period, or
                    false if the timeout expired.
    */
    bool enter (int timeOutMillisecs = -1) throw();

    /** Releases the lock if it's currently held by this process.
    */
    void exit() throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    void* internal;
    String name;
    int reentrancyLevel;

    InterProcessLock (const InterProcessLock&);
    const InterProcessLock& operator= (const InterProcessLock&);
};


#endif   // __JUCE_INTERPROCESSLOCK_JUCEHEADER__
