/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

//==============================================================================
/**
    Encapsulates a thread.

    Subclasses derive from Thread and implement the run() method, in which they
    do their business. The thread can then be started with the startThread() or
    startRealtimeThread() methods and controlled with various other methods.

    This class also contains some thread-related static methods, such
    as sleep(), yield(), getCurrentThreadId() etc.

    @see CriticalSection, WaitableEvent, Process, ThreadWithProgressWindow,
         MessageManagerLock

    @tags{Core}
*/
class JUCE_API  Thread
{
public:
    //==============================================================================
    static constexpr size_t osDefaultStackSize { 0 };

    //==============================================================================
    /** The different runtime priorities of non-realtime threads.

        @see startThread
    */
    enum class Priority
    {
        /** The highest possible priority that isn't a dedicated realtime thread. */
        highest     = 2,

        /** Makes use of performance cores and higher clocks. */
        high        = 1,

        /** The OS default. It will balance out across all cores. */
        normal      = 0,

        /** Uses efficiency cores when possible. */
        low         = -1,

        /** Restricted to efficiency cores on platforms that have them. */
        background  = -2
    };

    //==============================================================================
    /** A selection of options available when creating realtime threads.

        @see startRealtimeThread
    */
    struct RealtimeOptions
    {
        /** A value with a range of 0-10, where 10 is the highest priority.

            Currently only used by Posix platforms.

            @see getPriority
        */
        [[nodiscard]] RealtimeOptions withPriority (int newPriority) const
        {
            jassert (isPositiveAndNotGreaterThan (newPriority, 10));
            return withMember (*this, &RealtimeOptions::priority, juce::jlimit (0, 10, newPriority));
        }

        /** Specify the expected amount of processing time required each time the thread wakes up.

            Only used by macOS/iOS.

            @see getProcessingTimeMs, withMaximumProcessingTimeMs, withPeriodMs, withPeriodHz
        */
        [[nodiscard]] RealtimeOptions withProcessingTimeMs (double newProcessingTimeMs) const
        {
            jassert (newProcessingTimeMs > 0.0);
            return withMember (*this, &RealtimeOptions::processingTimeMs, newProcessingTimeMs);
        }

        /** Specify the maximum amount of processing time required each time the thread wakes up.

            Only used by macOS/iOS.

            @see getMaximumProcessingTimeMs, withProcessingTimeMs, withPeriodMs, withPeriodHz
        */
        [[nodiscard]] RealtimeOptions withMaximumProcessingTimeMs (double newMaximumProcessingTimeMs) const
        {
            jassert (newMaximumProcessingTimeMs > 0.0);
            return withMember (*this, &RealtimeOptions::maximumProcessingTimeMs, newMaximumProcessingTimeMs);
        }

        /** Specify the maximum amount of processing time required each time the thread wakes up.

            This is identical to 'withMaximumProcessingTimeMs' except it calculates the processing time
            from a sample rate and block size. This is useful if you want to run this thread in parallel
            to an audio device thread.

            Only used by macOS/iOS.

            @see withMaximumProcessingTimeMs, AudioWorkgroup, ScopedWorkgroupToken
        */
        [[nodiscard]] RealtimeOptions withApproximateAudioProcessingTime (int samplesPerFrame, double sampleRate) const
        {
            jassert (samplesPerFrame > 0);
            jassert (sampleRate > 0.0);

            const auto approxFrameTimeMs = (samplesPerFrame / sampleRate) * 1000.0;
            return withMaximumProcessingTimeMs (approxFrameTimeMs);
        }

        /** Specify the approximate amount of time between each thread wake up.

            Alternatively call withPeriodHz().

            Only used by macOS/iOS.

            @see getPeriodMs, withPeriodHz, withProcessingTimeMs, withMaximumProcessingTimeMs,
        */
        [[nodiscard]] RealtimeOptions withPeriodMs (double newPeriodMs) const
        {
            jassert (newPeriodMs > 0.0);
            return withMember (*this, &RealtimeOptions::periodMs, newPeriodMs);
        }

