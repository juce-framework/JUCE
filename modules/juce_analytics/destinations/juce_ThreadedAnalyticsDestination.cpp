/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

ThreadedAnalyticsDestination::ThreadedAnalyticsDestination (const String& threadName)
    : dispatcher (threadName, *this)
{}

ThreadedAnalyticsDestination::~ThreadedAnalyticsDestination()
{
    // If you hit this assertion then the analytics thread has not been shut down
    // before this class is destroyed. Call stopAnalyticsThread() in your destructor!
    jassert (! dispatcher.isThreadRunning());
}

void ThreadedAnalyticsDestination::setBatchPeriod (int newBatchPeriodMilliseconds)
{
    dispatcher.batchPeriodMilliseconds = newBatchPeriodMilliseconds;
}

void ThreadedAnalyticsDestination::logEvent (const AnalyticsEvent& event)
{
    dispatcher.addToQueue (event);
}

void ThreadedAnalyticsDestination::startAnalyticsThread (int initialBatchPeriodMilliseconds)
{
    setBatchPeriod (initialBatchPeriodMilliseconds);
    dispatcher.startThread();
}

void ThreadedAnalyticsDestination::stopAnalyticsThread (int timeout)
{
    dispatcher.signalThreadShouldExit();
    stopLoggingEvents();
    dispatcher.stopThread (timeout);

    if (dispatcher.eventQueue.size() > 0)
        saveUnloggedEvents (dispatcher.eventQueue);
}

ThreadedAnalyticsDestination::EventDispatcher::EventDispatcher (const String& dispatcherThreadName,
                                                                ThreadedAnalyticsDestination& destination)
    : Thread (dispatcherThreadName),
      parent (destination)
{}

void ThreadedAnalyticsDestination::EventDispatcher::run()
{
    // We may have inserted some events into the queue (on the message thread)
    // before this thread has started, so make sure the old events are at the
    // front of the queue.
    {
        std::deque<AnalyticsEvent> restoredEventQueue;
        parent.restoreUnloggedEvents (restoredEventQueue);

        const ScopedLock lock (queueAccess);

        for (auto rit = restoredEventQueue.rbegin(); rit != restoredEventQueue.rend(); ++rit)
            eventQueue.push_front (*rit);
    }

    const int maxBatchSize = parent.getMaximumBatchSize();

    while (! threadShouldExit())
    {
        {
            const ScopedLock lock (queueAccess);

            const auto numEventsInBatch = eventsToSend.size();
            const auto freeBatchCapacity = maxBatchSize - numEventsInBatch;

            if (freeBatchCapacity > 0)
            {
                const auto numNewEvents = (int) eventQueue.size() - numEventsInBatch;

                if (numNewEvents > 0)
                {
                    const auto numEventsToAdd = jmin (numNewEvents, freeBatchCapacity);
                    const auto newBatchSize = numEventsInBatch + numEventsToAdd;

                    for (auto i = numEventsInBatch; i < newBatchSize; ++i)
                        eventsToSend.add (eventQueue[(size_t) i]);
                }
            }
        }

        const auto submissionTime = Time::getMillisecondCounter();

        if (! eventsToSend.isEmpty())
        {
            if (parent.logBatchedEvents (eventsToSend))
            {
                const ScopedLock lock (queueAccess);

                for (auto i = 0; i < eventsToSend.size(); ++i)
                    eventQueue.pop_front();

                eventsToSend.clearQuick();
            }
        }

        while (Time::getMillisecondCounter() - submissionTime < (uint32) batchPeriodMilliseconds.get())
        {
            if (threadShouldExit())
                return;

            Thread::sleep (100);
        }
    }
}

