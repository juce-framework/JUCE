#ifndef ztd_IntrusiveListNode_h__
#define ztd_IntrusiveListNode_h__

ZTD_NAMESPACE_START;

template<typename T> class Stack;
template<typename T> class Queue;
template<typename T> class DubList;
template<typename T> class LockfreeStack;
template<typename T> class LockfreeQueue;
template<typename T> class LockfreeDubList;

/***********************
* 链表元素,可用于各种单链表形式的数据结构(不可用于Lockfree)
***********************/
template<typename T,int content = 0>
class ListNode
{
public:
	friend class Stack<T>;
	friend class Queue<T>;
protected:
	forcedinline ListNode() :m_next(nullptr){};
	forcedinline ~ListNode(){};
private:
	forcedinline T*& next() { return m_next; }
private:
	T* m_next;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ListNode);
};
ZTD_NAMESPACE_END;

#endif // ztd_IntrusiveListNode_h__
