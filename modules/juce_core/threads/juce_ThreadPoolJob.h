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

class ThreadPool;

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

    @tags{Core}
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
                                     removed from the pool */

        jobNeedsRunningAgain    /**< indicates that the job would like to be called
                                     again when a thread is free */
    };

    /** Performs the actual work that this job needs to do.

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

    /** Add a listener to this thread job which will receive a callback when
        signalJobShouldExit was called on this thread job.

        @see signalJobShouldExit, removeListener
    */
    void addListener (Thread::Listener*);

    /** Removes a listener added with addListener. */
    void removeListener (Thread::Listener*);

    //==============================================================================
    /** If the calling thread is being invoked inside a runJob() method, this will
        return the ThreadPoolJob that it belongs to.
    */
    static ThreadPoolJob* getCurrentThreadPoolJob();

    //==============================================================================
private:
    friend class ThreadPool;
    String jobName;
    ThreadPool* pool = nullptr;
    std::atomic<bool> shouldStop { false }, isActive { false }, shouldBeDeleted { false };
    ThreadSafeListenerList<Thread::Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreadPoolJob)
};

} // namespace juce
