/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

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
          firstTimer (nullptr),
          callbackNeeded (0)
    {
        triggerAsyncUpdate();
    }

    ~TimerThread() noexcept
    {
        stopThread (4000);

        jassert (instance == this || instance == nullptr);
        if (instance == this)
            instance = nullptr;
    }

    void run()
    {
        uint32 lastTime = Time::getMillisecondCounter();
        MessageManager::MessageBase::Ptr messageToSend (new CallTimersMessage());

        while (! threadShouldExit())
        {
            const uint32 now = Time::getMillisecondCounter();

            if (now == lastTime)
            {
                wait (1);
                continue;
            }

            const int elapsed = (int) (now >= lastTime ? (now - lastTime)
                                                       : (std::numeric_limits<uint32>::max() - (lastTime - now)));
            lastTime = now;

            const int timeUntilFirstTimer = getTimeUntilFirstTimer (elapsed);

            if (timeUntilFirstTimer <= 0)
            {
                /* If we managed to set the atomic boolean to true then send a message, this is needed
                   as a memory barrier so the message won't be sent before callbackNeeded is set to true,
                   but if it fails it means the message-thread changed the value from under us so at least
                   some processing is happenening and we can just loop around and try again
                */
                if (callbackNeeded.compareAndSetBool (1, 0))
                {
                    messageToSend->post();

                    /* Sometimes our message can get discarded by the OS (e.g. when running as an RTAS
                       when the app has a modal loop), so this is how long to wait before assuming the
                       message has been lost and trying again.
                    */
                    const uint32 messageDeliveryTimeout = now + 2000;

                    while (callbackNeeded.get() != 0)
                    {
                        wait (4);

                        if (threadShouldExit())
                            return;

                        if (Time::getMillisecondCounter() > messageDeliveryTimeout)
                            break;
                    }
                }
            }
            else
            {
                // don't wait for too long because running this loop also helps keep the
                // Time::getApproximateMillisecondTimer value stay up-to-date
                wait (jlimit (1, 50, timeUntilFirstTimer));
            }
        }
    }

    void callTimers()
    {
        const LockType::ScopedLockType sl (lock);

        while (firstTimer != nullptr && firstTimer->countdownMs <= 0)
        {
            Timer* const t = firstTimer;
            t->countdownMs = t->periodMs;

            removeTimer (t);
            addTimer (t);

            const LockType::ScopedUnlockType ul (lock);

            JUCE_TRY
            {
                t->timerCallback();
            }
            JUCE_CATCH_EXCEPTION
        }

        /* This is needed as a memory barrier to make sure all processing of current timers is done
           before the boolean is set. This set should never fail since if it was false in the first place,
           we wouldn't get a message (so it can't be changed from false to true from under us), and if we
           get a message then the value is true and the other thread can only set it to true again and
           we will get another callback to set it to false.
        */
        callbackNeeded.set (0);
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
            tim->countdownMs = newCounter;
            tim->periodMs = newCounter;

            if ((tim->next != nullptr && tim->next->countdownMs < tim->countdownMs)
                 || (tim->previous != nullptr && tim->previous->countdownMs > tim->countdownMs))
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
    Atomic <int> callbackNeeded;

    struct CallTimersMessage  : public MessageManager::MessageBase
    {
        CallTimersMessage() {}

        void messageCallback()
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

        if (i == nullptr || i->countdownMs > t->countdownMs)
        {
            t->next = firstTimer;
            firstTimer = t;
        }
        else
        {
            while (i->next != nullptr && i->next->countdownMs <= t->countdownMs)
                i = i->next;

            jassert (i != nullptr);

            t->next = i->next;
            t->previous = i;
            i->next = t;
        }

        if (t->next != nullptr)
            t->next->previous = t;

        jassert ((t->next == nullptr || t->next->countdownMs >= t->countdownMs)
                  && (t->previous == nullptr || t->previous->countdownMs <= t->countdownMs));

        notify();
    }

    void removeTimer (Timer* const t) noexcept
    {
       #if JUCE_DEBUG
        // trying to remove a timer that's not here - shouldn't get to this point,
        // so if you get this assertion, let me know!
        jassert (timerExists (t));
       #endif

        if (t->previous != nullptr)
        {
            jassert (firstTimer != t);
            t->previous->next = t->next;
        }
        else
        {
            jassert (firstTimer == t);
            firstTimer = t->next;
        }

        if (t->next != nullptr)
            t->next->previous = t->previous;

        t->next = nullptr;
        t->previous = nullptr;
    }

    int getTimeUntilFirstTimer (const int numMillisecsElapsed) const
    {
        const LockType::ScopedLockType sl (lock);

        for (Timer* t = firstTimer; t != nullptr; t = t->next)
            t->countdownMs -= numMillisecsElapsed;

        return firstTimer != nullptr ? firstTimer->countdownMs : 1000;
    }

    void handleAsyncUpdate()
    {
        startThread (7);
    }

   #if JUCE_DEBUG
    bool timerExists (Timer* const t) const noexcept
    {
        for (Timer* tt = firstTimer; tt != nullptr; tt = tt->next)
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
#if JUCE_DEBUG
static SortedSet <Timer*> activeTimers;
#endif

Timer::Timer() noexcept
   : countdownMs (0),
     periodMs (0),
     previous (nullptr),
     next (nullptr)
{
   #if JUCE_DEBUG
    const TimerThread::LockType::ScopedLockType sl (TimerThread::lock);
    activeTimers.add (this);
   #endif
}

Timer::Timer (const Timer&) noexcept
   : countdownMs (0),
     periodMs (0),
     previous (nullptr),
     next (nullptr)
{
   #if JUCE_DEBUG
    const TimerThread::LockType::ScopedLockType sl (TimerThread::lock);
    activeTimers.add (this);
   #endif
}

Timer::~Timer()
{
    stopTimer();

   #if JUCE_DEBUG
    activeTimers.removeValue (this);
   #endif
}

void Timer::startTimer (const int interval) noexcept
{
    const TimerThread::LockType::ScopedLockType sl (TimerThread::lock);

   #if JUCE_DEBUG
    // this isn't a valid object! Your timer might be a dangling pointer or something..
    jassert (activeTimers.contains (this));
   #endif

    if (periodMs == 0)
    {
        countdownMs = interval;
        periodMs = jmax (1, interval);
        TimerThread::add (this);
    }
    else
    {
        TimerThread::resetCounter (this, interval);
    }
}

void Timer::stopTimer() noexcept
{
    const TimerThread::LockType::ScopedLockType sl (TimerThread::lock);

   #if JUCE_DEBUG
    // this isn't a valid object! Your timer might be a dangling pointer or something..
    jassert (activeTimers.contains (this));
   #endif

    if (periodMs > 0)
    {
        TimerThread::remove (this);
        periodMs = 0;
    }
}

void JUCE_CALLTYPE Timer::callPendingTimersSynchronously()
{
    if (TimerThread::instance != nullptr)
        TimerThread::instance->callTimersSynchronously();
}
