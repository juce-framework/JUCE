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

#ifndef __JUCE_WAITABLEEVENT_JUCEHEADER__
#define __JUCE_WAITABLEEVENT_JUCEHEADER__

#include "../text/juce_String.h"


//==============================================================================
/**
    Allows threads to wait for events triggered by other threads.

    A thread can call wait() on a WaitableObject, and this will suspend the
    calling thread until another thread wakes it up by calling the signal()
    method.
*/
class JUCE_API  WaitableEvent
{
public:
    //==============================================================================
    /** Creates a WaitableEvent object. */
    WaitableEvent() throw();

    /** Destructor.

        If other threads are waiting on this object when it gets deleted, this
        can cause nasty errors, so be careful!
    */
    ~WaitableEvent() throw();

    //==============================================================================
    /** Suspends the calling thread until the event has been signalled.

        This will wait until the object's signal() method is called by another thread,
        or until the timeout expires.

        After the event has been signalled, this method will return true and reset
        the event.

        @param timeOutMilliseconds  the maximum time to wait, in milliseconds. A negative
                                    value will cause it to wait forever.

        @returns    true if the object has been signalled, false if the timeout expires first.
        @see signal, reset
    */
    bool wait (const int timeOutMilliseconds = -1) const throw();

    //==============================================================================
    /** Wakes up any threads that are currently waiting on this object.

        If signal() is called when nothing is waiting, the next thread to call wait()
        will return immediately and reset the signal.

        @see wait, reset
    */
    void signal() const throw();

    //==============================================================================
    /** Resets the event to an unsignalled state.

        If it's not already signalled, this does nothing.
    */
    void reset() const throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    void* internal;

    WaitableEvent (const WaitableEvent&);
    const WaitableEvent& operator= (const WaitableEvent&);
};


#endif   // __JUCE_WAITABLEEVENT_JUCEHEADER__
