/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_Thread.h"
#include "juce_ScopedLock.h"
#include "../basics/juce_Time.h"
#include "../containers/juce_VoidArray.h"

// these functions are implemented in the platform-specific code.
void* juce_createThread (void* userData) throw();
void juce_killThread (void* handle) throw();
void juce_setThreadPriority (void* handle, int priority) throw();
void juce_setCurrentThreadName (const String& name) throw();

//==============================================================================
static VoidArray runningThreads (4);
static CriticalSection runningThreadsLock;

//==============================================================================
void Thread::threadEntryPoint (Thread* const thread) throw()
{
    runningThreadsLock.enter();
    runningThreads.add (thread);
    runningThreadsLock.exit();

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

    runningThreadsLock.enter();
    jassert (runningThreads.contains (thread));
    runningThreads.removeValue (thread);
    runningThreadsLock.exit();

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
void Thread::startThread() throw()
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

void Thread::startThread (const int priority) throw()
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

bool Thread::isThreadRunning() const throw()
{
    return threadHandle_ != 0;
}

//==============================================================================
void Thread::signalThreadShouldExit() throw()
{
    threadShouldExit_ = true;
}

bool Thread::waitForThreadToExit (const int timeOutMilliseconds) const throw()
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

void Thread::stopThread (const int timeOutMilliseconds) throw()
{
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

            const ScopedLock sl (runningThreadsLock);
            runningThreads.removeValue (this);
        }
    }
}

//==============================================================================
void Thread::setPriority (const int priority) throw()
{
    const ScopedLock sl (startStopLock);

    threadPriority_ = priority;
    juce_setThreadPriority (threadHandle_, priority);
}

void Thread::setCurrentThreadPriority (const int priority) throw()
{
    juce_setThreadPriority (0, priority);
}

void Thread::setAffinityMask (const uint32 affinityMask) throw()
{
    affinityMask_ = affinityMask;
}

int Thread::getThreadId() const throw()
{
    return threadId_;
}

//==============================================================================
bool Thread::wait (const int timeOutMilliseconds) const throw()
{
    return defaultEvent_.wait (timeOutMilliseconds);
}

void Thread::notify() const throw()
{
    defaultEvent_.signal();
}

//==============================================================================
int Thread::getNumRunningThreads() throw()
{
    return runningThreads.size();
}

Thread* Thread::getCurrentThread() throw()
{
    const int thisId = getCurrentThreadId();
    Thread* result = 0;

    runningThreadsLock.enter();

    for (int i = runningThreads.size(); --i >= 0;)
    {
        Thread* const t = (Thread*) (runningThreads.getUnchecked(i));

        if (t->threadId_ == thisId)
        {
            result = t;
            break;
        }
    }

    runningThreadsLock.exit();

    return result;
}

void Thread::stopAllThreads (const int timeOutMilliseconds) throw()
{
    runningThreadsLock.enter();

    for (int i = runningThreads.size(); --i >= 0;)
        ((Thread*) runningThreads.getUnchecked(i))->signalThreadShouldExit();

    runningThreadsLock.exit();

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
