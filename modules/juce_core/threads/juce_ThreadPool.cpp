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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

struct ThreadPool::ThreadPoolThread final : public Thread
{
    ThreadPoolThread (ThreadPool& p, const Options& options)
       : Thread { options.threadName, options.threadStackSizeBytes },
         pool { p }
    {
    }

    void run() override
    {
        while (! threadShouldExit())
        {
            if (! pool.runNextJob (*this))
                wait (500);
        }
    }

    std::atomic<ThreadPoolJob*> currentJob { nullptr };

    ThreadPool& pool;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPoolThread)
};

//==============================================================================
ThreadPool::ThreadPool (const Options& options)
{
    // not much point having a pool without any threads!
    jassert (options.numberOfThreads > 0);

    for (int i = jmax (1, options.numberOfThreads); --i >= 0;)
        threads.add (new ThreadPoolThread (*this, options));

    for (auto* t : threads)
        t->startThread (options.desiredThreadPriority);
}

ThreadPool::ThreadPool (int numberOfThreads,
                        size_t threadStackSizeBytes,
                        Thread::Priority desiredThreadPriority)
    : ThreadPool { Options{}.withNumberOfThreads (numberOfThreads)
                            .withThreadStackSizeBytes (threadStackSizeBytes)
                            .withDesiredThreadPriority (desiredThreadPriority) }
{
}

ThreadPool::~ThreadPool()
{
    removeAllJobs (true, 5000);
    stopThreads();
}

void ThreadPool::stopThreads()
{
    for (auto* t : threads)
        t->signalThreadShouldExit();

    for (auto* t : threads)
        t->stopThread (500);
}

void ThreadPool::addJob (ThreadPoolJob* job, bool deleteJobWhenFinished)
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

        for (auto* t : threads)
            t->notify();
    }
}

int ThreadPool::getNumJobs() const noexcept
{
    const ScopedLock sl (lock);
    return jobs.size();
}

int ThreadPool::getNumThreads() const noexcept
{
    return threads.size();
}

ThreadPoolJob* ThreadPool::getJob (int index) const noexcept
{
    const ScopedLock sl (lock);
    return jobs [index];
}

bool ThreadPool::contains (const ThreadPoolJob* job) const noexcept
{
    const ScopedLock sl (lock);
    return jobs.contains (const_cast<ThreadPoolJob*> (job));
}

bool ThreadPool::isJobRunning (const ThreadPoolJob* job) const noexcept
{
    const ScopedLock sl (lock);
    return jobs.contains (const_cast<ThreadPoolJob*> (job)) && job->isActive;
}

void ThreadPool::moveJobToFront (const ThreadPoolJob* job) noexcept
{
    const ScopedLock sl (lock);

    auto index = jobs.indexOf (const_cast<ThreadPoolJob*> (job));

    if (index > 0 && ! job->isActive)
        jobs.move (index, 0);
}

bool ThreadPool::waitForJobToFinish (const ThreadPoolJob* job, int timeOutMs) const
{
    if (job != nullptr)
    {
        auto start = Time::getMillisecondCounter();

        while (contains (job))
        {
            if (timeOutMs >= 0 && Time::getMillisecondCounter() >= start + (uint32) timeOutMs)
                return false;

            jobFinishedSignal.wait (2);
        }
    }

    return true;
}

bool ThreadPool::removeJob (ThreadPoolJob* job, bool interruptIfRunning, int timeOutMs)
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

bool ThreadPool::removeAllJobs (bool interruptRunningJobs, int timeOutMs,
                                ThreadPool::JobSelector* selectedJobsToRemove)
{
    Array<ThreadPoolJob*> jobsToWaitFor;

    {
        OwnedArray<ThreadPoolJob> deletionList;

        {
            const ScopedLock sl (lock);

            for (int i = jobs.size(); --i >= 0;)
            {
                auto* job = jobs.getUnchecked (i);

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

    auto start = Time::getMillisecondCounter();

    for (;;)
    {
        for (int i = jobsToWaitFor.size(); --i >= 0;)
        {
            auto* job = jobsToWaitFor.getUnchecked (i);

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

StringArray ThreadPool::getNamesOfAllJobs (bool onlyReturnActiveJobs) const
{
    StringArray s;
    const ScopedLock sl (lock);

    for (auto* job : jobs)
        if (job->isActive || ! onlyReturnActiveJobs)
            s.add (job->getJobName());

    return s;
}

ThreadPoolJob* ThreadPool::pickNextJobToRun()
{
    OwnedArray<ThreadPoolJob> deletionList;

    {
        const ScopedLock sl (lock);

        for (int i = 0; i < jobs.size(); ++i)
        {
            if (auto* job = jobs[i])
            {
                if (! job->isActive)
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
    }

    return nullptr;
}

bool ThreadPool::runNextJob (ThreadPoolThread& thread)
{
    if (auto* job = pickNextJobToRun())
    {
        auto result = ThreadPoolJob::jobHasFinished;
        thread.currentJob = job;

        try
        {
            result = job->runJob();
        }
        catch (...)
        {
            jassertfalse; // Your runJob() method mustn't throw any exceptions!
        }

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

void ThreadPool::addToDeleteList (OwnedArray<ThreadPoolJob>& deletionList, ThreadPoolJob* job) const
{
    job->shouldStop = true;
    job->pool = nullptr;

    if (job->shouldBeDeleted)
        deletionList.add (job);
}

} // namespace juce