        /** Specify the approximate frequency at which the thread will be woken up.

            Alternatively call withPeriodMs().

            Only used by macOS/iOS.

            @see getPeriodHz, withPeriodMs, withProcessingTimeMs, withMaximumProcessingTimeMs,
        */
        [[nodiscard]] RealtimeOptions withPeriodHz (double newPeriodHz) const
        {
            jassert (newPeriodHz > 0.0);
            return withPeriodMs (1'000.0 / newPeriodHz);
        }

        /** Returns a value with a range of 0-10, where 10 is the highest priority.

            @see withPriority
        */
        [[nodiscard]] int getPriority() const
        {
            return priority;
        }

        /** Returns the expected amount of processing time required each time the thread
            wakes up.

            @see withProcessingTimeMs, getMaximumProcessingTimeMs, getPeriodMs
        */
        [[nodiscard]] std::optional<double> getProcessingTimeMs() const
        {
            return processingTimeMs;
        }

        /** Returns the maximum amount of processing time required each time the thread
            wakes up.

            @see withMaximumProcessingTimeMs, getProcessingTimeMs, getPeriodMs
        */
        [[nodiscard]] std::optional<double> getMaximumProcessingTimeMs() const
        {
            return maximumProcessingTimeMs;
        }

        /** Returns the approximate amount of time between each thread wake up, or
            nullopt if there is no inherent periodicity.

            @see withPeriodMs, withPeriodHz, getProcessingTimeMs, getMaximumProcessingTimeMs
        */
        [[nodiscard]] std::optional<double> getPeriodMs() const
        {
            return periodMs;
        }

    private:
        int priority { 5 };
        std::optional<double> processingTimeMs;
        std::optional<double> maximumProcessingTimeMs;
        std::optional<double> periodMs{};
    };

    //==============================================================================
    /**
        Creates a thread.

        When first created, the thread is not running. Use the startThread()
        method to start it.

        @param threadName       The name of the thread which typically appears in
                                debug logs and profiles.
        @param threadStackSize  The size of the stack of the thread. If this value
                                is zero then the default stack size of the OS will
                                be used.
    */
    explicit Thread (const String& threadName, size_t threadStackSize = osDefaultStackSize);

    /** Destructor.

        You must never attempt to delete a Thread object while it's still running -
        always call stopThread() and make sure your thread has stopped before deleting
        the object. Failing to do so will throw an assertion, and put you firmly into
        undefined behaviour territory.
    */
    virtual ~Thread();

    //==============================================================================
    /** Must be implemented to perform the thread's actual code.

        Remember that the thread must regularly check the threadShouldExit()
        method whilst running, and if this returns true it should return from
        the run() method as soon as possible to avoid being forcibly killed.

        @see threadShouldExit, startThread
    */
    virtual void run() = 0;

    //==============================================================================
    /** Attempts to start a new thread with default ('Priority::normal') priority.

        This will cause the thread's run() method to be called by a new thread.
        If this thread is already running, startThread() won't do anything.

        If a thread cannot be created with the requested priority, this will return false
        and Thread::run() will not be called. An exception to this is the Android platform,
        which always starts a thread and attempts to upgrade the thread after creation.

        @returns    true if the thread started successfully. false if it was unsuccesful.

        @see stopThread
    */
    bool startThread();

    /** Attempts to start a new thread with a given priority.

        This will cause the thread's run() method to be called by a new thread.
        If this thread is already running, startThread() won't do anything.

        If a thread cannot be created with the requested priority, this will return false
        and Thread::run() will not be called. An exception to this is the Android platform,
        which always starts a thread and attempts to upgrade the thread after creation.

        @param newPriority    Priority the thread should be assigned. This parameter is ignored
                              on Linux.

        @returns    true if the thread started successfully, false if it was unsuccesful.

        @see startThread, setPriority, startRealtimeThread
    */
    bool startThread (Priority newPriority);

    /** Starts the thread with realtime performance characteristics on platforms
        that support it.

        You cannot change the options of a running realtime thread, nor switch
        a non-realtime thread to a realtime thread. To make these changes you must
        first stop the thread and then restart with different options.

        @param options    Realtime options the thread should be created with.

        @see startThread, RealtimeOptions
    */
    bool startRealtimeThread (const RealtimeOptions& options);

    /** Attempts to stop the thread running.

        This method will cause the threadShouldExit() method to return true
        and call notify() in case the thread is currently waiting.

        Hopefully the thread will then respond to this by exiting cleanly, and
        the stopThread method will wait for a given time-period for this to
        happen.

        If the thread is stuck and fails to respond after the timeout, it gets
        forcibly killed, which is a very bad thing to happen, as it could still
        be holding locks, etc. which are needed by other parts of your program.

        @param timeOutMilliseconds  The number of milliseconds to wait for the
                                    thread to finish before killing it by force. A negative
                                    value in here will wait forever.
        @returns    true if the thread was cleanly stopped before the timeout, or false
                    if it had to be killed by force.
        @see signalThreadShouldExit, threadShouldExit, waitForThreadToExit, isThreadRunning
    */
    bool stopThread (int timeOutMilliseconds);

