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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

/**
    @internal

    A simple type that allows reporting and waiting for millisecond time intervals.
*/
struct TimeProvider
{
    uint32_t getMillisecondCounter() const
    {
        return Time::getMillisecondCounter();
    }

    void waitForMillisecondCounter (uint32_t c) const
    {
        Time::waitForMillisecondCounter (c);
    }
};

/**
    @internal

    Stores events ordered by their timestamps.
    Popping an event will not always retrieve the event with the smallest timestamp, since that
    event might be scheduled after the current timer wraps around. Instead, a read pointer is
    maintained, and popping an event will return the event at the current position of the read
    pointer. Adding an event generally does not affect the position of the read pointer, with the
    exception that adding an event whose time falls between the current time and the next event time
    will also set the read pointer to reference that added event.

    The Event type must have a getTimeStamp() member function that returns the output time of the
    event as a uint32_t.
*/
template <typename Event, typename TimeProvider = TimeProvider>
class ScheduledEventQueue
{
public:
    explicit ScheduledEventQueue (const TimeProvider* tp)
        : timeProvider (tp)
    {
    }

    void clearAllPendingMessages()
    {
        while (! pendingMessages.empty())
            storage.push_back (pendingMessages.extract (pendingMessages.begin()));

        nextMessage = pendingMessages.end();
    }

    bool empty() const
    {
        return nextMessage == pendingMessages.end();
    }

    /*  Add a new event to the queue.

        Events are always considered to be scheduled after the time provider's current time, minus
        some leeway specified by leewayMs.

        An event timestamp that is less than the current provider time minus the leeway value
        indicates that the event is scheduled after the time provider wraps around.

        The purpose of the leeway is to allow the thread that is popping events to 'catch up' with
        the thread(s) that are adding events, in the case that the reader gets stalled.
        It's unlikely for the writer threads to be scheduling events 49.7 days in the future, so new
        events with timestamps in the past up to leewayMs ago (relative to the time provider)
        will be scheduled to play as soon as possible.
    */
    void addEvent (const Event& event, uint32_t leewayMs = 1000 * 60 * 60)
    {
        if (! storage.empty())
        {
            auto extracted = std::move (storage.back());
            storage.pop_back();
            extracted.value() = event;
            pendingMessages.insert (std::move (extracted));
        }
        else
        {
            pendingMessages.insert (event);
        }

        if (nextMessage == pendingMessages.end())
        {
            nextMessage = pendingMessages.begin();
        }
        else
        {
            const auto timebase = (uint32_t) timeProvider->getMillisecondCounter() - leewayMs;
            const auto msToThis = (uint32_t) event.getTimeStamp() - timebase;
            const auto msToNext = (uint32_t) nextMessage->getTimeStamp() - timebase;

            if (msToThis < msToNext)
                nextMessage = std::prev (nextMessage == pendingMessages.begin() ? pendingMessages.end() : nextMessage);
        }
    }

    using PoppedEvent = std::variant<uint32_t, Event>;

    /*  Compares the timestamp of the event currently at the 'next' pointer with the current
        provider time.
        - If the next event time is less than leewayMs milliseconds in the future, or slightly
          in the past, returns that event and removes it from the queue.
        - If the next event time is more than leewayMs milliseconds in the future, returns a
          suggested duration to wait in milliseconds before reattempting to retrieve the event, and
          leaves the event in the queue.
        - If the next event is a long way in the past, removes the event from the queue and returns
          a pause duration of 0, indicating that the calling thread should try to retrieve the
          following event as quickly as possible in order to catch up with the time provider.
        - If the queue is empty, returns an infinite timeout.
    */
    PoppedEvent popEvent (uint32_t leewayMs = 20)
    {
        if (nextMessage == pendingMessages.end())
            return std::numeric_limits<uint32_t>::max();

        const auto now = (uint32_t) timeProvider->getMillisecondCounter();
        const auto timestamp = (uint32_t) nextMessage->getTimeStamp();
        const auto difference = timestamp - now;

        if (leewayMs < difference && difference <= std::numeric_limits<uint32_t>::max() / 2)
            return difference - leewayMs;

        const ScopeGuard scope { [&]
        {
            storage.push_back (pendingMessages.extract (std::exchange (nextMessage, std::next (nextMessage))));

            if (nextMessage == pendingMessages.end())
                nextMessage = pendingMessages.begin();
        } };

        if (difference <= leewayMs || std::numeric_limits<uint32_t>::max() - 200 < difference)
            return std::move (*nextMessage);

        return (uint32_t) 0;
    }

