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

class ShutdownDetector : private DeletedAtShutdown
{
public:
    ShutdownDetector() = default;

    ~ShutdownDetector() override
    {
        getListeners().call (&Listener::applicationShuttingDown);
        clearSingletonInstance();
    }

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void applicationShuttingDown() = 0;
    };

    static void addListener (Listener* listenerToAdd)
    {
        // Only try to create an instance of the ShutdownDetector when a listener is added
        [[maybe_unused]] auto* instance = getInstance();
        getListeners().add (listenerToAdd);
    }

    static void removeListener (Listener* listenerToRemove)
    {
        getListeners().remove (listenerToRemove);
    }

private:
    using ListenerListType = ThreadSafeListenerList<Listener>;

    // By having a static ListenerList it can outlive the ShutdownDetector instance preventing
    // issues for objects trying to remove themselves after the instance has been deleted
    static ListenerListType& getListeners()
    {
        static ListenerListType listeners;
        return listeners;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShutdownDetector)
    JUCE_DECLARE_NON_MOVEABLE (ShutdownDetector)
    JUCE_DECLARE_SINGLETON_INLINE (ShutdownDetector, false)
};

class Timer::TimerThread final : private Thread,
                                 private ShutdownDetector::Listener
{
public:
    using LockType = CriticalSection;

    TimerThread()
        : Thread (SystemStats::getJUCEVersion() + ": Timer")
    {
        timers.reserve (32);
        ShutdownDetector::addListener (this);
    }

    ~TimerThread() override
    {
        // If this is hit, a timer has outlived the platform event system.
        jassert (MessageManager::getInstanceWithoutCreating() != nullptr);

        stopThreadAsync();
        ShutdownDetector::removeListener (this);
        stopThread (-1);
    }

    void run() override
    {
        auto lastTime = Time::getMillisecondCounter();
        ReferenceCountedObjectPtr<CallTimersMessage> messageToSend (new CallTimersMessage());

        while (! threadShouldExit())
        {
            auto now = Time::getMillisecondCounter();
            auto elapsed = (int) (now >= lastTime ? (now - lastTime)
                                                  : (std::numeric_limits<uint32>::max() - (lastTime - now)));
            lastTime = now;

            auto timeUntilFirstTimer = getTimeUntilFirstTimer (elapsed);

            if (timeUntilFirstTimer <= 0)
            {
                if (callbackArrived.wait (0))
                {
                    // already a message in flight - do nothing..
                }
                else
                {
                    messageToSend->post();

                    if (! callbackArrived.wait (300))
                    {
                        // Sometimes our message can get discarded by the OS (e.g. when running as an RTAS
                        // when the app has a modal loop), so this is how long to wait before assuming the
                        // message has been lost and trying again.
                        messageToSend->post();
                    }

                    continue;
                }
            }

            // don't wait for too long because running this loop also helps keep the
            // Time::getApproximateMillisecondTimer value stay up-to-date
            wait (jlimit (1, 100, timeUntilFirstTimer));
        }
    }

    void callTimers()
    {
        auto timeout = Time::getMillisecondCounter() + 100;

        const LockType::ScopedLockType sl (lock);

        while (! timers.empty())
        {
            auto& first = timers.front();

            if (first.countdownMs > 0)
                break;

            auto* timer = first.timer;
            first.countdownMs = timer->timerPeriodMs;
            shuffleTimerBackInQueue (0);
            notify();

            const LockType::ScopedUnlockType ul (lock);

            JUCE_TRY
            {
                timer->timerCallback();
            }
            JUCE_CATCH_EXCEPTION

            // avoid getting stuck in a loop if a timer callback repeatedly takes too long
            if (Time::getMillisecondCounter() > timeout)
                break;
        }

        callbackArrived.signal();
    }

    void callTimersSynchronously()
    {
        callTimers();
    }

    void addTimer (Timer* t)
    {
        const LockType::ScopedLockType sl (lock);

        if (! isThreadRunning())
            startThread (Thread::Priority::high);

        // Trying to add a timer that's already here - shouldn't get to this point,
        // so if you get this assertion, let me know!
        jassert (std::none_of (timers.begin(), timers.end(),
                               [t] (TimerCountdown i) { return i.timer == t; }));

        auto pos = timers.size();

        timers.push_back ({ t, t->timerPeriodMs });
        t->positionInQueue = pos;
        shuffleTimerForwardInQueue (pos);
        notify();
    }

    void removeTimer (Timer* t)
    {
        const LockType::ScopedLockType sl (lock);

        auto pos = t->positionInQueue;
        auto lastIndex = timers.size() - 1;

        jassert (pos <= lastIndex);
        jassert (timers[pos].timer == t);

        for (auto i = pos; i < lastIndex; ++i)
        {
            timers[i] = timers[i + 1];
            timers[i].timer->positionInQueue = i;
        }

        timers.pop_back();
    }

    void resetTimerCounter (Timer* t) noexcept
    {
        const LockType::ScopedLockType sl (lock);

        auto pos = t->positionInQueue;

        jassert (pos < timers.size());
        jassert (timers[pos].timer == t);

        auto lastCountdown = timers[pos].countdownMs;
        auto newCountdown = t->timerPeriodMs;

        if (newCountdown != lastCountdown)
        {
            timers[pos].countdownMs = newCountdown;

            if (newCountdown > lastCountdown)
                shuffleTimerBackInQueue (pos);
            else
                shuffleTimerForwardInQueue (pos);

            notify();
        }
    }

private:
    LockType lock;

    struct TimerCountdown
    {
        Timer* timer;
        int countdownMs;
    };

    std::vector<TimerCountdown> timers;

    WaitableEvent callbackArrived;

    struct CallTimersMessage final : public MessageManager::MessageBase
    {
        CallTimersMessage() = default;

        void messageCallback() override
        {
            if (auto instance = SharedResourcePointer<TimerThread>::getSharedObjectWithoutCreating())
                (*instance)->callTimers();
        }
    };

    //==============================================================================
    void shuffleTimerBackInQueue (size_t pos)
    {
        auto numTimers = timers.size();

        if (pos < numTimers - 1)
        {
            auto t = timers[pos];

            for (;;)
            {
                auto next = pos + 1;

                if (next == numTimers || timers[next].countdownMs >= t.countdownMs)
                    break;

                timers[pos] = timers[next];
                timers[pos].timer->positionInQueue = pos;

                ++pos;
            }

            timers[pos] = t;
            t.timer->positionInQueue = pos;
        }
    }

    void shuffleTimerForwardInQueue (size_t pos)
    {
        if (pos > 0)
        {
            auto t = timers[pos];

            while (pos > 0)
            {
                auto& prev = timers[(size_t) pos - 1];

                if (prev.countdownMs <= t.countdownMs)
                    break;

                timers[pos] = prev;
                timers[pos].timer->positionInQueue = pos;

                --pos;
            }

            timers[pos] = t;
            t.timer->positionInQueue = pos;
        }
    }

    int getTimeUntilFirstTimer (int numMillisecsElapsed)
    {
        const LockType::ScopedLockType sl (lock);

        if (timers.empty())
            return 1000;

        for (auto& t : timers)
            t.countdownMs -= numMillisecsElapsed;

        return timers.front().countdownMs;
    }

    //==============================================================================
    void applicationShuttingDown() final
    {
        stopThreadAsync();
    }

    void stopThreadAsync()
    {
        signalThreadShouldExit();
        callbackArrived.signal();
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimerThread)
};

