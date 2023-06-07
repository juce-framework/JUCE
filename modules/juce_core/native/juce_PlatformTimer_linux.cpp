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
        timer.setIntervalMs (newIntervalMs);
    }

    void cancelTimer()
    {
        startTimer (0);
    }

    int getIntervalMs() const
    {
        return thread.isRunning() ? timer.getIntervalMs() : 0;
    }

private:
    void runThread() override
    {
        if (! (timer.isValid() && exitThread.isValid()))
            return;

        pollfd pollData[]
        {
            { timer.get(),        POLLIN, 0 },
            { exitThread.get(),   POLLIN, 0 }
        };

        const auto& [timerPollData, exitThreadPollData] = pollData;

        for (;;)
        {
            if (poll (pollData, numElementsInArray (pollData), -1) <= 0)
                return;

            if (exitThreadPollData.revents & POLLIN)
                return;

            if (timerPollData.revents & POLLIN)
                listener.onTimerExpired (timer.getAndClearNumberOfExpirations());
        }
    }

    void signalThreadShouldExit() override
    {
        exitThread.signal();
    }

    PlatformTimerListener& listener;
    const TimerFd timer;
    const EventFd exitThread;
    HighResolutionTimerThread thread { *this };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTimer)
    JUCE_DECLARE_NON_MOVEABLE (PlatformTimer)
};

}
