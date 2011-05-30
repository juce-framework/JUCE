/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "juce_ThreadPool.h"
#include "../core/juce_Time.h"


//==============================================================================
ThreadPoolJob::ThreadPoolJob (const String& name)
    : jobName (name),
      pool (nullptr),
      shouldStop (false),
      isActive (false),
      shouldBeDeleted (false)
{
}

ThreadPoolJob::~ThreadPoolJob()
{
    // you mustn't delete a job while it's still in a pool! Use ThreadPool::removeJob()
    // to remove it first!
    jassert (pool == nullptr || ! pool->contains (this));
}

String ThreadPoolJob::getJobName() const
{
    return jobName;
}

void ThreadPoolJob::setJobName (const String& newName)
{
    jobName = newName;
}

void ThreadPoolJob::signalJobShouldExit()
{
    shouldStop = true;
}


//==============================================================================
class ThreadPool::ThreadPoolThread  : public Thread
{
public:
    ThreadPoolThread (ThreadPool& pool_)
        : Thread ("Pool"),
          pool (pool_),
          busy (false)
    {
    }

    void run()
    {
        while (! threadShouldExit())
        {
            if (! pool.runNextJob())
                wait (500);
        }
    }

private:
    ThreadPool& pool;
    bool volatile busy;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPoolThread);
};

//==============================================================================
ThreadPool::ThreadPool (const int numThreads,
                        const bool startThreadsOnlyWhenNeeded,
                        const int stopThreadsWhenNotUsedTimeoutMs)
    : threadStopTimeout (stopThreadsWhenNotUsedTimeoutMs),
      priority (5)
{
    jassert (numThreads > 0); // not much point having one of these with no threads in it.

    for (int i = jmax (1, numThreads); --i >= 0;)
        threads.add (new ThreadPoolThread (*this));

    if (! startThreadsOnlyWhenNeeded)
        for (int i = threads.size(); --i >= 0;)
            threads.getUnchecked(i)->startThread (priority);
}

ThreadPool::~ThreadPool()
{
    removeAllJobs (true, 4000);

    int i;
    for (i = threads.size(); --i >= 0;)
        threads.getUnchecked(i)->signalThreadShouldExit();

    for (i = threads.size(); --i >= 0;)
        threads.getUnchecked(i)->stopThread (500);
}

void ThreadPool::addJob (ThreadPoolJob* const job)
{
    jassert (job != nullptr);
    jassert (job->pool == nullptr);

    if (job->pool == nullptr)
    {
        job->pool = this;
        job->shouldStop = false;
        job->isActive = false;

        {
            const ScopedLock sl (lock);
            jobs.add (job);

            int numRunning = 0;

            for (int i = threads.size(); --i >= 0;)
                if (threads.getUnchecked(i)->isThreadRunning() && ! threads.getUnchecked(i)->threadShouldExit())
                    ++numRunning;

            if (numRunning < threads.size())
            {
                bool startedOne = false;
                int n = 1000;

                while (--n >= 0 && ! startedOne)
                {
                    for (int i = threads.size(); --i >= 0;)
                    {
                        if (! threads.getUnchecked(i)->isThreadRunning())
                        {
                            threads.getUnchecked(i)->startThread (priority);
                            startedOne = true;
                            break;
                        }
                    }

                    if (! startedOne)
                        Thread::sleep (2);
                }
            }
        }

        for (int i = threads.size(); --i >= 0;)
            threads.getUnchecked(i)->notify();
    }
}

int ThreadPool::getNumJobs() const
{
    return jobs.size();
}

ThreadPoolJob* ThreadPool::getJob (const int index) const
{
    const ScopedLock sl (lock);
    return jobs [index];
}

bool ThreadPool::contains (const ThreadPoolJob* const job) const
{
    const ScopedLock sl (lock);
    return jobs.contains (const_cast <ThreadPoolJob*> (job));
}

bool ThreadPool::isJobRunning (const ThreadPoolJob* const job) const
{
    const ScopedLock sl (lock);
    return jobs.contains (const_cast <ThreadPoolJob*> (job)) && job->isActive;
}

bool ThreadPool::waitForJobToFinish (const ThreadPoolJob* const job,
                                     const int timeOutMs) const
{
    if (job != nullptr)
    {
        const uint32 start = Time::getMillisecondCounter();

        while (contains (job))
        {
            if (timeOutMs >= 0 && Time::getMillisecondCounter() >= start + timeOutMs)
                return false;

            jobFinishedSignal.wait (2);
        }
    }

    return true;
}