    //==============================================================================
    /** Invokes a lambda or function on its own thread with the default priority.

        This will spin up a Thread object which calls the function and then exits.
        Bear in mind that starting and stopping a thread can be a fairly heavyweight
        operation, so you might prefer to use a ThreadPool if you're kicking off a lot
        of short background tasks.

        Also note that using an anonymous thread makes it very difficult to interrupt
        the function when you need to stop it, e.g. when your app quits. So it's up to
        you to deal with situations where the function may fail to stop in time.

        @param functionToRun  The lambda to be called from the new Thread.

        @returns    true if the thread started successfully, or false if it failed.

        @see launch.
    */
    static bool launch (std::function<void()> functionToRun);

    //==============================================================================
    /** Invokes a lambda or function on its own thread with a custom priority.

        This will spin up a Thread object which calls the function and then exits.
        Bear in mind that starting and stopping a thread can be a fairly heavyweight
        operation, so you might prefer to use a ThreadPool if you're kicking off a lot
        of short background tasks.

        Also note that using an anonymous thread makes it very difficult to interrupt
        the function when you need to stop it, e.g. when your app quits. So it's up to
        you to deal with situations where the function may fail to stop in time.

        @param priority       The priority the thread is started with.
        @param functionToRun  The lambda to be called from the new Thread.

        @returns    true if the thread started successfully, or false if it failed.
    */
    static bool launch (Priority priority, std::function<void()> functionToRun);

    //==============================================================================
    /** Returns true if the thread is currently active */
    bool isThreadRunning() const;

    /** Sets a flag to tell the thread it should stop.

        Calling this means that the threadShouldExit() method will then return true.
        The thread should be regularly checking this to see whether it should exit.

        If your thread makes use of wait(), you might want to call notify() after calling
        this method, to interrupt any waits that might be in progress, and allow it
        to reach a point where it can exit.

        @see threadShouldExit, waitForThreadToExit
    */
    void signalThreadShouldExit();

    /** Checks whether the thread has been told to stop running.

        Threads need to check this regularly, and if it returns true, they should
        return from their run() method at the first possible opportunity.

        @see signalThreadShouldExit, currentThreadShouldExit
    */
    bool threadShouldExit() const;

    /** Checks whether the current thread has been told to stop running.
        On the message thread, this will always return false, otherwise
        it will return threadShouldExit() called on the current thread.

        @see threadShouldExit
    */
    static bool currentThreadShouldExit();

    /** Waits for the thread to stop.
        This will wait until isThreadRunning() is false or until a timeout expires.

        @param timeOutMilliseconds  the time to wait, in milliseconds. If this value
                                    is less than zero, it will wait forever.
        @returns    true if the thread exits, or false if the timeout expires first.
    */
    bool waitForThreadToExit (int timeOutMilliseconds) const;

    //==============================================================================
    /** Used to receive callbacks for thread exit calls */
    class JUCE_API Listener
    {
    public:
        virtual ~Listener() = default;

        /** Called if Thread::signalThreadShouldExit was called.
            @see Thread::threadShouldExit, Thread::addListener, Thread::removeListener
        */
        virtual void exitSignalSent() = 0;
    };

    /** Add a listener to this thread which will receive a callback when
        signalThreadShouldExit was called on this thread.

        @see signalThreadShouldExit, removeListener
    */
    void addListener (Listener*);

    /** Removes a listener added with addListener. */
    void removeListener (Listener*);

    /** Returns true if this Thread represents a realtime thread. */
    bool isRealtime() const;

    //==============================================================================
    /** Sets the affinity mask for the thread.

        This will only have an effect next time the thread is started - i.e. if the
        thread is already running when called, it'll have no effect.

        @see setCurrentThreadAffinityMask
    */
    void setAffinityMask (uint32 affinityMask);

    /** Changes the affinity mask for the caller thread.

        This will change the affinity mask for the thread that calls this static method.

        @see setAffinityMask
    */
    static void JUCE_CALLTYPE setCurrentThreadAffinityMask (uint32 affinityMask);

    //==============================================================================
    /** Suspends the execution of the current thread until the specified timeout period
        has elapsed (note that this may not be exact).

        The timeout period must not be negative and whilst sleeping the thread cannot
        be woken up so it should only be used for short periods of time and when other
        methods such as using a WaitableEvent or CriticalSection are not possible.
    */
    static void JUCE_CALLTYPE sleep (int milliseconds);

    /** Yields the current thread's CPU time-slot and allows a new thread to run.

        If there are no other threads of equal or higher priority currently running then
        this will return immediately and the current thread will continue to run.
    */
    static void JUCE_CALLTYPE yield();

