#ifndef ztd_IntrusiveDubListNode_h__
#define ztd_IntrusiveDubListNode_h__

ZTD_NAMESPACE_START;

/***********************
* 链表元素,可用于各种单链表形式的数据结构(不可用于Lockfree)
***********************/
template<typename T>
class DubListNode
{
public:
	friend class Stack<T>;
	friend class Queue<T>;
	friend class DubList<T>;
public:
	forcedinline bool isInchain() const { return m_prev == this; }
	forcedinline void setUnchain()
	{
		m_prev->m_next = m_next;
		m_next->m_prev = m_prev;
	}
protected:
	forcedinline DubListNode() :m_prev(this),m_next(this){};
	forcedinline ~DubListNode(){};
private:
	forcedinline T*& next() { return m_next; }
	forcedinline T*& prev() { return m_prev; }
private:
	T* m_prev;
	T* m_next;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DubListNode);
};


ZTD_NAMESPACE_END;

#endif // ztd_IntrusiveDubListNode_h__
