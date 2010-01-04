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


#include "juce_ThreadPool.h"
#include "../core/juce_Time.h"


//==============================================================================
ThreadPoolJob::ThreadPoolJob (const String& name)
    : jobName (name),
      pool (0),
      shouldStop (false),
      isActive (false),
      shouldBeDeleted (false)
{
}

ThreadPoolJob::~ThreadPoolJob()
{
    // you mustn't delete a job while it's still in a pool! Use ThreadPool::removeJob()
    // to remove it first!
    jassert (pool == 0 || ! pool->contains (this));
}

const String ThreadPoolJob::getJobName() const
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
class ThreadPoolThread  : public Thread
{
    ThreadPool& pool;
    bool volatile busy;

    ThreadPoolThread (const ThreadPoolThread&);
    const ThreadPoolThread& operator= (const ThreadPoolThread&);

public:
    ThreadPoolThread (ThreadPool& pool_)
        : Thread (T("Pool")),
          pool (pool_),
          busy (false)
    {
    }

    ~ThreadPoolThread()
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
};

//==============================================================================
ThreadPool::ThreadPool (const int numThreads_,
                        const bool startThreadsOnlyWhenNeeded,
                        const int stopThreadsWhenNotUsedTimeoutMs)
    : numThreads (jmax (1, numThreads_)),
      threadStopTimeout (stopThreadsWhenNotUsedTimeoutMs),
      priority (5)
{
    jassert (numThreads_ > 0); // not much point having one of these with no threads in it.

    threads.calloc (numThreads);

    for (int i = numThreads; --i >= 0;)
        threads[i] = new ThreadPoolThread (*this);

    if (! startThreadsOnlyWhenNeeded)
        for (int i = numThreads; --i >= 0;)
            threads[i]->startThread (priority);
}

ThreadPool::~ThreadPool()
{
    removeAllJobs (true, 4000);

    int i;
    for (i = numThreads; --i >= 0;)
        threads[i]->signalThreadShouldExit();

    for (i = numThreads; --i >= 0;)
    {
        threads[i]->stopThread (500);
        delete threads[i];
    }
}

void ThreadPool::addJob (ThreadPoolJob* const job)
{
    jassert (job != 0);
    jassert (job->pool == 0);

    if (job->pool == 0)
    {
        job->pool = this;
        job->shouldStop = false;
        job->isActive = false;

        lock.enter();
        jobs.add (job);

        int numRunning = 0;

        int i;
        for (i = numThreads; --i >= 0;)
            if (threads[i]->isThreadRunning() && ! threads[i]->threadShouldExit())
                ++numRunning;

        if (numRunning < numThreads)
        {
            bool startedOne = false;
            int n = 1000;

            while (--n >= 0 && ! startedOne)
            {
                for (i = numThreads; --i >= 0;)
                {
                    if (! threads[i]->isThreadRunning())
                    {
                        threads[i]->startThread (priority);
                        startedOne = true;
                        break;
                    }
                }

                if (! startedOne)
                    Thread::sleep (2);
            }
        }

        lock.exit();

        for (i = numThreads; --i >= 0;)
            threads[i]->notify();
    }
}

int ThreadPool::getNumJobs() const
{
    return jobs.size();
}

ThreadPoolJob* ThreadPool::getJob (const int index) const
{
    const ScopedLock sl (lock);
    return (ThreadPoolJob*) jobs [index];
}

bool ThreadPool::contains (const ThreadPoolJob* const job) const
{
    const ScopedLock sl (lock);
    return jobs.contains ((void*) job);
}

bool ThreadPool::isJobRunning (const ThreadPoolJob* const job) const
{
    const ScopedLock sl (lock);
    return jobs.contains ((void*) job) && job->isActive;
}

bool ThreadPool::waitForJobToFinish (const ThreadPoolJob* const job,
                                     const int timeOutMs) const
{
    if (job != 0)
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
    if (job != 0)
    {
        lock.enter();

        if (jobs.contains (job))
        {
            if (job->isActive)
            {
                if (interruptIfRunning)
                    job->signalJobShouldExit();

                lock.exit();

                return waitForJobToFinish (job, timeOutMs);
            }
            else
            {
                jobs.removeValue (job);
            }
        }

        lock.exit();
    }

    return true;
}

bool ThreadPool::removeAllJobs (const bool interruptRunningJobs,
                                const int timeOutMs,
                                const bool deleteInactiveJobs,
                                ThreadPool::JobSelector* selectedJobsToRemove)
{
    Array <ThreadPoolJob*> jobsToWaitFor;

    lock.enter();

    for (int i = jobs.size(); --i >= 0;)
    {
        ThreadPoolJob* const job = (ThreadPoolJob*) jobs.getUnchecked(i);

        if (selectedJobsToRemove == 0 || selectedJobsToRemove->isJobSuitable (job))
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
            }
        }
    }

    lock.exit();

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

const StringArray ThreadPool::getNamesOfAllJobs (const bool onlyReturnActiveJobs) const
{
    StringArray s;
    const ScopedLock sl (lock);

    for (int i = 0; i < jobs.size(); ++i)
    {
        const ThreadPoolJob* const job = (const ThreadPoolJob*) jobs.getUnchecked(i);
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

        for (int i = numThreads; --i >= 0;)
            if (! threads[i]->setPriority (newPriority))
                ok = false;
    }

    return ok;
}

bool ThreadPool::runNextJob()
{
    lock.enter();
    ThreadPoolJob* job = 0;

    for (int i = 0; i < jobs.size(); ++i)
    {
        job = (ThreadPoolJob*) jobs [i];

        if (job != 0 && ! (job->isActive || job->shouldStop))
            break;

        job = 0;
    }

    if (job != 0)
    {
        job->isActive = true;
        lock.exit();

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
                    job->pool = 0;
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
            lock.enter();
            jobs.removeValue (job);
            lock.exit();
        }
#endif
    }
    else
    {
        lock.exit();

        if (threadStopTimeout > 0
             && Time::getApproximateMillisecondCounter() > lastJobEndTime + threadStopTimeout)
        {
            const ScopedLock sl (lock);

            if (jobs.size() == 0)
                for (int i = numThreads; --i >= 0;)
                    threads[i]->signalThreadShouldExit();
        }
        else
        {
            return false;
        }
    }

    return true;
}

END_JUCE_NAMESPACE
