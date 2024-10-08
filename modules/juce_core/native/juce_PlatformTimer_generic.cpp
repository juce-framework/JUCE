/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class GenericPlatformTimer final : private Thread
{
public:
    explicit GenericPlatformTimer (PlatformTimerListener& ptl)
        : Thread { "HighResolutionTimerThread" },
          listener { ptl }
    {
        if (startThread (Priority::highest))
            return;

        // This likely suggests there are too many threads running!
        jassertfalse;
    }

    ~GenericPlatformTimer() override
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericPlatformTimer)
    JUCE_DECLARE_NON_MOVEABLE (GenericPlatformTimer)
};

#if ! JUCE_WINDOWS
using PlatformTimer = GenericPlatformTimer;
#endif

} // namespace juce
