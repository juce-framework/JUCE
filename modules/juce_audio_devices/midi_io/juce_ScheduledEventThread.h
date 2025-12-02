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

/**
    @internal

    Allows events to be queued up, then for each event calls the OutputCallback at the time
    dictated by that event's timestamp.

    Event must have a getTimeStamp() member function that returns the output time of the event.
*/
template <typename Event>
class ScheduledEventThread : private Thread
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
            pendingMessages.clear();
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
            pendingMessages.insert (event);
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
            std::unique_lock lock (mutex);
            condvar.wait (lock, [&]
            {
                return ! pendingMessages.empty() || ! backgroundThreadRunning;
            });

            if (! backgroundThreadRunning)
                return;

            const auto now = Time::getMillisecondCounter();
            const auto event = *pendingMessages.begin();
            pendingMessages.erase (pendingMessages.begin());

            const auto timestamp = event.getTimeStamp();

            if (timestamp > now + 20)
            {
                const auto millis = static_cast<int64_t> (timestamp - (now + 20));
                condvar.wait_for (lock, std::chrono::milliseconds (millis));
                continue;
            }

            if (timestamp > now)
                Time::waitForMillisecondCounter ((uint32) timestamp);

            if (timestamp > now - 200)
                outputCallback (event);
        }
    }

    struct Comparator
    {
        bool operator() (const Event& a, const Event& b) const
        {
            return a.getTimeStamp() < b.getTimeStamp();
        }
    };

    mutable std::mutex mutex;
    std::condition_variable condvar;
    std::multiset<Event, Comparator> pendingMessages;
    OutputCallback outputCallback;
    bool backgroundThreadRunning = false;
};

} // namespace juce
