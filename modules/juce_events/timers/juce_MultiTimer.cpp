/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

class MultiTimer::MultiTimerCallback    : public Timer
{
public:
    MultiTimerCallback (const int timerId_, MultiTimer& owner_)
        : timerId (timerId_),
          owner (owner_)
    {
    }

    void timerCallback() override
    {
        owner.timerCallback (timerId);
    }

    const int timerId;

private:
    MultiTimer& owner;
};

//==============================================================================
MultiTimer::MultiTimer() noexcept
{
}

MultiTimer::MultiTimer (const MultiTimer&) noexcept
{
}

MultiTimer::~MultiTimer()
{
    const SpinLock::ScopedLockType sl (timerListLock);
    timers.clear();
}

//==============================================================================
void MultiTimer::startTimer (const int timerId, const int intervalInMilliseconds) noexcept
{
    const SpinLock::ScopedLockType sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        MultiTimerCallback* const t = timers.getUnchecked(i);

        if (t->timerId == timerId)
        {
            t->startTimer (intervalInMilliseconds);
            return;
        }
    }

    MultiTimerCallback* const newTimer = new MultiTimerCallback (timerId, *this);
    timers.add (newTimer);
    newTimer->startTimer (intervalInMilliseconds);
}

void MultiTimer::stopTimer (const int timerId) noexcept
{
    const SpinLock::ScopedLockType sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        MultiTimerCallback* const t = timers.getUnchecked(i);

        if (t->timerId == timerId)
            t->stopTimer();
    }
}

bool MultiTimer::isTimerRunning (const int timerId) const noexcept
{
    const SpinLock::ScopedLockType sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        const MultiTimerCallback* const t = timers.getUnchecked(i);
        if (t->timerId == timerId)
            return t->isTimerRunning();
    }

    return false;
}

int MultiTimer::getTimerInterval (const int timerId) const noexcept
{
    const SpinLock::ScopedLockType sl (timerListLock);

    for (int i = timers.size(); --i >= 0;)
    {
        const MultiTimerCallback* const t = timers.getUnchecked(i);
        if (t->timerId == timerId)
            return t->getTimerInterval();
    }

    return 0;
}
