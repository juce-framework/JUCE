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

#ifndef JUCE_THREADPOOL_H_INCLUDED
#define JUCE_THREADPOOL_H_INCLUDED

class ThreadPool;
class ThreadPoolThread;


//==============================================================================
/**
    A task that is executed by a ThreadPool object.

    A ThreadPool keeps a list of ThreadPoolJob objects which are executed by
    its threads.

    The runJob() method needs to be implemented to do the task, and if the code that
    does the work takes a significant time to run, it must keep checking the shouldExit()
    method to see if something is trying to interrupt the job. If shouldExit() returns
    true, the runJob() method must return immediately.

    @see ThreadPool, Thread
*/
class JUCE_API  ThreadPoolJob
{
public:
    //==============================================================================
    /** Creates a thread pool job object.
        After creating your job, add it to a thread pool with ThreadPool::addJob().
    */
    explicit ThreadPoolJob (const String& name);

    /** Destructor. */
    virtual ~ThreadPoolJob();

    //==============================================================================
    /** Returns the name of this job.
        @see setJobName
    */
    String getJobName() const;

    /** Changes the job's name.
        @see getJobName
    */
    void setJobName (const String& newName);

    //==============================================================================
    /** These are the values that can be returned by the runJob() method.
    */
    enum JobStatus
    {
        jobHasFinished = 0,     /**< indicates that the job has finished and can be
                                     removed from the pool. */

        jobNeedsRunningAgain    /**< indicates that the job would like to be called
                                     again when a thread is free. */
    };

    /** Peforms the actual work that this job needs to do.

        Your subclass must implement this method, in which is does its work.

        If the code in this method takes a significant time to run, it must repeatedly check
        the shouldExit() method to see if something is trying to interrupt the job.
        If shouldExit() ever returns true, the runJob() method must return immediately.

        If this method returns jobHasFinished, then the job will be removed from the pool
        immediately. If it returns jobNeedsRunningAgain, then the job will be left in the
        pool and will get a chance to run again as soon as a thread is free.

        @see shouldExit()
    */
    virtual JobStatus runJob() = 0;


    //==============================================================================
    /** Returns true if this job is currently running its runJob() method. */
    bool isRunning() const noexcept                     { return isActive; }

    /** Returns true if something is trying to interrupt this job and make it stop.

        Your runJob() method must call this whenever it gets a chance, and if it ever
        returns true, the runJob() method must return immediately.

        @see signalJobShouldExit()
    */
    bool shouldExit() const noexcept                    { return shouldStop; }

    /** Calling this will cause the shouldExit() method to return true, and the job
        should (if it's been implemented correctly) stop as soon as possible.

        @see shouldExit()
    */
    void signalJobShouldExit();

    //==============================================================================
    /** If the calling thread is being invoked inside a runJob() method, this will
        return the ThreadPoolJob that it belongs to.
    */
    static ThreadPoolJob* getCurrentThreadPoolJob();

    //==============================================================================
private:
    friend class ThreadPool;
    friend class ThreadPoolThread;
    String jobName;
    ThreadPool* pool;
    bool shouldStop, isActive, shouldBeDeleted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPoolJob)
};


//==============================================================================
/**
    A set of threads that will run a list of jobs.

    When a ThreadPoolJob object is added to the ThreadPool's list, its runJob() method
    will be called by the next pooled thread that becomes free.

    @see ThreadPoolJob, Thread
*/
class JUCE_API  ThreadPool
{
public:
    //==============================================================================
    /** Creates a thread pool.
        Once you've created a pool, you can give it some jobs by calling addJob().
        @param numberOfThreads  the number of threads to run. These will be started
                                immediately, and will run until the pool is deleted.
    */
    ThreadPool (int numberOfThreads);

    /** Creates a thread pool with one thread per CPU core.
        Once you've created a pool, you can give it some jobs by calling addJob().
        If you want to specify the number of threads, use the other constructor; this
        one creates a pool which has one thread for each CPU core.
        @see SystemStats::getNumCpus()
    */
    ThreadPool();

    /** Destructor.

        This will attempt to remove all the jobs before deleting, but if you want to
        specify a timeout, you should call removeAllJobs() explicitly before deleting
        the pool.
    */
    ~ThreadPool();

    //==============================================================================
    /** A callback class used when you need to select which ThreadPoolJob objects are suitable
        for some kind of operation.
        @see ThreadPool::removeAllJobs
    */
    class JUCE_API  JobSelector
    {
    public:
        virtual ~JobSelector() {}

        /** Should return true if the specified thread matches your criteria for whatever
            operation that this object is being used for.

            Any implementation of this method must be extremely fast and thread-safe!
        */
        virtual bool isJobSuitable (ThreadPoolJob* job) = 0;
    };

