/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
// sets the process to 0=low priority, 1=normal, 2=high, 3=realtime
JUCE_API void JUCE_CALLTYPE Process::setPriority (ProcessPriority prior)
{
    // TODO

    struct sched_param param;
    int policy, maxp, minp;

    const int p = (int) prior;

    if (p <= 1)
        policy = SCHED_OTHER;
    else
        policy = SCHED_RR;

    minp = sched_get_priority_min (policy);
    maxp = sched_get_priority_max (policy);

    if (p < 2)
        param.sched_priority = 0;
    else if (p == 2 )
        // Set to middle of lower realtime priority range
        param.sched_priority = minp + (maxp - minp) / 4;
    else
        // Set to middle of higher realtime priority range
        param.sched_priority = minp + (3 * (maxp - minp) / 4);

    pthread_setschedparam (pthread_self(), policy, &param);
}

JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger()
{
    return false;
}

JUCE_API bool JUCE_CALLTYPE Process::isRunningUnderDebugger()
{
    return juce_isRunningUnderDebugger();
}

JUCE_API void JUCE_CALLTYPE Process::raisePrivilege() {}
JUCE_API void JUCE_CALLTYPE Process::lowerPrivilege() {}

struct AndroidThreadData
{
    AndroidThreadData (Thread* thread) noexcept
        : owner (thread), tId (0)
    {
    }

    Thread* owner;
    Thread::ThreadID tId;
    WaitableEvent eventSet, eventGet;
};

void JUCE_API juce_threadEntryPoint (void*);

extern "C" void* threadEntryProc (void*);
extern "C" void* threadEntryProc (void* userData)
{
    ScopedPointer<AndroidThreadData> priv (reinterpret_cast<AndroidThreadData*> (userData));
    priv->tId = (Thread::ThreadID) pthread_self();
    priv->eventSet.signal();
    priv->eventGet.wait (-1);

    juce_threadEntryPoint (priv->owner);

    return nullptr;
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024JuceThread), runThread,
                   void, (JNIEnv* env, jobject device, jlong host))
{
    // Java may create a Midi thread which JUCE doesn't know about and this callback may be
    // received on this thread. Java will have already created a JNI Env for this new thread,
    // which we need to tell Juce about
    setEnv (env);

    if (Thread* thread = reinterpret_cast<Thread*> (host))
        threadEntryProc (thread);
}

void Thread::launchThread()
{
    threadHandle = 0;

    ScopedPointer<AndroidThreadData> threadPrivateData = new AndroidThreadData (this);

    jobject juceNewThread = android.activity.callObjectMethod (JuceAppActivity.createNewThread, (jlong) threadPrivateData.get());

    if (jobject juceThread = getEnv()->NewGlobalRef (juceNewThread))
    {
        AndroidThreadData* priv = threadPrivateData.release();

        threadHandle = (void*) juceThread;
        getEnv()->CallVoidMethod (juceThread, JuceThread.start);

        priv->eventSet.wait (-1);
        threadId = priv->tId;
        priv->eventGet.signal();
    }
}

void Thread::closeThreadHandle()
{
    if (threadHandle != 0)
    {
        jobject juceThread = reinterpret_cast<jobject> (threadHandle);
        getEnv()->DeleteGlobalRef (juceThread);
        threadHandle = 0;
    }

    threadId = 0;
}

void Thread::killThread()
{
    if (threadHandle != 0)
    {
        jobject juceThread = reinterpret_cast<jobject> (threadHandle);
        getEnv()->CallVoidMethod (juceThread, JuceThread.stop);
    }
}

void JUCE_CALLTYPE Thread::setCurrentThreadName (const String& name)
{
    LocalRef<jobject> juceThread (getEnv()->CallStaticObjectMethod (JuceThread, JuceThread.currentThread));

    if (jobject t = juceThread.get())
        getEnv()->CallVoidMethod (t, JuceThread.setName, javaString (name).get());
}

bool Thread::setThreadPriority (void* handle, int priority)
{
    if (handle == nullptr)
    {
        LocalRef<jobject> juceThread (getEnv()->CallStaticObjectMethod (JuceThread, JuceThread.currentThread));

        if (jobject t = juceThread.get())
            return setThreadPriority (t, priority);

        return false;
    }

    jobject juceThread = reinterpret_cast<jobject> (handle);

    const int minPriority = 1;
    const int maxPriority = 10;

    jint javaPriority = ((maxPriority - minPriority) * priority) / 10 + minPriority;

    getEnv()->CallVoidMethod (juceThread, JuceThread.setPriority, javaPriority);

    return true;
}

//==============================================================================
struct HighResolutionTimer::Pimpl
{
    struct HighResolutionThread   : public Thread
    {
        HighResolutionThread (HighResolutionTimer::Pimpl& parent)
            : Thread ("High Resolution Timer"), pimpl (parent)
        {
            startThread();
        }

        void run() override
        {
            pimpl.timerThread();
        }

    private:
        HighResolutionTimer::Pimpl& pimpl;
    };

    //==============================================================================
    Pimpl (HighResolutionTimer& t)  : owner (t) {}

    ~Pimpl()
    {
        stop();
    }

    void start (int newPeriod)
    {
        if (periodMs != newPeriod)
        {
            if (thread.get() == nullptr
                 || thread->getThreadId() != Thread::getCurrentThreadId()
                 || thread->threadShouldExit())
            {
                stop();

                periodMs = newPeriod;

                thread = new HighResolutionThread (*this);
            }
            else
            {
                periodMs = newPeriod;
            }
        }
    }

    void stop()
    {
        if (thread.get() != nullptr)
        {
            thread->signalThreadShouldExit();

            if (thread->getThreadId() != Thread::getCurrentThreadId())
            {
                thread->waitForThreadToExit (-1);
                thread = nullptr;
            }
        }
    }

    HighResolutionTimer& owner;
    int volatile periodMs;

private:
    ScopedPointer<Thread> thread;

    void timerThread()
    {
        jassert (thread.get() != nullptr);

        int lastPeriod = periodMs;
        Clock clock (lastPeriod);

        while (! thread->threadShouldExit())
        {
            clock.wait();
            owner.hiResTimerCallback();

            if (lastPeriod != periodMs)
            {
                lastPeriod = periodMs;
                clock = Clock (lastPeriod);
            }
        }

        periodMs = 0;
    }

    struct Clock
    {
        Clock (double millis) noexcept  : delta ((uint64) (millis * 1000000))
        {
        }

        void wait() noexcept
        {
            struct timespec t;
            t.tv_sec  = (time_t) (delta / 1000000000);
            t.tv_nsec = (long)   (delta % 1000000000);
            nanosleep (&t, nullptr);
        }

        uint64 delta;
    };

    static bool setThreadToRealtime (pthread_t thread, uint64 periodMs)
    {
        ignoreUnused (periodMs);
        struct sched_param param;
        param.sched_priority = sched_get_priority_max (SCHED_RR);
        return pthread_setschedparam (thread, SCHED_RR, &param) == 0;
    }

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};
