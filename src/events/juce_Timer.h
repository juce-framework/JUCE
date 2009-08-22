/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_TIMER_JUCEHEADER__
#define __JUCE_TIMER_JUCEHEADER__

class InternalTimerThread;


//==============================================================================
/**
    Repeatedly calls a user-defined method at a specified time interval.

    A Timer's timerCallback() method will be repeatedly called at a given
    interval. Initially when a Timer object is created, they will do nothing
    until the startTimer() method is called, then the message thread will
    start calling it back until stopTimer() is called.

    The time interval isn't guaranteed to be precise to any more than maybe
    10-20ms, and the intervals may end up being much longer than requested if the
    system is busy. Because it's the message thread that is doing the callbacks,
    any messages that take a significant amount of time to process will block
    all the timers for that period.

    If you need to have a single callback that is shared by multiple timers with
    different frequencies, then the MultiTimer class allows you to do that - its
    structure is very similar to the Timer class, but contains multiple timers
    internally, each one identified by an ID number.

    @see MultiTimer
*/
class JUCE_API  Timer
{
protected:
    //==============================================================================
    /** Creates a Timer.

        When created, the timer is stopped, so use startTimer() to get it going.
    */
    Timer() throw();

    /** Creates a copy of another timer.

        Note that this timer won't be started, even if the one you're copying
        is running.
    */
    Timer (const Timer& other) throw();

public:
    //==============================================================================
    /** Destructor. */
    virtual ~Timer();

    //==============================================================================
    /** The user-defined callback routine that actually gets called periodically.

        It's perfectly ok to call startTimer() or stopTimer() from within this
        callback to change the subsequent intervals.
    */
    virtual void timerCallback() = 0;

    //==============================================================================
    /** Starts the timer and sets the length of interval required.

        If the timer is already started, this will reset it, so the
        time between calling this method and the next timer callback
        will not be less than the interval length passed in.

        @param  intervalInMilliseconds  the interval to use (any values less than 1 will be
                                        rounded up to 1)
    */
    void startTimer (const int intervalInMilliseconds) throw();

    /** Stops the timer.

        No more callbacks will be made after this method returns.

        If this is called from a different thread, any callbacks that may
        be currently executing may be allowed to finish before the method
        returns.
    */
    void stopTimer() throw();

    //==============================================================================
    /** Checks if the timer has been started.

        @returns true if the timer is running.
    */
    bool isTimerRunning() const throw()                     { return periodMs > 0; }

    /** Returns the timer's interval.

        @returns the timer's interval in milliseconds if it's running, or 0 if it's not.
    */
    int getTimerInterval() const throw()                    { return periodMs; }


    //==============================================================================
private:
    friend class InternalTimerThread;
    int countdownMs, periodMs;
    Timer* previous;
    Timer* next;

    const Timer& operator= (const Timer&);
};

#endif   // __JUCE_TIMER_JUCEHEADER__
