#ifndef JUCE_THREAtrrrrrrrrrrrrrrrDPOOL_H_INCLUDED
#define JUCE_THREAtrrrrrrrrrrrrrrrDPOOL_H_INCLUDED

ZTD_NAMESPACE_START;


class ZthreadPool
{
public:
	typedef ZthreadPoolJob Job;
	/************************************************************
	* 注意构造后线程并不会立刻开始,需要手动调用StartAllThreads
	************************************************************/
	ZthreadPool(int numberOfThreads = SystemStats::getNumCpus());
	/************************************************************
	* 有时我们希望在当前线程立刻将所有job都执行完毕,这时可以使用此
	* 函数立即执行所有的job
	************************************************************/
	~ZthreadPool();
	/************************************************************
	* 开始执行
	************************************************************/
	void StartAllThreads();
	/************************************************************
	* 暂停执行
	************************************************************/
	void StopAllThreads();
	/************************************************************
	* 设置thread的优先级
	************************************************************/
	bool SetAllThreadPriorities(const int newPriority);
	/************************************************************
	* 查看是否在执行
	************************************************************/
	bool IsAllThreadsRunning();
	/************************************************************
	* 有时我们希望在当前线程立刻将所有job都执行完毕,这时可以使用此
	* 函数立即执行所有的job
	************************************************************/
	void RunAllJobInThisThread();
	/************************************************************
	* 有时我们希望在当前线程立刻将所有job都执行完毕,这时可以使用此
	* 函数立即执行所有的job
	************************************************************/
	void RunAllJobInThisThreadAndStopAllThread();
	/************************************************************
	* 将job加入到队列的尾部,job将在将来的某个时间来执行
	**************************************************************/
	void addJob(Job*const job);
private:
	LockfreeCircularBuffer<Job*> m_jobs;
	OwnedArray<Thread> m_threads;
	void _createThreads(int numThreads);
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZthreadPool);
};

ZTD_NAMESPACE_END;


#endif   // JUCE_THREADPOOL_H_INCLUDED