//==============================================================================
Timer::Timer() noexcept {}
Timer::Timer (const Timer&) noexcept {}

Timer::~Timer()
{
    // If you're destroying a timer on a background thread, make sure the timer has
    // been stopped before execution reaches this point. A simple way to achieve this
    // is to add a call to `stopTimer()` to the destructor of your class which inherits
    // from Timer.
    jassert (! isTimerRunning()
             || MessageManager::getInstanceWithoutCreating() == nullptr
             || MessageManager::getInstanceWithoutCreating()->currentThreadHasLockedMessageManager());

    stopTimer();
}

void Timer::startTimer (int interval) noexcept
{
    // If you're calling this before (or after) the MessageManager is
    // running, then you're not going to get any timer callbacks!
    JUCE_ASSERT_MESSAGE_MANAGER_EXISTS

    bool wasStopped = (timerPeriodMs == 0);
    timerPeriodMs = jmax (1, interval);

    if (wasStopped)
        timerThread->addTimer (this);
    else
        timerThread->resetTimerCounter (this);
}

void Timer::startTimerHz (int timerFrequencyHz) noexcept
{
    if (timerFrequencyHz > 0)
        startTimer (1000 / timerFrequencyHz);
    else
        stopTimer();
}

void Timer::stopTimer() noexcept
{
    if (timerPeriodMs > 0)
    {
        timerThread->removeTimer (this);
        timerPeriodMs = 0;
    }
}

void JUCE_CALLTYPE Timer::callPendingTimersSynchronously()
{
    if (auto instance = SharedResourcePointer<TimerThread>::getSharedObjectWithoutCreating())
        (*instance)->callTimersSynchronously();
}

struct LambdaInvoker final : private Timer,
                             private DeletedAtShutdown
{
    LambdaInvoker (int milliseconds, std::function<void()> f)
        : function (std::move (f))
    {
        startTimer (milliseconds);
    }

    ~LambdaInvoker() final
    {
        stopTimer();
    }

    void timerCallback() final
    {
        NullCheckedInvocation::invoke (function);
        delete this;
    }

    std::function<void()> function;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LambdaInvoker)
};

void JUCE_CALLTYPE Timer::callAfterDelay (int milliseconds, std::function<void()> f)
{
    new LambdaInvoker (milliseconds, std::move (f));
}

} // namespace juce
