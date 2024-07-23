/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/**
    A high-resolution periodic timer.

    This provides accurately-timed regular callbacks. Unlike the normal Timer
    class, this one uses a dedicated thread, not the message thread, so is
    far more stable and precise.

    You should only use this class in situations where you really need accuracy,
    because unlike the normal Timer class, which is very lightweight and cheap
    the HighResolutionTimer will use far more resources and require thread
    safety considerations.

    @see Timer

    @tags{Core}
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

        On some platforms the dedicated timer thread may be shared with
        other HighResolutionTimer's so aim to complete any work in this
        callback as fast as possible.

        It's perfectly ok to call startTimer() or stopTimer() from within this
        callback to change the subsequent intervals. However, if you call
        stopTimer() in the callback it's still best practice to call stopTimer()
        from the destructor in order to avoid data races.
    */
    virtual void hiResTimerCallback() = 0;

    //==============================================================================
    /** Starts the timer and sets the length of interval required.

        If the timer has already started, this will reset the timer, so the
        time between calling this method and the next timer callback
        will not be less than the interval length passed in.

        In exceptional circumstances the dedicated timer thread may not start,
        if this is a potential concern for your use case, you can call isTimerRunning()
        to confirm if the timer actually started.

        @param  intervalInMilliseconds  the interval to use (a value of zero or less will stop the timer)
    */
    void startTimer (int intervalInMilliseconds);

    /** Stops the timer.

        This method may block while it waits for pending callbacks to complete. Once it
        returns, no more callbacks will be made. If it is called from the timer's own thread,
        it will cancel the timer after the current callback returns.

        To prevent data races it's normally best practice to call this in the derived classes
        destructor, even if stopTimer() was called in the hiResTimerCallback().
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
    class Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HighResolutionTimer)
};

} // namespace juce
