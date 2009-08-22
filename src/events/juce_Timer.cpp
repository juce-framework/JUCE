/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Timer.h"
#include "juce_MessageManager.h"
#include "juce_AsyncUpdater.h"
#include "../application/juce_Application.h"
#include "../utilities/juce_DeletedAtShutdown.h"
#include "../core/juce_Time.h"
#include "../threads/juce_Thread.h"
#include "../threads/juce_ScopedLock.h"
#include "../containers/juce_VoidArray.h"


//==============================================================================
class InternalTimerThread  : private Thread,
                             private MessageListener,
                             private DeletedAtShutdown,
                             private AsyncUpdater
{
private:
    friend class Timer;
    static InternalTimerThread* instance;
    static CriticalSection lock;

    Timer* volatile firstTimer;
    bool volatile callbackNeeded;

    InternalTimerThread (const InternalTimerThread&);
    const InternalTimerThread& operator= (const InternalTimerThread&);

    void addTimer (Timer* const t) throw()
    {
#ifdef JUCE_DEBUG
        Timer* tt = firstTimer;

        while (tt != 0)
        {
            // trying to add a timer that's already here - shouldn't get to this point,
            // so if you get this assertion, let me know!
            jassert (tt != t);

            tt = tt->next;
        }

        jassert (t->previous == 0 && t->next == 0);
#endif

        Timer* i = firstTimer;

        if (i == 0 || i->countdownMs > t->countdownMs)
        {
            t->next = firstTimer;
            firstTimer = t;
        }
        else
        {
            while (i->next != 0 && i->next->countdownMs <= t->countdownMs)
                i = i->next;

            jassert (i != 0);

            t->next = i->next;
            t->previous = i;
            i->next = t;
        }

        if (t->next != 0)
            t->next->previous = t;

        jassert ((t->next == 0 || t->next->countdownMs >= t->countdownMs)
                  && (t->previous == 0 || t->previous->countdownMs <= t->countdownMs));

        notify();
    }

    void removeTimer (Timer* const t) throw()
    {
#ifdef JUCE_DEBUG
        Timer* tt = firstTimer;
        bool found = false;

        while (tt != 0)
        {
            if (tt == t)
            {
                found = true;
                break;
            }

            tt = tt->next;
        }

        // trying to remove a timer that's not here - shouldn't get to this point,
        // so if you get this assertion, let me know!
        jassert (found);
#endif

        if (t->previous != 0)
        {
            jassert (firstTimer != t);
            t->previous->next = t->next;
        }
        else
        {
            jassert (firstTimer == t);
            firstTimer = t->next;
        }

        if (t->next != 0)
            t->next->previous = t->previous;

        t->next = 0;
        t->previous = 0;
    }

    void decrementAllCounters (const int numMillisecs) const
    {
        Timer* t = firstTimer;

        while (t != 0)
        {
            t->countdownMs -= numMillisecs;
            t = t->next;
        }
    }

    void handleAsyncUpdate()
    {
        startThread (7);
    }

public:
    InternalTimerThread()
        : Thread ("Juce Timer"),
          firstTimer (0),
          callbackNeeded (false)
    {
        triggerAsyncUpdate();
    }

    ~InternalTimerThread() throw()
    {
        stopThread (4000);

        jassert (instance == this || instance == 0);
        if (instance == this)
            instance = 0;
    }

    void run()
    {
        uint32 lastTime = Time::getMillisecondCounter();

        while (! threadShouldExit())
        {
            uint32 now = Time::getMillisecondCounter();

            if (now <= lastTime)
            {
                wait (2);
                continue;
            }

            const int elapsed = now - lastTime;
            lastTime = now;

            lock.enter();
            decrementAllCounters (elapsed);
            const int timeUntilFirstTimer = (firstTimer != 0) ? firstTimer->countdownMs
                                                              : 1000;
            lock.exit();

            if (timeUntilFirstTimer <= 0)
            {
                callbackNeeded = true;
                postMessage (new Message());

                // sometimes, our message could get discarded by the OS (particularly when running as an RTAS when the app has a modal loop),
                // so this is how long to wait before assuming the message has been lost and trying again.
                const uint32 messageDeliveryTimeout = now + 2000;

                while (callbackNeeded)
                {
                    wait (4);

                    if (threadShouldExit())
                        return;

                    now = Time::getMillisecondCounter();

                    if (now > messageDeliveryTimeout)
                        break;
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

    void handleMessage (const Message&)
    {
        const ScopedLock sl (lock);

        while (firstTimer != 0 && firstTimer->countdownMs <= 0)
        {
            Timer* const t = firstTimer;
            t->countdownMs = t->periodMs;

            removeTimer (t);
            addTimer (t);

            const ScopedUnlock ul (lock);

            JUCE_TRY
            {
                t->timerCallback();
            }
            JUCE_CATCH_EXCEPTION
        }

        callbackNeeded = false;
    }

    static void callAnyTimersSynchronously()
    {
        if (InternalTimerThread::instance != 0)
        {
            const Message m;
            InternalTimerThread::instance->handleMessage (m);
        }
    }

    static inline void add (Timer* const tim) throw()
    {
        if (instance == 0)
            instance = new InternalTimerThread();

        const ScopedLock sl (instance->lock);
        instance->addTimer (tim);
    }

    static inline void remove (Timer* const tim) throw()
    {
        if (instance != 0)
        {
            const ScopedLock sl (instance->lock);
            instance->removeTimer (tim);
        }
    }

    static inline void resetCounter (Timer* const tim,
                                     const int newCounter) throw()
    {
        if (instance != 0)
        {
            tim->countdownMs = newCounter;
            tim->periodMs = newCounter;

            if ((tim->next != 0 && tim->next->countdownMs < tim->countdownMs)
                 || (tim->previous != 0 && tim->previous->countdownMs > tim->countdownMs))
            {
                const ScopedLock sl (instance->lock);
                instance->removeTimer (tim);
                instance->addTimer (tim);
            }
        }
    }
};

InternalTimerThread* InternalTimerThread::instance = 0;
CriticalSection InternalTimerThread::lock;

void juce_callAnyTimersSynchronously()
{
    InternalTimerThread::callAnyTimersSynchronously();
}

//==============================================================================
#ifdef JUCE_DEBUG
static SortedSet <Timer*> activeTimers;
#endif

Timer::Timer() throw()
   : countdownMs (0),
     periodMs (0),
     previous (0),
     next (0)
{
#ifdef JUCE_DEBUG
    activeTimers.add (this);
#endif
}

Timer::Timer (const Timer&) throw()
   : countdownMs (0),
     periodMs (0),
     previous (0),
     next (0)
{
#ifdef JUCE_DEBUG
    activeTimers.add (this);
#endif
}

Timer::~Timer()
{
    stopTimer();

#ifdef JUCE_DEBUG
    activeTimers.removeValue (this);
#endif
}

void Timer::startTimer (const int interval) throw()
{
    const ScopedLock sl (InternalTimerThread::lock);

#ifdef JUCE_DEBUG
    // this isn't a valid object! Your timer might be a dangling pointer or something..
    jassert (activeTimers.contains (this));
#endif

    if (periodMs == 0)
    {
        countdownMs = interval;
        periodMs = jmax (1, interval);
        InternalTimerThread::add (this);
    }
    else
    {
        InternalTimerThread::resetCounter (this, interval);
    }
}

void Timer::stopTimer() throw()
{
    const ScopedLock sl (InternalTimerThread::lock);

#ifdef JUCE_DEBUG
    // this isn't a valid object! Your timer might be a dangling pointer or something..
    jassert (activeTimers.contains (this));
#endif

    if (periodMs > 0)
    {
        InternalTimerThread::remove (this);
        periodMs = 0;
    }
}

END_JUCE_NAMESPACE