    //==============================================================================
    /** Suspends the execution of this thread until either the specified timeout period
        has elapsed, or another thread calls the notify() method to wake it up.

        A negative timeout value means that the method will wait indefinitely.

        @returns    true if the event has been signalled, false if the timeout expires.
    */
    bool wait (double timeOutMilliseconds) const;

    /** Wakes up the thread.

        If the thread has called the wait() method, this will wake it up.

        @see wait
    */
    void notify() const;

    //==============================================================================
    /** A value type used for thread IDs.

        @see getCurrentThreadId(), getThreadId()
    */
    using ThreadID = void*;

    /** Returns an id that identifies the caller thread.

        To find the ID of a particular thread object, use getThreadId().

        @returns    a unique identifier that identifies the calling thread.
        @see getThreadId
    */
    static ThreadID JUCE_CALLTYPE getCurrentThreadId();

    /** Finds the thread object that is currently running.

        Note that the main UI thread (or other non-JUCE threads) don't have a Thread
        object associated with them, so this will return nullptr.
    */
    static Thread* JUCE_CALLTYPE getCurrentThread();

    /** Returns the ID of this thread.

        That means the ID of this thread object - not of the thread that's calling the method.
        This can change when the thread is started and stopped, and will be invalid if the
        thread's not actually running.

        @see getCurrentThreadId
    */
    ThreadID getThreadId() const noexcept;

    /** Returns the name of the thread. This is the name that gets set in the constructor. */
    const String& getThreadName() const noexcept                    { return threadName; }

    /** Changes the name of the caller thread.

        Different OSes may place different length or content limits on this name.
    */
    static void JUCE_CALLTYPE setCurrentThreadName (const String& newThreadName);

   #if JUCE_ANDROID || DOXYGEN
    //==============================================================================
    /** Initialises the JUCE subsystem for projects not created by the Projucer

        On Android, JUCE needs to be initialised once before it is used. The Projucer
        will automatically generate the necessary java code to do this. However, if
        you are using JUCE without the Projucer or are creating a library made with
        JUCE intended for use in non-JUCE apks, then you must call this method
        manually once on apk startup.

        You can call this method from C++ or directly from java by calling the
        following java method:

        @code
        com.rmsl.juce.Java.initialiseJUCE (myContext);
        @endcode

        Note that the above java method is only available in Android Studio projects
        created by the Projucer. If you need to call this from another type of project
        then you need to add the following java file to
        your project:

        @code
        package com.rmsl.juce;

        public class Java
        {
            static { System.loadLibrary ("juce_jni"); }
            public native static void initialiseJUCE (Context context);
        }
        @endcode

        @param jniEnv   this is a pointer to JNI's JNIEnv variable. Any callback
                        from Java into C++ will have this passed in as it's first
                        parameter.
        @param jContext this is a jobject referring to your app/service/receiver/
                        provider's Context. JUCE needs this for many of it's internal
                        functions.
    */
    static void initialiseJUCE (void* jniEnv, void* jContext);
   #endif

protected:
    //==============================================================================
    /** Returns the current priority of this thread.

        This can only be called from the target thread. Doing so from another thread
        will cause an assert.

        @see setPriority
    */
    Priority getPriority() const;

    /** Attempts to set the priority for this thread. Returns true if the new priority
        was set successfully, false if not.

        This can only be called from the target thread. Doing so from another thread
        will cause an assert.

        @param newPriority The new priority to be applied to the thread. Note: This
                           has no effect on Linux platforms, subsequent calls to
                           'getPriority' will return this value.

        @see Priority
    */
    bool setPriority (Priority newPriority);

private:
    //==============================================================================
    const String threadName;
    std::atomic<void*> threadHandle { nullptr };
    std::atomic<ThreadID> threadId { nullptr };
    std::optional<RealtimeOptions> realtimeOptions = {};
    CriticalSection startStopLock;
    WaitableEvent startSuspensionEvent, defaultEvent;
    size_t threadStackSize;
    uint32 affinityMask = 0;
    bool deleteOnThreadEnd = false;
    std::atomic<bool> shouldExit { false };
    ListenerList<Listener, Array<Listener*, CriticalSection>> listeners;

   #if JUCE_ANDROID || JUCE_LINUX || JUCE_BSD
    std::atomic<Priority> priority;
   #endif

   #ifndef DOXYGEN
    friend void JUCE_API juce_threadEntryPoint (void*);
   #endif

    bool startThreadInternal (Priority);
    bool createNativeThread (Priority);
    void closeThreadHandle();
    void killThread();
    void threadEntryPoint();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Thread)
};

} // namespace juce
