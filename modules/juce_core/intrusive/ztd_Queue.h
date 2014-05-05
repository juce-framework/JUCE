#ifndef ztd_Zqueue_h__
#define ztd_Zqueue_h__

ZTD_NAMESPACE_START;


template<typename T>
class Queue
{
public:
	//* 创建一个空的队列
	Queue();
	//* 析构一个队列,注意! 本函数不做任何事,本栈持有的元素本栈并不负责销毁
	~Queue()=default;
	//* 将另一个栈的内容添加到本队列中,行为完全如同从other中pop再push进本队列,复杂度为O(n).
	Queue(Stack<T>& other):Queue(){ *this<<other; }
	//* 将另一个队列的内容添加到本队列中,行为完全如同从other中pop再push进本队列,复杂度为O(1).
	Queue(Queue& other):Queue(){ *this<<other; }
	//* 将另一个ZlockfreeStack中的添加追加到本队列中,行为完全如同从other中pop再push进本队列,复杂度O(1),优势在于只需要O(1)次的原子操作
	Queue(LockfreeStack<T>& other):Queue(){ *this<<other; }
	//* 将另一个ZlockfreeQueue中的添加追加到本队列中,行为完全如同从other中pop再push进本队列,复杂度O(1),优势在于只需要O(1)次的原子操作
	Queue(LockfreeQueue<T>& other):Queue(){ *this<<other; }
	//* 将另一个栈的内容追加到本队列中,行为完全如同从other中pop再push进本队列,复杂度为O(n).
	Queue& operator<<( Stack<T>& other );
	//* 将另一个队列的内容追加到本队列中,行为完全如同从other中pop再push进本队列,复杂度为O(1).
	Queue& operator<<( Queue<T>& other );
	Queue& operator<<( DubList<T>& other );
	//* 将另一个ZlockfreeStack中的内容追加到本队列中,行为完全如同从other中pop再push进本队列,复杂度O(1),优势在于只需要O(1)次的原子操作
	Queue& operator<<( LockfreeStack<T>& other );
	//* 将另一个ZlockfreeQueue中的内容追加到本队列中,行为完全如同从other中pop再push进本队列,复杂度O(1),优势在于只需要O(1)次的原子操作
	Queue& operator<<( LockfreeQueue<T>& other );
	Queue& operator<<( LockfreeDubList<T>& other );
	//* 将一个节点push入当前栈
	void Push(T* obj);
	//* 从栈中pop一个节点,如果成功返回true,ptr为pop的那个节点的指针,失败返回false,同时ptr的值为未定义
	bool Pop(T*& ptr);
	//* 检测栈是否为空
	bool isEmpty() const;
	//* 将栈设为空,如果栈持有节点的生存权,请不要这样做
	void setEmpty();
	//* 遍历栈的所有节点,做一些事情,但不能删除节点!! 删除节点请使用popEach或者forEachPop
	template<typename Func> void forEach(const Func&& func);
	//* 遍历栈的所有节点,在第一个查找成功(func返回1)时,不再继续查找
	template<typename Func> void forEachFind(const Func&& func);
	//* 遍历节点,在第一个查找成功时(func返回1)时,将该节点从栈中pop,(但栈不受影响)继续查找,返回2则停止查找,返回0则继续查找而不pop当前节点
	template<typename Func> T* forEachPop(const Func&& func);
	//* 遍历节点,func负责处理所有节点,请小心如果栈持有这些节点的生存权,pop的时候要小心处理
	template<typename Func> void popEach(const Func&& func);
	//* 遍历节点,在返回false时,停止pop
	template<typename Func> T* popEachInRange(const Func&& func);
	//* 删除所有节点,非常危险,使用者必须确定该栈中的节点全都是可以删除的,而不是来自objCache或者其他容器
	void deleteAllNode();
private:
	friend class Stack<T>;
	friend class DubList<T>;
	friend class LockfreeStack<T>;
	friend class LockfreeQueue<T>;
	ListNode<T> m_head;
	T* m_tail;
	Queue& operator=(Queue&)=delete;
	JUCE_LEAK_DETECTOR(Queue);
};

//=======================================================================================================

template<typename T>
void Queue<T>::deleteAllNode() 
{
	popEach([](T*const k){ 
		delete k; 
	}); 
}



template<typename T>
void Queue<T>::setEmpty()
{
	m_head.next() = (T*)&m_head;
	m_tail = (T*)&m_head;
}

template<typename T>
bool Queue<T>::isEmpty() const
{
	return m_tail == (T*)&m_head;
}

template<typename T>
Queue<T>::Queue()
{
	setEmpty();
}

template<typename T>
void Queue<T>::Push(T* obj)
{
	obj->m_next = m_tail->m_next; //head
	m_tail->m_next = obj;
	m_tail = obj;
}

template<typename T>
bool Queue<T>::Pop(T*& ptr)
{
	T*const head = m_tail->m_next;
	m_tail->m_next = head->m_next;
	ptr = head;
	return (T*)&m_head != head;
}

ZTD_NAMESPACE_END;

#endif // ztd_Zqueue_h__