void ThreadedAnalyticsDestination::EventDispatcher::addToQueue (const AnalyticsEvent& event)
{
    const ScopedLock lock (queueAccess);
    eventQueue.push_back (event);
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

namespace DestinationTestHelpers
{
    //==============================================================================
    struct BasicDestination   : public ThreadedAnalyticsDestination
    {
        BasicDestination (std::deque<AnalyticsEvent>& loggedEvents,
                          std::deque<AnalyticsEvent>& unloggedEvents)
            : ThreadedAnalyticsDestination ("ThreadedAnalyticsDestinationTest"),
              loggedEventQueue (loggedEvents),
              unloggedEventStore (unloggedEvents)
        {
            startAnalyticsThread (20);
        }

        ~BasicDestination() override
        {
            stopAnalyticsThread (1000);
        }

        int getMaximumBatchSize() override
        {
            return 5;
        }

        void saveUnloggedEvents (const std::deque<AnalyticsEvent>& eventsToSave) override
        {
            unloggedEventStore = eventsToSave;
        }

        void restoreUnloggedEvents (std::deque<AnalyticsEvent>& restoredEventQueue) override
        {
            restoredEventQueue = unloggedEventStore;
        }

        bool logBatchedEvents (const Array<AnalyticsEvent>& events) override
        {
            jassert (events.size() <= getMaximumBatchSize());

            if (loggingIsEnabled)
            {
                const ScopedLock lock (eventQueueChanging);

                for (auto& event : events)
                    loggedEventQueue.push_back (event);

                return true;
            }

            return false;
        }

        void stopLoggingEvents() override {}

        void setLoggingEnabled (bool shouldLogEvents)
        {
            loggingIsEnabled = shouldLogEvents;
        }

        std::deque<AnalyticsEvent>& loggedEventQueue;
        std::deque<AnalyticsEvent>& unloggedEventStore;
        bool loggingIsEnabled = true;
        CriticalSection eventQueueChanging;
    };
}

//==============================================================================
struct ThreadedAnalyticsDestinationTests   : public UnitTest
{
    ThreadedAnalyticsDestinationTests()
        : UnitTest ("ThreadedAnalyticsDestination", UnitTestCategories::analytics)
    {}

    void compareEventQueues (const std::deque<AnalyticsDestination::AnalyticsEvent>& a,
                             const std::deque<AnalyticsDestination::AnalyticsEvent>& b)
    {
        const auto numEntries = a.size();
        expectEquals ((int) b.size(), (int) numEntries);

        for (size_t i = 0; i < numEntries; ++i)
        {
            expectEquals (a[i].name, b[i].name);
            expect (a[i].timestamp == b[i].timestamp);
        }
    }

    void runTest() override
    {
        std::deque<AnalyticsDestination::AnalyticsEvent> testEvents;

        for (int i = 0; i < 7; ++i)
            testEvents.push_back ({ String (i), 0, Time::getMillisecondCounter(), {}, "TestUser", {} });

        std::deque<AnalyticsDestination::AnalyticsEvent> loggedEvents, unloggedEvents;

        beginTest ("New events");

        {
            DestinationTestHelpers::BasicDestination destination (loggedEvents, unloggedEvents);

            for (auto& event : testEvents)
                destination.logEvent (event);

            size_t waitTime = 0, numLoggedEvents = 0;

            while (numLoggedEvents < testEvents.size())
            {
                if (waitTime > 4000)
                {
                    expect (waitTime < 4000);
                    break;
                }

                Thread::sleep (40);
                waitTime += 40;

                const ScopedLock lock (destination.eventQueueChanging);
                numLoggedEvents = loggedEvents.size();
            }
        }

        compareEventQueues (loggedEvents, testEvents);
        expect (unloggedEvents.size() == 0);

        loggedEvents.clear();

        beginTest ("Unlogged events");
        {
            DestinationTestHelpers::BasicDestination destination (loggedEvents, unloggedEvents);
            destination.setLoggingEnabled (false);

            for (auto& event : testEvents)
                destination.logEvent (event);
        }

        compareEventQueues (unloggedEvents, testEvents);
        expect (loggedEvents.size() == 0);
    }
};

static ThreadedAnalyticsDestinationTests threadedAnalyticsDestinationTests;

#endif

} // namespace juce
