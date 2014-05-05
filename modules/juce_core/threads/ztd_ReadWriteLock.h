#ifndef ztd_ZlogfdssssssssssckfreeCircularBuffer_h__
#define ztd_ZlogfdssssssssssckfreeCircularBuffer_h__

ZTD_NAMESPACE_START;

/************************************************************************/
/* 读写锁,同一时间只能有1个writer,但可以同时有多个reader来读.
*/
/************************************************************************/
class ReadWriteLock
{
public:
	class ScopedReadLock
	{
	public:
		forcedinline ScopedReadLock(ReadWriteLock& readWriteLock)
			:m_readWriteLock(readWriteLock)
		{
			m_readWriteLock.EnterRead();
		}
		forcedinline ~ScopedReadLock()
		{
			m_readWriteLock.ExitRead();	
		}
	private:
		ReadWriteLock& m_readWriteLock;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopedReadLock);
	};
	class ScopedWriteLock
	{
	public:
		forcedinline ScopedWriteLock(ReadWriteLock& readWriteLock)
			:m_readWriteLock(readWriteLock)
		{
			m_readWriteLock.EnterWrite();
		}
		forcedinline ~ScopedWriteLock()
		{
			m_readWriteLock.ExitWrite();
		}
	private:
		ReadWriteLock& m_readWriteLock;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopedWriteLock);
	};
public:
	forcedinline ReadWriteLock():m_lockFlag(0){}
	forcedinline ~ReadWriteLock(){};
	inline void EnterRead()
	{
		int k;
		for(int i = 0;; ++i) {
			unlikely_if(i == 40) { i = 0; Thread::sleep(100); }
			k = m_lockFlag.get();
			jassert(k >= -1);
			unlikely_if(k == -1) continue;
			likely_if(m_lockFlag.compareAndSetBool(k,k + 1)) break;
		}
	}
	inline void ExitRead()
	{
		--m_lockFlag;
	}
	inline int EnterWriteAny()
	{
		int k;
		for(int i = 0;; ++i) {
			unlikely_if(i == 40) { i = 0; Thread::sleep(100); }
			k = m_lockFlag.compareAndSetValue(0,-1);
			likely_if( (k==-1)||(k==0) ) break;
		}
		return k;
	}
	inline void EnterWrite()
	{
		int k;
		for(int i = 0;; ++i) {
			unlikely_if(i == 40) { i = 0; Thread::sleep(100); }
			k = m_lockFlag.compareAndSetValue(0,-1);
			likely_if( k==0 ) break;
		}
	}
	/**************************************************** 
	* 当EnterWrite时,一定要注意,如果EnterWriteAny返回-1,
	  说明已经有其他线程进入Write,此时我们不能调用ExitWrite因为当前线程并没有持有锁!
	******************************************************/
	inline void ExitWrite()
	{
#		if JUCE_DEBUG
			const bool temp2 = m_lockFlag.compareAndSetBool(-1,0);
			jassert(temp2); //因为只有一个线程能修改m_reallocLock,所以此处必然成功
#		else
			m_lockFlag = 0;
#		endif
	}
private:
	Zatomic<int> m_lockFlag;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReadWriteLock);
};

ZTD_NAMESPACE_END;

#endif // ztd_ZlockfreeCircularBuffer_h__
