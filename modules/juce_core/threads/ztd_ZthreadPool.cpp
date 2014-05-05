

ZTD_NAMESPACE_START;

class ZthreadPoolThread: public Thread
{
public:
	typedef ZthreadPool::Job Job;
public:
	ZthreadPoolThread(LockfreeCircularBuffer<Job*>& jobQueue)
		:Thread("ZthreadPoolThread")
		,m_jobQueue(jobQueue)
	{}
	virtual ~ZthreadPoolThread(){}
	void run() override final
	{
		for(;unlikely(!threadShouldExit());wait(30)) {
			runJobsNow(m_jobQueue,30);
		}
	}
public:
	static void runJobsNow(LockfreeCircularBuffer<Job*>& jobQueue,int waitTime=0)
	{
		for(Job* k = nullptr; jobQueue.pop(k);) {
			unlikely_while( ! k->m_jobState.compareAndSetBool(Job::jobIsInQueue,Job::jobIsRunning) );
			auto result = k->runJob();
			if(result == Job::jobNeedsRunningAgain) {
				unlikely_while( ! k->m_jobState.compareAndSetBool(Job::jobIsRunning,Job::jobIsInQueue) );
				jobQueue.push(k);
				if(waitTime>0) Thread::sleep(waitTime);
			} else {
				unlikely_while( ! k->m_jobState.compareAndSetBool(Job::jobIsRunning,Job::jobNotInQueue) );
			}
		}
	}
private:
	LockfreeCircularBuffer<Job*>& m_jobQueue;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZthreadPoolThread);
};


ZthreadPool::ZthreadPool(int numberOfThreads)
	:m_jobs(14)
{
	jassert(numberOfThreads > 0);
	numberOfThreads = jmax(numberOfThreads,1);
	_createThreads(numberOfThreads);
}

void ZthreadPool::_createThreads(int numThreads)
{
	for(int i = jmax(1,numThreads); --i >= 0;) {
		m_threads.add(new ZthreadPoolThread(m_jobs));
	}
}

void ZthreadPool::StartAllThreads()
{
	for(int i = m_threads.size(); --i >= 0;) {
		checkInRange(0,m_threads.size(),i);
		m_threads.getUnchecked(i)->startThread();
	}
}

void ZthreadPool::StopAllThreads()
{
	for(int i = m_threads.size(); --i >= 0;) {
		checkInRange(0,m_threads.size(),i);
		m_threads.getUnchecked(i)->stopThread(500);
	}
}

void ZthreadPool::RunAllJobInThisThread()
{
	ZthreadPoolThread::runJobsNow(m_jobs);
}

bool ZthreadPool::IsAllThreadsRunning()
{
	for(int i = 0; i < m_threads.size(); ++i) {
		checkInRange(0,m_threads.size(),i);
		if(m_threads.getUnchecked(i)->isThreadRunning()) return true;
	}
	return false;
}

bool ZthreadPool::SetAllThreadPriorities(const int newPriority)
{
	bool ok = true;
	for(int i = m_threads.size(); --i >= 0;) {
		checkInRange(0,m_threads.size(),i);
		ok = ok&&m_threads.getUnchecked(i)->setPriority(newPriority);
	}
	return ok;
}

void ZthreadPool::addJob(Job* job)
{
	jassert(job != nullptr);
	unlikely_while(job->m_jobState.compareAndSetBool(Job::jobNotInQueue,Job::jobIsInQueue));
	m_jobs.push(job);
}

ZthreadPool::~ZthreadPool()
{
	RunAllJobInThisThreadAndStopAllThread();
}

void ZthreadPool::RunAllJobInThisThreadAndStopAllThread()
{
	RunAllJobInThisThread();
	StopAllThreads();
	jassert(m_jobs.isEmpty());
}


ZTD_NAMESPACE_END;