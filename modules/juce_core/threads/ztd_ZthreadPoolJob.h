#ifndef ztd_ZthreadPoolJob_h__
#define ztd_ZthreadPoolJob_h__

ZTD_NAMESPACE_START;

class ZthreadPoolJob
{
public:
	enum RunResult {
		jobHasFinished = 0,
		jobNeedsRunningAgain
	};
	enum JobState {
		jobNotInQueue =0,
		jobIsInQueue ,
		jobIsRunning
	};
public:
	forcedinline ZthreadPoolJob()
		:m_jobState(jobNotInQueue)
	{};
	/**********************************************************************
	*析构函数,析构函数必须在job从ZthreadPool中弹出后才结束,因此在job析构之前,
	*一定要务必保证它已经从ZthreadPool中被弹出
	**********************************************************************/
	inline virtual ~ZthreadPoolJob()
	{
		jassert(m_jobState.get()==jobNotInQueue);
	}
	/*********************************************************************
	*将job放入ZthreadPool中后,便会按照顺序一个一个地执行runJob()方法,当方法
	*返回jobHasFinished时,ZthreadPool会将job从队列中排除(注意!不删除!),当
	*返回jobNeedsRunningAgain时,ZthreadPool会将其重新放入到job队列尾部等待下次执行
	*********************************************************************/
	virtual RunResult runJob() = 0;
	/********************************************************************
	*返回job当前的状态
	********************************************************************/
	forcedinline JobState isJobInQueue() noexcept { return m_jobState.get(); };
private:
	friend class ZthreadPool;
	friend class ZthreadPoolThread;
	Zatomic<JobState> m_jobState;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZthreadPoolJob);
};

ZTD_NAMESPACE_END;

#endif // ztd_ZthreadPoolJob_h__
