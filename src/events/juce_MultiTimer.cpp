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

#include "juce_MultiTimer.h"
#include "juce_Timer.h"
#include "../threads/juce_ScopedLock.h"


//==============================================================================
class InternalMultiTimerCallback    : public Timer
{
public:
    InternalMultiTimerCallback (const int timerId_, MultiTimer& owner_)
        : timerId (timerId_),
          owner (owner_)
    {
    }

    ~InternalMultiTimerCallback()
    {
    }

    void timerCallback()
    {
        owner.timerCallback (timerId);
    }

    const int timerId;

private:
    MultiTimer& owner;
};

//==============================================================================
MultiTimer::MultiTimer() throw()
{
}

MultiTimer::MultiTimer (const MultiTimer&) throw()
{
}

MultiTimer::~MultiTimer()
{
    const ScopedLock sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
        delete (InternalMultiTimerCallback*) timers.getUnchecked(i);

    timers.clear();
}

//==============================================================================
void MultiTimer::startTimer (const int timerId, const int intervalInMilliseconds) throw()
{
    const ScopedLock sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        InternalMultiTimerCallback* const t = (InternalMultiTimerCallback*) timers.getUnchecked(i);

        if (t->timerId == timerId)
        {
            t->startTimer (intervalInMilliseconds);
            return;
        }
    }

    InternalMultiTimerCallback* const newTimer = new InternalMultiTimerCallback (timerId, *this);
    timers.add (newTimer);
    newTimer->startTimer (intervalInMilliseconds);
}

void MultiTimer::stopTimer (const int timerId) throw()
{
    const ScopedLock sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        InternalMultiTimerCallback* const t = (InternalMultiTimerCallback*) timers.getUnchecked(i);

        if (t->timerId == timerId)
            t->stopTimer();
    }
}

bool MultiTimer::isTimerRunning (const int timerId) const throw()
{
    const ScopedLock sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        const InternalMultiTimerCallback* const t = (InternalMultiTimerCallback*) timers.getUnchecked(i);
        if (t->timerId == timerId)
            return t->isTimerRunning();
    }

    return false;
}

int MultiTimer::getTimerInterval (const int timerId) const throw()
{
    const ScopedLock sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        const InternalMultiTimerCallback* const t = (InternalMultiTimerCallback*) timers.getUnchecked(i);
        if (t->timerId == timerId)
            return t->getTimerInterval();
    }

    return 0;
}


END_JUCE_NAMESPACE
