/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A base class for dispatching analytics events on a dedicated thread.

    This class is particularly useful for sending analytics events to a web
    server without blocking the message thread. It can also save (and restore)
    events that were not dispatched so no information is lost when an internet
    connection is absent or something else prevents successful logging.

    Once startAnalyticsThread is called the logBatchedEvents method is
    periodically invoked on an analytics thread, with the period determined by
    calls to setBatchPeriod. Here events are grouped together into batches, with
    the maximum batch size set by the implementation of getMaximumBatchSize.

    It's important to call stopAnalyticsThread in the destructor of your
    subclass (or before then) to give the analytics thread time to shut down.
    Calling stopAnalyticsThread will, in turn, call stopLoggingEvents, which
    you should use to terminate the currently running logBatchedEvents call.

    @see Analytics, AnalyticsDestination, AnalyticsDestination::AnalyticsEvent

    @tags{Analytics}
*/
class JUCE_API  ThreadedAnalyticsDestination   : public AnalyticsDestination
{
public:
    //==============================================================================
    /**
        Creates a ThreadedAnalyticsDestination.

        @param threadName     used to identify the analytics
                              thread in debug builds
    */
    ThreadedAnalyticsDestination (const String& threadName = "Analytics thread");

    /** Destructor. */
    virtual ~ThreadedAnalyticsDestination();

    //==============================================================================
    /**
        Override this method to provide the maximum batch size you can handle in
        your subclass.

        Calls to logBatchedEvents will contain no more than this number of events.
    */
    virtual int getMaximumBatchSize() = 0;

    /**
        This method will be called periodically on the analytics thread.

        If this method returns false then the subsequent call of this function will
        contain the same events as previous call, plus any new events that have been
        generated in the period between calls. The order of events will not be
        changed. This allows you to retry logging events until they are logged
        successfully.

        @param events        a list of events to be logged
        @returns             if the events were successfully logged
    */
    virtual bool logBatchedEvents (const Array<AnalyticsEvent>& events) = 0;

    /**
        You must always call stopAnalyticsThread in the destructor of your subclass
        (or before then) to give the analytics thread time to shut down.

        Calling stopAnalyticsThread triggers a call to this method. At this point
        you are guaranteed that logBatchedEvents has been called for the last time
        and you should make sure that the current call to logBatchedEvents finishes
        as quickly as possible. This method and a subsequent call to
        saveUnloggedEvents must both complete before the timeout supplied to
        stopAnalyticsThread.

        In a normal use case stopLoggingEvents will be called on the message thread
        from the destructor of your ThreadedAnalyticsDestination subclass, and must
        stop the logBatchedEvents method which is running on the analytics thread.

        @see stopAnalyticsThread
    */
    virtual void stopLoggingEvents() = 0;

    //==============================================================================
    /**
        Call this to set the period between logBatchedEvents invocations.

        This method is thread safe and can be used to implements things like
        exponential backoff in logBatchedEvents calls.

        @param newSubmissionPeriodMilliseconds     the new submission period to
                                                   use in milliseconds
    */
    void setBatchPeriod (int newSubmissionPeriodMilliseconds);

    /**
        Adds an event to the queue, which will ultimately be submitted to
        logBatchedEvents.

        This method is thread safe.

        @param event               the analytics event to add to the queue
    */
    void logEvent (const AnalyticsEvent& event) override final;

protected:
    //==============================================================================
    /**
        Starts the analytics thread, with an initial event batching period.

        @param initialBatchPeriodMilliseconds    the initial event batching period
                                                 in milliseconds
    */
    void startAnalyticsThread (int initialBatchPeriodMilliseconds);

    //==============================================================================
    /**
        Triggers the shutdown of the analytics thread.

        You must call this method in the destructor of your subclass (or before
        then) to give the analytics thread time to shut down.

        This method invokes stopLoggingEvents and you should ensure that both the
        analytics thread and a call to saveUnloggedEvents are able to finish before
        the supplied timeout. This timeout is important because on platforms like
        iOS an app is killed if it takes too long to shut down.

        @param timeoutMilliseconds               the number of milliseconds before
                                                 the analytics thread is forcibly
                                                 terminated
    */
    void stopAnalyticsThread (int timeoutMilliseconds);

private:
    //==============================================================================
    /**
        This method will be called when the analytics thread is shut down,
        giving you the chance to save any analytics events that could not be
        logged. Once saved these events can be put back into the queue of events
        when the ThreadedAnalyticsDestination is recreated via
        restoreUnloggedEvents.

        This method should return as quickly as possible, as both
        stopLoggingEvents and this method need to complete inside the timeout
        set in stopAnalyticsThread.

        @param eventsToSave                  the events that could not be logged

        @see stopAnalyticsThread, stopLoggingEvents, restoreUnloggedEvents
    */
    virtual void saveUnloggedEvents (const std::deque<AnalyticsEvent>& eventsToSave) = 0;

    /**
        The counterpart to saveUnloggedEvents.

        Events added to the event queue provided by this method will be the
        first events supplied to logBatchedEvents calls. Use this method to
        restore any unlogged events previously stored in a call to
        saveUnloggedEvents.

        This method is called on the analytics thread.

        @param restoredEventQueue          place restored events into this queue

        @see saveUnloggedEvents
    */
    virtual void restoreUnloggedEvents (std::deque<AnalyticsEvent>& restoredEventQueue) = 0;

    struct EventDispatcher   : public Thread
    {
        EventDispatcher (const String& threadName, ThreadedAnalyticsDestination&);

        void run() override;
        void addToQueue (const AnalyticsEvent&);

        ThreadedAnalyticsDestination& parent;

        std::deque<AnalyticsEvent> eventQueue;
        CriticalSection queueAccess;

        Atomic<int> batchPeriodMilliseconds { 1000 };

        Array<AnalyticsEvent> eventsToSend;
    };

    const String destinationName;
    EventDispatcher dispatcher;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadedAnalyticsDestination)
};

} // namespace juce
