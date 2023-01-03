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

class HighResolutionTimer::Pimpl : public Thread
{
    using steady_clock = std::chrono::steady_clock;
    using milliseconds = std::chrono::milliseconds;

public:
    explicit Pimpl (HighResolutionTimer& ownerRef)
        : Thread ("HighResolutionTimerThread"),
          owner (ownerRef)
    {
    }

    using Thread::isThreadRunning;

    void start (int periodMs)
    {
        {
            const std::scoped_lock lk { mutex };
            periodMillis = periodMs;
            nextTickTime = steady_clock::now() + milliseconds (periodMillis);
        }

        waitEvent.notify_one();

        if (! isThreadRunning())
            startThread (Thread::Priority::high);
    }

    void stop()
    {
        {
            const std::scoped_lock lk { mutex };
            periodMillis = 0;
        }

        waitEvent.notify_one();

        if (Thread::getCurrentThreadId() != getThreadId())
            stopThread (-1);
    }

    int getPeriod() const
    {
        return periodMillis;
    }

private:
    void run() override
    {
        for (;;)
        {
            {
                std::unique_lock lk { mutex };

                if (waitEvent.wait_until (lk, nextTickTime, [this] { return periodMillis == 0; }))
                    break;

                nextTickTime = steady_clock::now() + milliseconds (periodMillis);
            }

            owner.hiResTimerCallback();
        }
    }

    HighResolutionTimer& owner;
    std::atomic<int> periodMillis { 0 };
    steady_clock::time_point nextTickTime;
    std::mutex mutex;
    std::condition_variable waitEvent;
};

HighResolutionTimer::HighResolutionTimer()
    : pimpl (new Pimpl (*this))
{
}

HighResolutionTimer::~HighResolutionTimer()
{
    stopTimer();
}

void HighResolutionTimer::startTimer (int periodMs)
{
    pimpl->start (jmax (1, periodMs));
}

void HighResolutionTimer::stopTimer()
{
    pimpl->stop();
}

bool HighResolutionTimer::isTimerRunning() const noexcept     { return getTimerInterval() != 0; }
int HighResolutionTimer::getTimerInterval() const noexcept    { return pimpl->getPeriod(); }

} // namespace juce