    //==============================================================================
    /** Adds a job to the queue.

        Once a job has been added, then the next time a thread is free, it will run
        the job's ThreadPoolJob::runJob() method. Depending on the return value of the
        runJob() method, the pool will either remove the job from the pool or add it to
        the back of the queue to be run again.

        If deleteJobWhenFinished is true, then the job object will be owned and deleted by
        the pool when not needed - if you do this, make sure that your object's destructor
        is thread-safe.

        If deleteJobWhenFinished is false, the pointer will be used but not deleted, and
        the caller is responsible for making sure the object is not deleted before it has
        been removed from the pool.
    */
    void addJob (ThreadPoolJob* job,
                 bool deleteJobWhenFinished);

    /** Tries to remove a job from the pool.

        If the job isn't yet running, this will simply remove it. If it is running, it
        will wait for it to finish.

        If the timeout period expires before the job finishes running, then the job will be
        left in the pool and this will return false. It returns true if the job is successfully
        stopped and removed.

        @param job                  the job to remove
        @param interruptIfRunning   if true, then if the job is currently busy, its
                                    ThreadPoolJob::signalJobShouldExit() method will be called to try
                                    to interrupt it. If false, then if the job will be allowed to run
                                    until it stops normally (or the timeout expires)
        @param timeOutMilliseconds  the length of time this method should wait for the job to finish
                                    before giving up and returning false
    */
    bool removeJob (ThreadPoolJob* job,
                    bool interruptIfRunning,
                    int timeOutMilliseconds);

    /** Tries to remove all jobs from the pool.

        @param interruptRunningJobs if true, then all running jobs will have their ThreadPoolJob::signalJobShouldExit()
                                    methods called to try to interrupt them
        @param timeOutMilliseconds  the length of time this method should wait for all the jobs to finish
                                    before giving up and returning false
        @param selectedJobsToRemove if this is non-zero, the JobSelector object is asked to decide which
                                    jobs should be removed. If it is zero, all jobs are removed
        @returns    true if all jobs are successfully stopped and removed; false if the timeout period
                    expires while waiting for one or more jobs to stop
    */
    bool removeAllJobs (bool interruptRunningJobs,
                        int timeOutMilliseconds,
                        JobSelector* selectedJobsToRemove = nullptr);

    /** Returns the number of jobs currently running or queued.
    */
    int getNumJobs() const;

    /** Returns one of the jobs in the queue.

        Note that this can be a very volatile list as jobs might be continuously getting shifted
        around in the list, and this method may return nullptr if the index is currently out-of-range.
    */
    ThreadPoolJob* getJob (int index) const;

    /** Returns true if the given job is currently queued or running.

        @see isJobRunning()
    */
    bool contains (const ThreadPoolJob* job) const;

    /** Returns true if the given job is currently being run by a thread.
    */
    bool isJobRunning (const ThreadPoolJob* job) const;

    /** Waits until a job has finished running and has been removed from the pool.

        This will wait until the job is no longer in the pool - i.e. until its
        runJob() method returns ThreadPoolJob::jobHasFinished.

        If the timeout period expires before the job finishes, this will return false;
        it returns true if the job has finished successfully.
    */
    bool waitForJobToFinish (const ThreadPoolJob* job,
                             int timeOutMilliseconds) const;

    /** Returns a list of the names of all the jobs currently running or queued.
        If onlyReturnActiveJobs is true, only the ones currently running are returned.
    */
    StringArray getNamesOfAllJobs (bool onlyReturnActiveJobs) const;

    /** Changes the priority of all the threads.

        This will call Thread::setPriority() for each thread in the pool.
        May return false if for some reason the priority can't be changed.
    */
    bool setThreadPriorities (int newPriority);


private:
    //==============================================================================
    Array <ThreadPoolJob*> jobs;

    class ThreadPoolThread;
    friend class ThreadPoolJob;
    friend class ThreadPoolThread;
    friend struct ContainerDeletePolicy<ThreadPoolThread>;
    OwnedArray<ThreadPoolThread> threads;

    CriticalSection lock;
    WaitableEvent jobFinishedSignal;

    bool runNextJob (ThreadPoolThread&);
    ThreadPoolJob* pickNextJobToRun();
    void addToDeleteList (OwnedArray<ThreadPoolJob>&, ThreadPoolJob*) const;
    void createThreads (int numThreads);
    void stopThreads();

    // Note that this method has changed, and no longer has a parameter to indicate
    // whether the jobs should be deleted - see the new method for details.
    void removeAllJobs (bool, int, bool);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPool)
};


#endif   // JUCE_THREADPOOL_H_INCLUDED
