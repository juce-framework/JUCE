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

#include "juce_MultiTimer.h"
#include "juce_Timer.h"
#include "../../juce_core/threads/juce_ScopedLock.h"


//==============================================================================
class InternalMultiTimerCallback    : public Timer
{
public:
    InternalMultiTimerCallback (const int id_, MultiTimer& owner_)
        : id (id_),
          owner (owner_)
    {
    }

    ~InternalMultiTimerCallback()
    {
    }

    void timerCallback()
    {
        owner.timerCallback (id);
    }

    const int id;

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

        if (t->id == timerId)
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

        if (t->id == timerId)
            t->stopTimer();
    }
}

bool MultiTimer::isTimerRunning (const int timerId) const throw()
{
    const ScopedLock sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        const InternalMultiTimerCallback* const t = (InternalMultiTimerCallback*) timers.getUnchecked(i);
        if (t->id == timerId)
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
        if (t->id == timerId)
            return t->getTimerInterval();
    }

    return 0;
}


END_JUCE_NAMESPACE
