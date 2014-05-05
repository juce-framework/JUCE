#ifndef ztd_IntrusiveDubLiggggggggggggggggstNode_h__
#define ztd_IntrusiveDubLiggggggggggggggggstNode_h__

ZTD_NAMESPACE_START;

template<typename T>
class LockfreeDubListNode
{
public:
	friend class Stack<T>;
	friend class Queue<T>;
	friend class DubList<T>;
	friend class LockfreeStack<T>;
	friend class LockfreeQueue<T>;
	friend class LockfreeDubList<T>;
protected:
	LockfreeDubListNode(){};
	~LockfreeDubListNode(){};
private:
	forcedinline T*& next() { return m_next.m_data; }
	forcedinline T*& prev() { return m_next.m_data; }
private:
	Zatomic<helper::CountedPtr<T>> m_prev;
	Zatomic<helper::CountedPtr<T>> m_next;
	Zatomic<int> m_state;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockfreeDubListNode);
};

ZTD_NAMESPACE_END;

#endif // ztd_IntrusiveDubListNode_h__