    uint32_t getTimeOfNextEvent() const
    {
        jassert (! empty());
        return nextMessage->getTimeStamp();
    }

private:
    struct Comparator
    {
        bool operator() (const Event& a, const Event& b) const
        {
            return (uint32_t) a.getTimeStamp() < (uint32_t) b.getTimeStamp();
        }
    };

    const TimeProvider* timeProvider = nullptr;
    std::multiset<Event, Comparator> pendingMessages;
    std::vector<typename decltype (pendingMessages)::node_type> storage;
    typename decltype (pendingMessages)::const_iterator nextMessage = pendingMessages.end();
};

/**
    @internal

    Allows events to be queued up, then for each event calls the OutputCallback at the time
    dictated by that event's timestamp.

    The Event type must have a getTimeStamp() member function that returns the output time of the
    event as a uint32_t.
*/
template <typename Event, typename TimeProvider = TimeProvider>
class ScheduledEventThread : private Thread,
                             private TimeProvider
{
public:
    using OutputCallback = std::function<void (const Event&)>;

    explicit ScheduledEventThread (OutputCallback&& c)
        : Thread (SystemStats::getJUCEVersion() + ": MIDI Out"),
          outputCallback (std::move (c))
    {
        jassert (outputCallback != nullptr);
    }

    ~ScheduledEventThread() override
    {
        stop();
    }

    void clearAllPendingMessages()
    {
        {
            const std::scoped_lock sl (mutex);
            queue.clearAllPendingMessages();
        }

        condvar.notify_one();
    }

    void start()
    {
        {
            const std::scoped_lock sl (mutex);
            backgroundThreadRunning = true;
        }

        startThread (Priority::high);
    }

    void stop()
    {
        {
            const std::scoped_lock sl (mutex);
            backgroundThreadRunning = false;
        }

        condvar.notify_one();
        stopThread (-1);
    }

    void addEvent (const Event& event)
    {
        // You've got to call startBackgroundThread() for this to actually work.
        jassert (isThreadRunning());

        {
            const std::scoped_lock sl (mutex);
            queue.addEvent (event);
        }

        condvar.notify_one();
    }

    bool isRunning() const
    {
        const std::scoped_lock sl (mutex);
        return backgroundThreadRunning;
    }

private:
    void run() override
    {
        for (;;)
        {
            struct Return {};

            const auto loopStatus = std::invoke ([&]() -> std::variant<std::monostate, Return, Event>
            {
                std::unique_lock lock (mutex);
                condvar.wait (lock, [&] { return ! queue.empty() || ! backgroundThreadRunning; });

                if (! backgroundThreadRunning)
                    return Return{};

                auto maybeEvent = queue.popEvent();

                if (auto* event = std::get_if<Event> (&maybeEvent))
                    return std::move (*event);

                // Adding an event always notifies the condition variable, so this should always
                // get unblocked, even if waiting on an effectively infinite timeout.
                if (auto* pause = std::get_if<uint32_t> (&maybeEvent))
                    if (*pause != 0)
                        condvar.wait_for (lock, std::chrono::milliseconds { *pause });

                return {};
            });

            if (std::holds_alternative<Return> (loopStatus))
                return;

            if (auto* event = std::get_if<Event> (&loopStatus))
            {
                const auto eventTime = (uint32_t) event->getTimeStamp();
                const auto difference = eventTime - (uint32_t) this->getMillisecondCounter();

                // Difference will either be fairly small (for an event in the future), or
                // huge (for an event in the past). We only want to wait if the difference is small.
                if (difference < std::numeric_limits<uint32_t>::max() / 2)
                    this->waitForMillisecondCounter (eventTime);

                outputCallback (*event);
            }
        }
    }

    mutable std::mutex mutex;
    std::condition_variable condvar;
    ScheduledEventQueue<Event, TimeProvider> queue { this };
    OutputCallback outputCallback;
    bool backgroundThreadRunning = false;
};

} // namespace juce
