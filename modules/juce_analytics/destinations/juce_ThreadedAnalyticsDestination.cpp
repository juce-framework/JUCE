/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

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

ThreadedAnalyticsDestination::EventDispatcher::EventDispatcher (const String& threadName,
                                                                ThreadedAnalyticsDestination& destination)
    : Thread (threadName),
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
        auto eventsToSendCapacity = maxBatchSize - eventsToSend.size();

        if (eventsToSendCapacity > 0)
        {
            const ScopedLock lock (queueAccess);

            const auto numEventsInQueue = (int) eventQueue.size();

            if (numEventsInQueue > 0)
            {
                const auto numEventsToAdd = jmin (eventsToSendCapacity, numEventsInQueue);

                for (size_t i = 0; i < (size_t) numEventsToAdd; ++i)
                    eventsToSend.add (eventQueue[i]);
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
#if JUCE_UNIT_TESTS

namespace DestinationTestHelpers
{
    //==============================================================================
    struct TestDestination   : public ThreadedAnalyticsDestination
    {
        TestDestination (std::deque<AnalyticsEvent>& loggedEvents,
                         std::deque<AnalyticsEvent>& unloggedEvents)
            : ThreadedAnalyticsDestination ("ThreadedAnalyticsDestinationTest"),
              loggedEventQueue (loggedEvents),
              unloggedEventStore (unloggedEvents)
        {}

        virtual ~TestDestination() {}

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

        std::deque<AnalyticsEvent>& loggedEventQueue;
        std::deque<AnalyticsEvent>& unloggedEventStore;
    };

    //==============================================================================
    struct BasicDestination   : public TestDestination
    {
        BasicDestination (std::deque<AnalyticsEvent>& loggedEvents,
                          std::deque<AnalyticsEvent>& unloggedEvents)
            : TestDestination (loggedEvents, unloggedEvents)
        {
            startAnalyticsThread (100);
        }

        virtual ~BasicDestination()
        {
            stopAnalyticsThread (1000);
        }


        bool logBatchedEvents (const Array<AnalyticsEvent>& events) override
        {
            jassert (events.size() <= getMaximumBatchSize());

            for (auto& event : events)
                loggedEventQueue.push_back (event);

            return true;
        }

        void stopLoggingEvents() override {}
    };

    //==============================================================================
    struct SlowWebDestination   : public TestDestination
    {
        SlowWebDestination (std::deque<AnalyticsEvent>& loggedEvents,
                            std::deque<AnalyticsEvent>& unloggedEvents)
            : TestDestination (loggedEvents, unloggedEvents)
        {
            startAnalyticsThread (initialPeriod);
        }

        virtual ~SlowWebDestination()
        {
            stopAnalyticsThread (1000);
        }

        bool logBatchedEvents (const Array<AnalyticsEvent>& events) override
        {
            threadHasStarted.signal();

            jassert (events.size() <= getMaximumBatchSize());

            {
                const ScopedLock lock (webStreamCreation);

                if (shouldExit)
                    return false;

                // An attempt to connect to an unroutable IP address will hang
                // indefinitely, which simulates a very slow server
                webStream = new WebInputStream (URL ("http://1.192.0.0"), true);
            }

            String data;

            for (auto& event : events)
                data << event.name;

            webStream->withExtraHeaders (data);

            const auto success = webStream->connect (nullptr);

            // Exponential backoff on failure
            if (success)
                period = initialPeriod;
            else
                period *= 2;

            setBatchPeriod (period);

            return success;
        }

        void stopLoggingEvents() override
        {
            const ScopedLock lock (webStreamCreation);

            shouldExit = true;

            if (webStream != nullptr)
                webStream->cancel();
        }

        const int initialPeriod = 100;
        int period = initialPeriod;

        CriticalSection webStreamCreation;
        bool shouldExit = false;

        ScopedPointer<WebInputStream> webStream;

        WaitableEvent threadHasStarted;
    };
}

//==============================================================================
struct ThreadedAnalyticsDestinationTests   : public UnitTest
{
    ThreadedAnalyticsDestinationTests()
        : UnitTest ("ThreadedAnalyticsDestination")
    {}

    void compareEventQueues (const std::deque<AnalyticsDestination::AnalyticsEvent>& a,
                             const std::deque<AnalyticsDestination::AnalyticsEvent>& b)
    {
        const auto numEntries = a.size();
        expectEquals (b.size(), numEntries);

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
            testEvents.push_back ({ String (i), Time::getMillisecondCounter(), {}, "TestUser", {} });

        std::deque<AnalyticsDestination::AnalyticsEvent> loggedEvents, unloggedEvents;

        beginTest ("Basic");
        {
            DestinationTestHelpers::BasicDestination destination (loggedEvents, unloggedEvents);

            for (auto& event : testEvents)
                destination.logEvent (event);

            Thread::sleep (400);

            compareEventQueues (loggedEvents, testEvents);
            expect (unloggedEvents.size() == 0);

            loggedEvents.clear();
        }

        beginTest ("Web");
        {
            {
                DestinationTestHelpers::SlowWebDestination destination (loggedEvents, unloggedEvents);

                for (auto& event : testEvents)
                    destination.logEvent (event);
            }

            expect (loggedEvents.size() == 0);
            compareEventQueues (unloggedEvents, testEvents);

            {
                DestinationTestHelpers::SlowWebDestination destination (loggedEvents, unloggedEvents);

                destination.threadHasStarted.wait();
                unloggedEvents.clear();
            }

            expect (loggedEvents.size() == 0);
            compareEventQueues (unloggedEvents, testEvents);

            unloggedEvents.clear();
        }
    }
};

static ThreadedAnalyticsDestinationTests threadedAnalyticsDestinationTests;

#endif

} // namespace juce
