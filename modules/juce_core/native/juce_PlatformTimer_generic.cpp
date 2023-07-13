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

class PlatformTimer final : private Thread
{
public:
    explicit PlatformTimer (PlatformTimerListener& ptl)
        : Thread { "HighResolutionTimerThread" },
          listener { ptl }
    {
        startThread (Priority::highest);
    }

    ~PlatformTimer()
    {
        stopThread (-1);
    }

    void startTimer (int newIntervalMs)
    {
        jassert (newIntervalMs > 0);
        jassert (timer == nullptr);

        {
            std::scoped_lock lock { runCopyMutex };
            timer = std::make_shared<Timer> (listener, newIntervalMs);
        }

        notify();
    }

    void cancelTimer()
    {
        jassert (timer != nullptr);

        timer->cancel();

        // Note the only race condition we need to protect against
        // here is the copy in run().
        //
        // Calls to startTimer(), cancelTimer(), and getIntervalMs()
        // are already guaranteed to be both thread safe and well
        // synchronised.

        std::scoped_lock lock { runCopyMutex };
        timer = nullptr;
    }

    int getIntervalMs() const
    {
        return isThreadRunning() && timer != nullptr ? timer->getIntervalMs() : 0;
    }

private:
    void run() final
    {
        const auto copyTimer = [&]
        {
            std::scoped_lock lock { runCopyMutex };
            return timer;
        };

        while (! threadShouldExit())
        {
            if (auto t = copyTimer())
                t->run();

            wait (-1);
        }
    }

    class Timer
    {
    public:
        Timer (PlatformTimerListener& l, int i)
            : listener { l }, intervalMs { i } {}

        int getIntervalMs() const
        {
            return intervalMs;
        }

        void cancel()
        {
            stop.signal();
        }

        void run()
        {
           #if JUCE_MAC || JUCE_IOS
            tryToUpgradeCurrentThreadToRealtime (Thread::RealtimeOptions{}.withPeriodMs (intervalMs));
           #endif

            const auto millisecondsUntil = [] (auto time)
            {
                return jmax (0.0, time - Time::getMillisecondCounterHiRes());
            };

            while (! stop.wait (millisecondsUntil (nextEventTime)))
            {
                if (Time::getMillisecondCounterHiRes() >= nextEventTime)
                {
                    listener.onTimerExpired();
                    nextEventTime += intervalMs;
                }
            }
        }

    private:
        PlatformTimerListener& listener;
        const int intervalMs;
        double nextEventTime = Time::getMillisecondCounterHiRes() + intervalMs;
        WaitableEvent stop { true };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Timer)
        JUCE_DECLARE_NON_MOVEABLE (Timer)
    };

    PlatformTimerListener& listener;
    mutable std::mutex runCopyMutex;
    std::shared_ptr<Timer> timer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTimer)
    JUCE_DECLARE_NON_MOVEABLE (PlatformTimer)
};

} // namespace juce
