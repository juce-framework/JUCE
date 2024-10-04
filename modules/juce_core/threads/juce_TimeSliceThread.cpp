/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

TimeSliceThread::TimeSliceThread (const String& name)  : Thread (name)
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

void TimeSliceThread::removeAllClients()
{
    for (;;)
    {
        if (auto* c = getClient (0))
            removeTimeSliceClient (c);
        else
            break;
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
    const ScopedLock sl (listLock);
    return clients.size();
}

TimeSliceClient* TimeSliceThread::getClient (const int i) const
{
    const ScopedLock sl (listLock);
    return clients[i];
}

bool TimeSliceThread::contains (const TimeSliceClient* c) const
{
    const ScopedLock sl (listLock);
    return std::any_of (clients.begin(), clients.end(), [=] (auto* registered) { return registered == c; });
}

//==============================================================================
TimeSliceClient* TimeSliceThread::getNextClient (int index) const
{
    Time soonest;
    TimeSliceClient* client = nullptr;

    for (int i = clients.size(); --i >= 0;)
    {
        auto* c = clients.getUnchecked ((i + index) % clients.size());

        if (c != nullptr && (client == nullptr || c->nextCallTime < soonest))
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
            int numClients = 0;

            {
                const ScopedLock sl2 (listLock);

                numClients = clients.size();
                index = numClients > 0 ? ((index + 1) % numClients) : 0;

                if (auto* firstClient = getNextClient (index))
                    nextClientTime = firstClient->nextCallTime;
            }

            if (numClients > 0)
            {
                auto now = Time::getCurrentTime();

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
        }

        if (timeToWait > 0)
            wait (timeToWait);
    }
}

} // namespace juce
