/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

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
