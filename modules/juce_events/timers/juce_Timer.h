/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_TIMER_H_INCLUDED
#define JUCE_TIMER_H_INCLUDED


//==============================================================================
/**
    Makes repeated callbacks to a virtual method at a specified time interval.

    A Timer's timerCallback() method will be repeatedly called at a given
    interval. When you create a Timer object, it will do nothing until the
    startTimer() method is called, which will cause the message thread to
    start making callbacks at the specified interval, until stopTimer() is called
    or the object is deleted.

    The time interval isn't guaranteed to be precise to any more than maybe
    10-20ms, and the intervals may end up being much longer than requested if the
    system is busy. Because the callbacks are made by the main message thread,
    anything that blocks the message queue for a period of time will also prevent
    any timers from running until it can carry on.

    If you need to have a single callback that is shared by multiple timers with
    different frequencies, then the MultiTimer class allows you to do that - its
    structure is very similar to the Timer class, but contains multiple timers
    internally, each one identified by an ID number.

    @see HighResolutionTimer, MultiTimer
*/
class JUCE_API  Timer
{
protected:
    //==============================================================================
    /** Creates a Timer.

        When created, the timer is stopped, so use startTimer() to get it going.
    */
    Timer() noexcept;

    /** Creates a copy of another timer.

        Note that this timer won't be started, even if the one you're copying
        is running.
    */
    Timer (const Timer& other) noexcept;

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
    void startTimer (int intervalInMilliseconds) noexcept;

    /** Starts the timer with an interval specified in Hertz.
        This is effectively the same as calling startTimer (1000 / timerFrequencyHz).
    */
    void startTimerHz (int timerFrequencyHz) noexcept;

    /** Stops the timer.

        No more callbacks will be made after this method returns.

        If this is called from a different thread, any callbacks that may
        be currently executing may be allowed to finish before the method
        returns.
    */
    void stopTimer() noexcept;

    //==============================================================================
    /** Returns true if the timer is currently running. */
    bool isTimerRunning() const noexcept                    { return periodMs > 0; }

    /** Returns the timer's interval.
        @returns the timer's interval in milliseconds if it's running, or 0 if it's not.
    */
    int getTimerInterval() const noexcept                   { return periodMs; }


    //==============================================================================
    /** For internal use only: invokes any timers that need callbacks.
        Don't call this unless you really know what you're doing!
    */
    static void JUCE_CALLTYPE callPendingTimersSynchronously();

private:
    class TimerThread;
    friend class TimerThread;
    int countdownMs, periodMs;
    Timer* previous;
    Timer* next;

    Timer& operator= (const Timer&);
};

#endif   // JUCE_TIMER_H_INCLUDED
