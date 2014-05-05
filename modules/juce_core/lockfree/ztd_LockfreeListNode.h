#ifndef ztd_IntrusiveLockfreeListNode_h__
#define ztd_IntrusiveLockfreeListNode_h__

ZTD_NAMESPACE_START;

NAMESPACE_START(helper);

template<typename T>
class CountedPtr
{
public:
	void IncCounter() { m_counter += 3; }
private:
	T*     m_ptr;
	uintptr_t m_counter;
};

NAMESPACE_END;


template<typename T>
class LockfreeListNode
{
public:
	friend class Stack<T>;
	friend class Queue<T>;
	friend class LockfreeStack<T>;
	friend class LockfreeQueue<T>;
protected:
	LockfreeListNode();
	~LockfreeListNode();
private:
	forcedinline T*& next() { return m_next.m_data; }
private:
	Zatomic<helper::CountedPtr<T>> m_next;
	Zatomic<int> m_state;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockfreeListNode);
};

ZTD_NAMESPACE_END;

#endif // ztd_IntrusiveLockfreeListNode_h__
