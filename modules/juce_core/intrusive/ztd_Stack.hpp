
ZTD_NAMESPACE_START;

template<typename T>
Stack<T>& Stack<T>::operator<<( Stack& other )
{
	for(T* k = nullptr; other.Pop(k); Push(k));
	return *this;
}

template<typename T>
Stack<T>& Stack<T>::operator<<( Queue<T>& other )
{
	T*const tail=other.m_tail;
	T*const head=tail->m_next;
	other.setEmpty();
	head->m_next=this->m_tail;
	m_tail=tail;
	return *this;
}

template<typename T>
Stack<T>& Stack<T>::operator<<( DubList<T>&  )
{
	return *this;
}

template<typename T>
Stack<T>& Stack<T>::operator<<( LockfreeStack<T>& )
{
	return *this;
}

template<typename T>
Stack<T>& Stack<T>::operator<<( LockfreeQueue<T>& )
{
	return *this;
}

ZTD_NAMESPACE_END;