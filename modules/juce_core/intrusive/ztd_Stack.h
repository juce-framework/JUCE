#ifndef ztd_Zstack_h__
#define ztd_Zstack_h__

ZTD_NAMESPACE_START;

template<typename T>
class Stack
{
public:
	//* 创建一个空的栈
	Stack();
	//* 析构一个栈,注意! 本函数不做任何事,本栈持有的元素本栈并不负责销毁
	~Stack() = default;
	//* 将另一个栈的内容送入本栈中,行为完全如同从other中pop再push进本栈,复杂度为O(n).
	Stack(Stack<T>& other):Stack(){ *this<<other; }
	//* 将另一个队列的内容送入本栈中,行为完全如同从other中pop再push进本栈,复杂度为O(1).
	Stack(Queue<T>& other):Stack(){ *this<<other; }
	//* 将另一个ZlockfreeStack中的内容送入本栈中,行为完全如同从other中pop再push进本栈,复杂度O(1),优势在于只需要O(1)次的原子操作
	Stack(LockfreeStack<T>& other):Stack(){ *this<<other; }
	//* 将另一个ZlockfreeQueue中的内容送入本栈中,行为完全如同从other中pop再push进本栈,复杂度O(1),优势在于只需要O(1)次的原子操作
	Stack(LockfreeQueue<T>& other):Stack(){ *this<<other; }
	//* 将另一个栈的内容追加到本栈中,行为完全如同从other中pop再push进本栈,复杂度为O(n).
	Stack& operator<<( Stack& other );
	//* 将另一个队列的内容追加到本栈中,行为完全如同从other中pop再push进本栈,复杂度为O(1).
	Stack& operator<<( Queue<T>& other );
	Stack& operator<<( DubList<T>& other );
	//* 将另一个ZlockfreeStack中的内容追加到本栈中,行为完全如同从other中pop再push进本栈,复杂度O(1),优势在于只需要O(1)次的原子操作
	Stack& operator<<( LockfreeStack<T>& other );
	//* 将另一个ZlockfreeQueue中的内容追加到本栈中,行为完全如同从other中pop再push进本栈,复杂度O(1),优势在于只需要O(1)次的原子操作
	Stack& operator<<( LockfreeQueue<T>& other );
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
	friend class Queue<T>;
	friend class LockfreeStack<T>;
	friend class LockfreeQueue<T>;
	T* m_tail;
	ListNode<T> m_dummy;
	Stack& operator=(Stack&)=delete;
	JUCE_LEAK_DETECTOR(Stack);
};

template<typename T>
template<typename Func>
void Stack<T>::forEach(const Func&& func)
{
	for(T* k = m_tail; k != (T*)&m_dummy; k = k->m_next) func(*k);
}

template<typename T>
template<typename Func>
void Stack<T>::forEachFind(const Func&& func)
{
	for(T* k = m_tail; k != (T*)&m_dummy; k = k->m_next) {
		if(!func(*k)) break;
	}
}

template<typename T>
template<typename Func>
T* Stack<T>::forEachPop(const Func&& func)
{
	//static_assert( is_same<result_of<Func>::type,int>::value,"Func must return int" ); GCC.....
	for(T* k = m_tail; k != (T*)&m_dummy;) {
		T*const temp = k->m_next;
		if(func(*k)) break;
		k = temp;
	}
}

template<typename T>
template<typename Func>
void Stack<T>::popEach(const Func&& func)
{
	for(T* k = nullptr; Pop(k); func(k));
}

template<typename T>
void Stack<T>::deleteAllNode()
{
	popEach([](T*const k){ delete k; });
}

//=======================================================================================================

template<typename T>
void Stack<T>::setEmpty()
{
	m_tail = (T*)&m_dummy;
	m_dummy.m_next = (T*)&m_dummy;
}

template<typename T>
bool Stack<T>::isEmpty() const
{
	return m_tail == (T*)&m_dummy;
}

template<typename T>
Stack<T>::Stack()
{
	setEmpty();
}

template<typename T>
void Stack<T>::Push(T* obj)
{
	obj->m_next = m_tail;
	m_tail = obj;
}

template<typename T>
bool Stack<T>::Pop(T*& ptr)
{
	T*const k = m_tail;
	m_tail = k->m_next;
	ptr = k;
	return k != (T*)&m_dummy;
}

ZTD_NAMESPACE_END;

#endif // ztd_Zstack_h__
