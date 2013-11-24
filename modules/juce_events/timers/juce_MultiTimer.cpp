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

struct MultiTimerCallback    : public Timer
{
    MultiTimerCallback (const int tid, MultiTimer& mt) noexcept
        : owner (mt), timerID (tid)
    {
    }

    void timerCallback() override
    {
        owner.timerCallback (timerID);
    }

    MultiTimer& owner;
    const int timerID;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTimerCallback)
};

//==============================================================================
MultiTimer::MultiTimer() noexcept {}
MultiTimer::MultiTimer (const MultiTimer&) noexcept {}

MultiTimer::~MultiTimer()
{
    const SpinLock::ScopedLockType sl (timerListLock);
    timers.clear();
}

//==============================================================================
Timer* MultiTimer::getCallback (int timerID) const noexcept
{
    for (int i = timers.size(); --i >= 0;)
    {
        MultiTimerCallback* const t = static_cast<MultiTimerCallback*> (timers.getUnchecked(i));

        if (t->timerID == timerID)
            return t;
    }

    return nullptr;
}

void MultiTimer::startTimer (const int timerID, const int intervalInMilliseconds) noexcept
{
    const SpinLock::ScopedLockType sl (timerListLock);

    Timer* timer = getCallback (timerID);

    if (timer == nullptr)
        timers.add (timer = new MultiTimerCallback (timerID, *this));

    timer->startTimer (intervalInMilliseconds);
}

void MultiTimer::stopTimer (const int timerID) noexcept
{
    const SpinLock::ScopedLockType sl (timerListLock);

    if (Timer* const t = getCallback (timerID))
        t->stopTimer();
}

bool MultiTimer::isTimerRunning (const int timerID) const noexcept
{
    const SpinLock::ScopedLockType sl (timerListLock);

    if (Timer* const t = getCallback (timerID))
        return t->isTimerRunning();

    return false;
}

int MultiTimer::getTimerInterval (const int timerID) const noexcept
{
    const SpinLock::ScopedLockType sl (timerListLock);

    if (Timer* const t = getCallback (timerID))
        return t->getTimerInterval();

    return 0;
}
