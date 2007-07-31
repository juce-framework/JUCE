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


#include "juce_ThreadPool.h"
#include "../basics/juce_Time.h"


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

    threads = (Thread**) juce_calloc (sizeof (Thread*) * numThreads);

    for (int i = numThreads; --i >= 0;)
    {
        threads[i] = new ThreadPoolThread (*this);

        if (! startThreadsOnlyWhenNeeded)
            threads[i]->startThread();
    }
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

    juce_free (threads);
}

void ThreadPool::addJob (ThreadPoolJob* const job)
{
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
                for (int i = numThreads; --i >= 0;)
                {
                    if (! threads[i]->isThreadRunning())
                    {
                        threads[i]->startThread();
                        startedOne = true;
                    }
                }

                if (! startedOne)
                    Thread::sleep (5);
            }
        }

        lock.exit();

        for (i = numThreads; --i >= 0;)
            threads[i]->notify();
    }
}

int ThreadPool::getNumJobs() const throw()
{
    return jobs.size();
}

ThreadPoolJob* ThreadPool::getJob (const int index) const
{
    const ScopedLock sl (lock);
    return (ThreadPoolJob*) jobs [index];
}

bool ThreadPool::contains (const ThreadPoolJob* const job) const throw()
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

            Thread::sleep (2);
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
                                const int timeOutMs)
{
    lock.enter();

    for (int i = jobs.size(); --i >= 0;)
    {
        ThreadPoolJob* const job = (ThreadPoolJob*) jobs.getUnchecked(i);

        if (job->isActive)
        {
            if (interruptRunningJobs)
                job->signalJobShouldExit();
        }
        else
        {
            jobs.remove (i);
        }
    }

    lock.exit();

    const uint32 start = Time::getMillisecondCounter();

    while (jobs.size() > 0)
    {
        if (timeOutMs >= 0 && Time::getMillisecondCounter() >= start + timeOutMs)
            return false;

        Thread::sleep (2);
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

void ThreadPool::setThreadPriorities (const int newPriority)
{
    if (priority != newPriority)
    {
        priority = newPriority;

        for (int i = numThreads; --i >= 0;)
            threads[i]->setPriority (newPriority);
    }
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
            lock.enter();

            if (jobs.size() == 0)
            {
                for (int i = numThreads; --i >= 0;)
                    threads[i]->signalThreadShouldExit();
            }

            lock.exit();
        }
        else
        {
            return false;
        }
    }

    return true;
}

END_JUCE_NAMESPACE
