/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_Thread.h"
#include "juce_ScopedLock.h"
#include "../core/juce_Time.h"
#include "../containers/juce_VoidArray.h"

// these functions are implemented in the platform-specific code.
void* juce_createThread (void* userData);
void juce_killThread (void* handle);
bool juce_setThreadPriority (void* handle, int priority);
void juce_setCurrentThreadName (const String& name);
#if JUCE_WIN32
void juce_CloseThreadHandle (void* handle);
#endif

//==============================================================================
static VoidArray runningThreads;
static CriticalSection runningThreadsLock;

//==============================================================================
void Thread::threadEntryPoint (Thread* const thread)
{
    {
        const ScopedLock sl (runningThreadsLock);
        runningThreads.add (thread);
    }

    JUCE_TRY
    {
        thread->threadId_ = Thread::getCurrentThreadId();

        if (thread->threadName_.isNotEmpty())
            juce_setCurrentThreadName (thread->threadName_);

        if (thread->startSuspensionEvent_.wait (10000))
        {
            if (thread->affinityMask_ != 0)
                setCurrentThreadAffinityMask (thread->affinityMask_);

            thread->run();
        }
    }
    JUCE_CATCH_ALL_ASSERT

    {
        const ScopedLock sl (runningThreadsLock);

        jassert (runningThreads.contains (thread));
        runningThreads.removeValue (thread);
    }

#if JUCE_WIN32
    juce_CloseThreadHandle (thread->threadHandle_);
#endif

    thread->threadHandle_ = 0;
    thread->threadId_ = 0;
}

// used to wrap the incoming call from the platform-specific code
void JUCE_API juce_threadEntryPoint (void* userData)
{
    Thread::threadEntryPoint ((Thread*) userData);
}


//==============================================================================
Thread::Thread (const String& threadName)
    : threadName_ (threadName),
      threadHandle_ (0),
      threadPriority_ (5),
      threadId_ (0),
      affinityMask_ (0),
      threadShouldExit_ (false)
{
}

Thread::~Thread()
{
    stopThread (100);
}

//==============================================================================
void Thread::startThread()
{
    const ScopedLock sl (startStopLock);

    threadShouldExit_ = false;

    if (threadHandle_ == 0)
    {
        threadHandle_ = juce_createThread ((void*) this);
        juce_setThreadPriority (threadHandle_, threadPriority_);
        startSuspensionEvent_.signal();
    }
}

void Thread::startThread (const int priority)
{
    const ScopedLock sl (startStopLock);

    if (threadHandle_ == 0)
    {
        threadPriority_ = priority;
        startThread();
    }
    else
    {
        setPriority (priority);
    }
}

bool Thread::isThreadRunning() const
{
    return threadHandle_ != 0;
}

//==============================================================================
void Thread::signalThreadShouldExit()
{
    threadShouldExit_ = true;
}

bool Thread::waitForThreadToExit (const int timeOutMilliseconds) const
{
    // Doh! So how exactly do you expect this thread to wait for itself to stop??
    jassert (getThreadId() != getCurrentThreadId());

    const int sleepMsPerIteration = 5;
    int count = timeOutMilliseconds / sleepMsPerIteration;

    while (isThreadRunning())
    {
        if (timeOutMilliseconds > 0 && --count < 0)
            return false;

        sleep (sleepMsPerIteration);
    }

    return true;
}

void Thread::stopThread (const int timeOutMilliseconds)
{
    // agh! You can't stop the thread that's calling this method! How on earth
    // would that work??
    jassert (getCurrentThreadId() != getThreadId());

    const ScopedLock sl (startStopLock);

    if (isThreadRunning())
    {
        signalThreadShouldExit();
        notify();

        if (timeOutMilliseconds != 0)
            waitForThreadToExit (timeOutMilliseconds);

        if (isThreadRunning())
        {
            // very bad karma if this point is reached, as
            // there are bound to be locks and events left in
            // silly states when a thread is killed by force..
            jassertfalse
            Logger::writeToLog ("!! killing thread by force !!");

            juce_killThread (threadHandle_);
            threadHandle_ = 0;
            threadId_ = 0;

            const ScopedLock sl2 (runningThreadsLock);
            runningThreads.removeValue (this);
        }
    }
}

//==============================================================================
bool Thread::setPriority (const int priority)
{
    const ScopedLock sl (startStopLock);

    const bool worked = juce_setThreadPriority (threadHandle_, priority);

    if (worked)
        threadPriority_ = priority;

    return worked;
}

bool Thread::setCurrentThreadPriority (const int priority)
{
    return juce_setThreadPriority (0, priority);
}

void Thread::setAffinityMask (const uint32 affinityMask)
{
    affinityMask_ = affinityMask;
}

//==============================================================================
bool Thread::wait (const int timeOutMilliseconds) const
{
    return defaultEvent_.wait (timeOutMilliseconds);
}

void Thread::notify() const
{
    defaultEvent_.signal();
}

//==============================================================================
int Thread::getNumRunningThreads()
{
    return runningThreads.size();
}

Thread* Thread::getCurrentThread()
{
    const ThreadID thisId = getCurrentThreadId();

    const ScopedLock sl (runningThreadsLock);

    for (int i = runningThreads.size(); --i >= 0;)
    {
        Thread* const t = (Thread*) runningThreads.getUnchecked(i);

        if (t->threadId_ == thisId)
            return t;
    }

    return 0;
}

void Thread::stopAllThreads (const int timeOutMilliseconds)
{
    {
        const ScopedLock sl (runningThreadsLock);

        for (int i = runningThreads.size(); --i >= 0;)
            ((Thread*) runningThreads.getUnchecked(i))->signalThreadShouldExit();
    }

    for (;;)
    {
        runningThreadsLock.enter();
        Thread* const t = (Thread*) runningThreads[0];
        runningThreadsLock.exit();

        if (t == 0)
            break;

        t->stopThread (timeOutMilliseconds);
    }
}

END_JUCE_NAMESPACE
