/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

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
