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

#ifndef JUCE_HIGHRESOLUTIONTIMER_H_INCLUDED
#define JUCE_HIGHRESOLUTIONTIMER_H_INCLUDED

/**
    A high-resolution periodic timer.

    This provides accurately-timed regular callbacks. Unlike the normal Timer
    class, this one uses a dedicated thread, not the message thread, so is
    far more stable and precise.

    You should only use this class in situations where you really need accuracy,
    because unlike the normal Timer class, which is very lightweight and cheap
    to start/stop, the HighResolutionTimer will use far more resources, and
    starting/stopping it may involve launching and killing threads.

    @see Timer
*/
class JUCE_API  HighResolutionTimer
{
protected:
    /** Creates a HighResolutionTimer.
        When created, the timer is stopped, so use startTimer() to get it going.
    */
    HighResolutionTimer();

public:
    /** Destructor. */
    virtual ~HighResolutionTimer();

    //==============================================================================
    /** The user-defined callback routine that actually gets called periodically.

        This will be called on a dedicated timer thread, so make sure your
        implementation is thread-safe!

        It's perfectly ok to call startTimer() or stopTimer() from within this
        callback to change the subsequent intervals.
    */
    virtual void hiResTimerCallback() = 0;

    //==============================================================================
    /** Starts the timer and sets the length of interval required.

        If the timer is already started, this will reset its counter, so the
        time between calling this method and the next timer callback will not be
        less than the interval length passed in.

        @param  intervalInMilliseconds  the interval to use (any values less than 1 will be
                                        rounded up to 1)
    */
    void startTimer (int intervalInMilliseconds);

    /** Stops the timer.

        This method may block while it waits for pending callbacks to complete. Once it
        returns, no more callbacks will be made. If it is called from the timer's own thread,
        it will cancel the timer after the current callback returns.
    */
    void stopTimer();

    /** Checks if the timer has been started.
        @returns true if the timer is running.
    */
    bool isTimerRunning() const noexcept;

    /** Returns the timer's interval.
        @returns the timer's interval in milliseconds if it's running, or 0 if it's not.
    */
    int getTimerInterval() const noexcept;

private:
    struct Pimpl;
    friend struct Pimpl;
    friend struct ContainerDeletePolicy<Pimpl>;
    ScopedPointer<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HighResolutionTimer)
};


#endif   // JUCE_HIGHRESOLUTIONTIMER_H_INCLUDED
