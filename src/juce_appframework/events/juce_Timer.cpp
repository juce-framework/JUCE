/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Timer.h"
#include "juce_MessageManager.h"
#include "juce_AsyncUpdater.h"
#include "../application/juce_Application.h"
#include "../application/juce_DeletedAtShutdown.h"
#include "../../juce_core/basics/juce_Time.h"
#include "../../juce_core/threads/juce_Thread.h"
#include "../../juce_core/threads/juce_ScopedLock.h"
#include "../../juce_core/containers/juce_VoidArray.h"


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
        uint32 lastMessageManagerCallback = lastTime;

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

                while (callbackNeeded)
                {
                    wait (4);

                    if (threadShouldExit())
                        return;

                    now = Time::getMillisecondCounter();

                    if (now > lastMessageManagerCallback + 200)
                    {
                        lastMessageManagerCallback = now;
                        MessageManager::inactivityCheckCallback();
                    }
                }
            }
            else
            {
                // don't wait for too long because running this loop also helps keep the
                // Time::getApproximateMillisecondTimer value stay up-to-date
                wait (jlimit (1, 50, timeUntilFirstTimer));
            }

            if (now - lastMessageManagerCallback > 200)
            {
                lastMessageManagerCallback = now;
                MessageManager::inactivityCheckCallback();
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
            callbackNeeded = false;

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

        instance->addTimer (tim);
    }

    static inline void remove (Timer* const tim) throw()
    {
        if (instance != 0)
            instance->removeTimer (tim);
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