bool ThreadPool::removeJob (ThreadPoolJob* const job,
                            const bool interruptIfRunning,
                            const int timeOutMs)
{
    bool dontWait = true;

    if (job != nullptr)
    {
        const ScopedLock sl (lock);

        if (jobs.contains (job))
        {
            if (job->isActive)
            {
                if (interruptIfRunning)
                    job->signalJobShouldExit();

                dontWait = false;
            }
            else
            {
                jobs.removeValue (job);
                job->pool = nullptr;
            }
        }
    }

    return dontWait || waitForJobToFinish (job, timeOutMs);
}

bool ThreadPool::removeAllJobs (const bool interruptRunningJobs,
                                const int timeOutMs,
                                const bool deleteInactiveJobs,
                                ThreadPool::JobSelector* selectedJobsToRemove)
{
    Array <ThreadPoolJob*> jobsToWaitFor;

    {
        const ScopedLock sl (lock);

        for (int i = jobs.size(); --i >= 0;)
        {
            ThreadPoolJob* const job = jobs.getUnchecked(i);

            if (selectedJobsToRemove == nullptr || selectedJobsToRemove->isJobSuitable (job))
            {
                if (job->isActive)
                {
                    jobsToWaitFor.add (job);

                    if (interruptRunningJobs)
                        job->signalJobShouldExit();
                }
                else
                {
                    jobs.remove (i);

                    if (deleteInactiveJobs)
                        delete job;
                    else
                        job->pool = nullptr;
                }
            }
        }
    }

    const uint32 start = Time::getMillisecondCounter();

    for (;;)
    {
        for (int i = jobsToWaitFor.size(); --i >= 0;)
            if (! isJobRunning (jobsToWaitFor.getUnchecked (i)))
                jobsToWaitFor.remove (i);

        if (jobsToWaitFor.size() == 0)
            break;

        if (timeOutMs >= 0 && Time::getMillisecondCounter() >= start + timeOutMs)
            return false;

        jobFinishedSignal.wait (20);
    }

    return true;
}

StringArray ThreadPool::getNamesOfAllJobs (const bool onlyReturnActiveJobs) const
{
    StringArray s;
    const ScopedLock sl (lock);

    for (int i = 0; i < jobs.size(); ++i)
    {
        const ThreadPoolJob* const job = jobs.getUnchecked(i);
        if (job->isActive || ! onlyReturnActiveJobs)
            s.add (job->getJobName());
    }

    return s;
}

bool ThreadPool::setThreadPriorities (const int newPriority)
{
    bool ok = true;

    if (priority != newPriority)
    {
        priority = newPriority;

        for (int i = threads.size(); --i >= 0;)
            if (! threads.getUnchecked(i)->setPriority (newPriority))
                ok = false;
    }

    return ok;
}

bool ThreadPool::runNextJob()
{
    ThreadPoolJob* job = nullptr;

    {
        const ScopedLock sl (lock);

        for (int i = 0; i < jobs.size(); ++i)
        {
            job = jobs[i];

            if (job != nullptr && ! (job->isActive || job->shouldStop))
                break;

            job = nullptr;
        }

        if (job != nullptr)
            job->isActive = true;

    }

    if (job != nullptr)
    {
        JUCE_TRY
        {
            ThreadPoolJob::JobStatus result = job->runJob();

            lastJobEndTime = Time::getApproximateMillisecondCounter();

            const ScopedLock sl (lock);

            if (jobs.contains (job))
            {
                job->isActive = false;

                if (result != ThreadPoolJob::jobNeedsRunningAgain || job->shouldStop)
                {
                    job->pool = nullptr;
                    job->shouldStop = true;
                    jobs.removeValue (job);

                    if (result == ThreadPoolJob::jobHasFinishedAndShouldBeDeleted)
                        delete job;

                    jobFinishedSignal.signal();
                }
                else
                {
                    // move the job to the end of the queue if it wants another go
                    jobs.move (jobs.indexOf (job), -1);
                }
            }
        }
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS
        catch (...)
        {
            const ScopedLock sl (lock);
            jobs.removeValue (job);
        }
#endif
    }
    else
    {
        if (threadStopTimeout > 0
             && Time::getApproximateMillisecondCounter() > lastJobEndTime + threadStopTimeout)
        {
            const ScopedLock sl (lock);

            if (jobs.size() == 0)
                for (int i = threads.size(); --i >= 0;)
                    threads.getUnchecked(i)->signalThreadShouldExit();
        }
        else
        {
            return false;
        }
    }

    return true;
}

END_JUCE_NAMESPACE
