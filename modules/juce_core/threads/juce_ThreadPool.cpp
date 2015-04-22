/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

class ThreadPool::ThreadPoolThread  : public Thread
{
public:
    ThreadPoolThread (ThreadPool& p)
       : Thread ("Pool"), currentJob (nullptr), pool (p)
    {
    }

    void run() override
    {
        while (! threadShouldExit())
            if (! pool.runNextJob (*this))
                wait (500);
    }

    ThreadPoolJob* volatile currentJob;
    ThreadPool& pool;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPoolThread)
};

//==============================================================================
ThreadPoolJob::ThreadPoolJob (const String& name)
    : jobName (name), pool (nullptr),
      shouldStop (false), isActive (false), shouldBeDeleted (false)
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

ThreadPoolJob* ThreadPoolJob::getCurrentThreadPoolJob()
{
    if (ThreadPool::ThreadPoolThread* t = dynamic_cast<ThreadPool::ThreadPoolThread*> (Thread::getCurrentThread()))
        return t->currentJob;

    return nullptr;
}

//==============================================================================
ThreadPool::ThreadPool (const int numThreads)
{
    jassert (numThreads > 0); // not much point having a pool without any threads!

    createThreads (numThreads);
}

ThreadPool::ThreadPool()
{
    createThreads (SystemStats::getNumCpus());
}

ThreadPool::~ThreadPool()
{
    removeAllJobs (true, 5000);
    stopThreads();
}

void ThreadPool::createThreads (int numThreads)
{
    for (int i = jmax (1, numThreads); --i >= 0;)
        threads.add (new ThreadPoolThread (*this));

    for (int i = threads.size(); --i >= 0;)
        threads.getUnchecked(i)->startThread();
}

void ThreadPool::stopThreads()
{
    for (int i = threads.size(); --i >= 0;)
        threads.getUnchecked(i)->signalThreadShouldExit();

    for (int i = threads.size(); --i >= 0;)
        threads.getUnchecked(i)->stopThread (500);
}

void ThreadPool::addJob (ThreadPoolJob* const job, const bool deleteJobWhenFinished)
{
    jassert (job != nullptr);
    jassert (job->pool == nullptr);

    if (job->pool == nullptr)
    {
        job->pool = this;
        job->shouldStop = false;
        job->isActive = false;
        job->shouldBeDeleted = deleteJobWhenFinished;

        {
            const ScopedLock sl (lock);
            jobs.add (job);
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

bool ThreadPool::waitForJobToFinish (const ThreadPoolJob* const job, const int timeOutMs) const
{
    if (job != nullptr)
    {
        const uint32 start = Time::getMillisecondCounter();

        while (contains (job))
        {
            if (timeOutMs >= 0 && Time::getMillisecondCounter() >= start + (uint32) timeOutMs)
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
    OwnedArray<ThreadPoolJob> deletionList;

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
                jobs.removeFirstMatchingValue (job);
                addToDeleteList (deletionList, job);
            }
        }
    }

    return dontWait || waitForJobToFinish (job, timeOutMs);
}

bool ThreadPool::removeAllJobs (const bool interruptRunningJobs, const int timeOutMs,
                                ThreadPool::JobSelector* const selectedJobsToRemove)
{
    Array <ThreadPoolJob*> jobsToWaitFor;

    {
        OwnedArray<ThreadPoolJob> deletionList;

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
                        addToDeleteList (deletionList, job);
                    }
                }
            }
        }
    }

    const uint32 start = Time::getMillisecondCounter();

    for (;;)
    {
        for (int i = jobsToWaitFor.size(); --i >= 0;)
        {
            ThreadPoolJob* const job = jobsToWaitFor.getUnchecked (i);

            if (! isJobRunning (job))
                jobsToWaitFor.remove (i);
        }

        if (jobsToWaitFor.size() == 0)
            break;

        if (timeOutMs >= 0 && Time::getMillisecondCounter() >= start + (uint32) timeOutMs)
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

    for (int i = threads.size(); --i >= 0;)
        if (! threads.getUnchecked(i)->setPriority (newPriority))
            ok = false;

    return ok;
}

ThreadPoolJob* ThreadPool::pickNextJobToRun()
{
    OwnedArray<ThreadPoolJob> deletionList;

    {
        const ScopedLock sl (lock);

        for (int i = 0; i < jobs.size(); ++i)
        {
            ThreadPoolJob* job = jobs[i];

            if (job != nullptr && ! job->isActive)
            {
                if (job->shouldStop)
                {
                    jobs.remove (i);
                    addToDeleteList (deletionList, job);
                    --i;
                    continue;
                }

                job->isActive = true;
                return job;
            }
        }
    }

    return nullptr;
}

bool ThreadPool::runNextJob (ThreadPoolThread& thread)
{
    if (ThreadPoolJob* const job = pickNextJobToRun())
    {
        ThreadPoolJob::JobStatus result = ThreadPoolJob::jobHasFinished;
        thread.currentJob = job;

        JUCE_TRY
        {
            result = job->runJob();
        }
        JUCE_CATCH_ALL_ASSERT

        thread.currentJob = nullptr;

        OwnedArray<ThreadPoolJob> deletionList;

        {
            const ScopedLock sl (lock);

            if (jobs.contains (job))
            {
                job->isActive = false;

                if (result != ThreadPoolJob::jobNeedsRunningAgain || job->shouldStop)
                {
                    jobs.removeFirstMatchingValue (job);
                    addToDeleteList (deletionList, job);

                    jobFinishedSignal.signal();
                }
                else
                {
                    // move the job to the end of the queue if it wants another go
                    jobs.move (jobs.indexOf (job), -1);
                }
            }
        }

        return true;
    }

    return false;
}

void ThreadPool::addToDeleteList (OwnedArray<ThreadPoolJob>& deletionList, ThreadPoolJob* const job) const
{
    job->shouldStop = true;
    job->pool = nullptr;

    if (job->shouldBeDeleted)
        deletionList.add (job);
}
