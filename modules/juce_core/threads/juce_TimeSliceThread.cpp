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

TimeSliceThread::TimeSliceThread (const String& name)
    : Thread (name),
      clientBeingCalled (nullptr)
{
}

TimeSliceThread::~TimeSliceThread()
{
    stopThread (2000);
}

//==============================================================================
void TimeSliceThread::addTimeSliceClient (TimeSliceClient* const client, int millisecondsBeforeStarting)
{
    if (client != nullptr)
    {
        const ScopedLock sl (listLock);
        client->nextCallTime = Time::getCurrentTime() + RelativeTime::milliseconds (millisecondsBeforeStarting);
        clients.addIfNotAlreadyThere (client);
        notify();
    }
}

void TimeSliceThread::removeTimeSliceClient (TimeSliceClient* const client)
{
    const ScopedLock sl1 (listLock);

    // if there's a chance we're in the middle of calling this client, we need to
    // also lock the outer lock..
    if (clientBeingCalled == client)
    {
        const ScopedUnlock ul (listLock); // unlock first to get the order right..

        const ScopedLock sl2 (callbackLock);
        const ScopedLock sl3 (listLock);

        clients.removeFirstMatchingValue (client);
    }
    else
    {
        clients.removeFirstMatchingValue (client);
    }
}

void TimeSliceThread::moveToFrontOfQueue (TimeSliceClient* client)
{
    const ScopedLock sl (listLock);

    if (clients.contains (client))
    {
        client->nextCallTime = Time::getCurrentTime();
        notify();
    }
}

int TimeSliceThread::getNumClients() const
{
    return clients.size();
}

TimeSliceClient* TimeSliceThread::getClient (const int i) const
{
    const ScopedLock sl (listLock);
    return clients [i];
}

//==============================================================================
TimeSliceClient* TimeSliceThread::getNextClient (int index) const
{
    Time soonest;
    TimeSliceClient* client = nullptr;

    for (int i = clients.size(); --i >= 0;)
    {
        TimeSliceClient* const c = clients.getUnchecked ((i + index) % clients.size());

        if (client == nullptr || c->nextCallTime < soonest)
        {
            client = c;
            soonest = c->nextCallTime;
        }
    }

    return client;
}

void TimeSliceThread::run()
{
    int index = 0;

    while (! threadShouldExit())
    {
        int timeToWait = 500;

        {
            Time nextClientTime;

            {
                const ScopedLock sl2 (listLock);

                index = clients.size() > 0 ? ((index + 1) % clients.size()) : 0;

                if (TimeSliceClient* const firstClient = getNextClient (index))
                    nextClientTime = firstClient->nextCallTime;
            }

            const Time now (Time::getCurrentTime());

            if (nextClientTime > now)
            {
                timeToWait = (int) jmin ((int64) 500, (nextClientTime - now).inMilliseconds());
            }
            else
            {
                timeToWait = index == 0 ? 1 : 0;

                const ScopedLock sl (callbackLock);

                {
                    const ScopedLock sl2 (listLock);
                    clientBeingCalled = getNextClient (index);
                }

                if (clientBeingCalled != nullptr)
                {
                    const int msUntilNextCall = clientBeingCalled->useTimeSlice();

                    const ScopedLock sl2 (listLock);

                    if (msUntilNextCall >= 0)
                        clientBeingCalled->nextCallTime = now + RelativeTime::milliseconds (msUntilNextCall);
                    else
                        clients.removeFirstMatchingValue (clientBeingCalled);

                    clientBeingCalled = nullptr;
                }
            }
        }

        if (timeToWait > 0)
            wait (timeToWait);
    }
}
