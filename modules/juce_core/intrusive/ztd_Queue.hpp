
ZTD_NAMESPACE_START;

template<typename T>
Queue<T>& Queue<T>::operator<<( Stack<T>& other )
{
	for(T* k = nullptr; other.Pop(k); Push(k));
	return *this;
}

template<typename T>
Queue<T>& Queue<T>::operator<<( Queue<T>& other )
{
	T*const tail = other.m_tail;
	T*const head = other.m_tail->m_next;
	other.setEmpty();
	tail->m_next=m_tail->m_next;
	m_tail->m_next=head;
	return *this;
}

template<typename T>
Queue<T>& Queue<T>::operator<<( DubList<T>& other )
{
	return *this;
}

template<typename T>
Queue<T>& Queue<T>::operator<<( LockfreeStack<T>& other )
{
	return *this;
}

template<typename T>
Queue<T>& Queue<T>::operator<<( LockfreeQueue<T>& other )
{
	return *this;
}

template<typename T>
Queue<T>& Queue<T>::operator<<( LockfreeDubList<T>& other )
{
	return *this;
}

ZTD_NAMESPACE_END;