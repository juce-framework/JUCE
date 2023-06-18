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

class PlatformTimer final : private HighResolutionTimerThread::Impl
{
public:
    explicit PlatformTimer (PlatformTimerListener& ptl)
        : listener { ptl } {}

    void startTimer (int newIntervalMs)
    {
        if (! thread.isRunning())
            return;

        {
            std::scoped_lock lock { mutex };
            intervalMs = newIntervalMs;
            nextEventTime = Time::getCurrentTime() + RelativeTime::milliseconds (newIntervalMs);
        }

        event.signal();
    }

    void cancelTimer()
    {
        jassert (thread.isRunning());

        {
            std::scoped_lock lock { mutex };
            jassert (intervalMs > 0);
            intervalMs = 0;
        }

        event.signal();
    }

    int getIntervalMs() const
    {
        std::scoped_lock lock { mutex };
        return thread.isRunning() ? intervalMs : 0;
    }

private:
    int millisecondsUntilNextEvent()
    {
        std::scoped_lock lock { mutex };
        return intervalMs > 0 ? jmax (0, (int) (nextEventTime - Time::getCurrentTime()).inMilliseconds()) : -1;
    }

    bool nextEvent()
    {
        std::scoped_lock lock { mutex };
        if (intervalMs <= 0 || nextEventTime > Time::getCurrentTime())
            return false;

        nextEventTime += RelativeTime::milliseconds (intervalMs);
        return true;
    }

    void runThread() override
    {
        while (! shouldExitThread.load())
        {
            if (nextEvent())
                listener.onTimerExpired (1);
            else
                event.wait (millisecondsUntilNextEvent());
        }
    }

    void signalThreadShouldExit() override
    {
        shouldExitThread.store (true);
        event.signal();
    }

    PlatformTimerListener& listener;
    mutable std::mutex mutex;
    int intervalMs{};
    Time nextEventTime;
    WaitableEvent event;
    std::atomic<bool> shouldExitThread { false };
    HighResolutionTimerThread thread { *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTimer)
    JUCE_DECLARE_NON_MOVEABLE (PlatformTimer)
};

} // namespace juce
