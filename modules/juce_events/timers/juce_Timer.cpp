/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

class Timer::TimerThread  : private Thread,
                            private DeletedAtShutdown,
                            private AsyncUpdater
{
public:
    typedef CriticalSection LockType; // (mysteriously, using a SpinLock here causes problems on some XP machines..)

    TimerThread()
        : Thread ("Juce Timer"),
          firstTimer (nullptr)
    {
        triggerAsyncUpdate();
    }

    ~TimerThread() noexcept
    {
        signalThreadShouldExit();
        callbackArrived.signal();
        stopThread (4000);

        jassert (instance == this || instance == nullptr);
        if (instance == this)
            instance = nullptr;
    }

    void run() override
    {
        uint32 lastTime = Time::getMillisecondCounter();
        MessageManager::MessageBase::Ptr messageToSend (new CallTimersMessage());

        while (! threadShouldExit())
        {
            const uint32 now = Time::getMillisecondCounter();

            const int elapsed = (int) (now >= lastTime ? (now - lastTime)
                                                       : (std::numeric_limits<uint32>::max() - (lastTime - now)));
            lastTime = now;

            const int timeUntilFirstTimer = getTimeUntilFirstTimer (elapsed);

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
        const LockType::ScopedLockType sl (lock);

        while (firstTimer != nullptr && firstTimer->timerCountdownMs <= 0)
        {
            Timer* const t = firstTimer;
            t->timerCountdownMs = t->timerPeriodMs;

            removeTimer (t);
            addTimer (t);

            const LockType::ScopedUnlockType ul (lock);

            JUCE_TRY
            {
                t->timerCallback();
            }
            JUCE_CATCH_EXCEPTION
        }

        callbackArrived.signal();
    }

    void callTimersSynchronously()
    {
        if (! isThreadRunning())
        {
            // (This is relied on by some plugins in cases where the MM has
            // had to restart and the async callback never started)
            cancelPendingUpdate();
            triggerAsyncUpdate();
        }

        callTimers();
    }

    static inline void add (Timer* const tim) noexcept
    {
        if (instance == nullptr)
            instance = new TimerThread();

        instance->addTimer (tim);
    }

    static inline void remove (Timer* const tim) noexcept
    {
        if (instance != nullptr)
            instance->removeTimer (tim);
    }

    static inline void resetCounter (Timer* const tim, const int newCounter) noexcept
    {
        if (instance != nullptr)
        {
            tim->timerCountdownMs = newCounter;
            tim->timerPeriodMs = newCounter;

            if ((tim->nextTimer != nullptr && tim->nextTimer->timerCountdownMs < tim->timerCountdownMs)
                 || (tim->previousTimer != nullptr && tim->previousTimer->timerCountdownMs > tim->timerCountdownMs))
            {
                instance->removeTimer (tim);
                instance->addTimer (tim);
            }
        }
    }

    static TimerThread* instance;
    static LockType lock;

private:
    Timer* volatile firstTimer;
    WaitableEvent callbackArrived;

    struct CallTimersMessage  : public MessageManager::MessageBase
    {
        CallTimersMessage() {}

        void messageCallback() override
        {
            if (instance != nullptr)
                instance->callTimers();
        }
    };

    //==============================================================================
    void addTimer (Timer* const t) noexcept
    {
       #if JUCE_DEBUG
        // trying to add a timer that's already here - shouldn't get to this point,
        // so if you get this assertion, let me know!
        jassert (! timerExists (t));
       #endif

        Timer* i = firstTimer;

        if (i == nullptr || i->timerCountdownMs > t->timerCountdownMs)
        {
            t->nextTimer = firstTimer;
            firstTimer = t;
        }
        else
        {
            while (i->nextTimer != nullptr && i->nextTimer->timerCountdownMs <= t->timerCountdownMs)
                i = i->nextTimer;

            jassert (i != nullptr);

            t->nextTimer = i->nextTimer;
            t->previousTimer = i;
            i->nextTimer = t;
        }

        if (t->nextTimer != nullptr)
            t->nextTimer->previousTimer = t;

        jassert ((t->nextTimer == nullptr || t->nextTimer->timerCountdownMs >= t->timerCountdownMs)
                  && (t->previousTimer == nullptr || t->previousTimer->timerCountdownMs <= t->timerCountdownMs));

        notify();
    }

    void removeTimer (Timer* const t) noexcept
    {
       #if JUCE_DEBUG
        // trying to remove a timer that's not here - shouldn't get to this point,
        // so if you get this assertion, let me know!
        jassert (timerExists (t));
       #endif

        if (t->previousTimer != nullptr)
        {
            jassert (firstTimer != t);
            t->previousTimer->nextTimer = t->nextTimer;
        }
        else
        {
            jassert (firstTimer == t);
            firstTimer = t->nextTimer;
        }

        if (t->nextTimer != nullptr)
            t->nextTimer->previousTimer = t->previousTimer;

        t->nextTimer = nullptr;
        t->previousTimer = nullptr;
    }

    int getTimeUntilFirstTimer (const int numMillisecsElapsed) const
    {
        const LockType::ScopedLockType sl (lock);

        for (Timer* t = firstTimer; t != nullptr; t = t->nextTimer)
            t->timerCountdownMs -= numMillisecsElapsed;

        return firstTimer != nullptr ? firstTimer->timerCountdownMs : 1000;
    }

    void handleAsyncUpdate() override
    {
        startThread (7);
    }

   #if JUCE_DEBUG
    bool timerExists (Timer* const t) const noexcept
    {
        for (Timer* tt = firstTimer; tt != nullptr; tt = tt->nextTimer)
            if (tt == t)
                return true;

        return false;
    }
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimerThread)
};

Timer::TimerThread* Timer::TimerThread::instance = nullptr;
Timer::TimerThread::LockType Timer::TimerThread::lock;

//==============================================================================
Timer::Timer() noexcept
   : timerCountdownMs (0),
     timerPeriodMs (0),
     previousTimer (nullptr),
     nextTimer (nullptr)
{
}

Timer::Timer (const Timer&) noexcept
   : timerCountdownMs (0),
     timerPeriodMs (0),
     previousTimer (nullptr),
     nextTimer (nullptr)
{
}

Timer::~Timer()
{
    stopTimer();
}

void Timer::startTimer (const int interval) noexcept
{
    const TimerThread::LockType::ScopedLockType sl (TimerThread::lock);

    if (timerPeriodMs == 0)
    {
        timerCountdownMs = interval;
        timerPeriodMs = jmax (1, interval);
        TimerThread::add (this);
    }
    else
    {
        TimerThread::resetCounter (this, interval);
    }
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
    const TimerThread::LockType::ScopedLockType sl (TimerThread::lock);

    if (timerPeriodMs > 0)
    {
        TimerThread::remove (this);
        timerPeriodMs = 0;
    }
}

void JUCE_CALLTYPE Timer::callPendingTimersSynchronously()
{
    if (TimerThread::instance != nullptr)
        TimerThread::instance->callTimersSynchronously();
}
