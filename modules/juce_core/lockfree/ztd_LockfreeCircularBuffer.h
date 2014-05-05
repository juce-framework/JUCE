#ifndef ztd_ZlogfdssssssssssckfreeCgggggggggggggircularBuffer_h__
#define ztd_ZlogfdssssssssssckfreeCgggggggggggggircularBuffer_h__


ZTD_NAMESPACE_START;

/*
环形缓冲区需要3种状态:
空: startPos==writePos
满: startPos==(writePos+1) //因此,我们最多只能往缓冲区里放入cycSize-1个元素,最后一个元素是费的
其他...

非环形缓冲区:size=writePos-startPos;
环形缓冲区: size = (writePos<startPos)?(writePos+cycSize-startPos):(writePos-startPos);
*/
NAMESPACE_START(helper);

template<typename T>
struct LockfreeCircularBufferNode
{
public:
	LockfreeCircularBufferNode() = default;
	LockfreeCircularBufferNode(const LockfreeCircularBufferNode&) = default;
	LockfreeCircularBufferNode& operator=( const LockfreeCircularBufferNode& ) = default;
	~LockfreeCircularBufferNode() = default;
public:
	T m_dataInNode;
	Zatomic<int> m_stateInNode;
};

NAMESPACE_END;

template<typename T>
class LockfreeCircularBuffer :private helper::CircularBufferBase<helper::LockfreeCircularBufferNode<T>>
{
public:
	forcedinline LockfreeCircularBuffer(intc pow2Size=1);
	forcedinline ~LockfreeCircularBuffer();
	template<typename Func> forcedinline bool bound_push(const Func& func);
	template<typename Func> forcedinline void lostable_push(const Func& func);
	template<typename Func> forcedinline void push(const Func& func);
	template<typename Func> forcedinline bool pop(const Func& func);
	forcedinline bool bound_push(const T& obj);
	forcedinline void lostable_push(const T& obj);
	forcedinline void push(const T& obj);
	forcedinline bool pop(T& obj);
	forcedinline bool isEmpty();
	forcedinline bool isFull();
private:
	enum {
		FreeToUse=0, //此处应为0,因为初始化Node数组和_Realloc()时初始时Node的状态必须设定为freeToUse
		HasNode,
		Pushing
	};

	intc m_pow2Size;

	Atomic<intc> m_readPos;
	Atomic<intc> m_writePos;

	ReadWriteLock m_reallocLock;

	forcedinline bool _isEmpty(intc writePos,intc readPos) noexcept;
	forcedinline bool _isFull(intc writePos,intc readPos) const noexcept;

	void _Realloc()
	{
		if(m_reallocLock.EnterWriteAny()==-1) return;
		intc const m=this->realloc(++m_pow2Size,m_readPos.get(),m_writePos.get(),true);
		m_readPos=0;
		m_writePos=m;
		m_reallocLock.ExitWrite();
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LockfreeCircularBuffer);
};



//------------------------------------------------------------------------------
template<typename T>
LockfreeCircularBuffer<T>::LockfreeCircularBuffer(intc pow2Size)
    :helper::CircularBufferBase<helper::LockfreeCircularBufferNode<T>>(pow2Size)
	,m_pow2Size(pow2Size)
	,m_readPos()
	,m_writePos()
	,m_reallocLock()
{
	//在push发生时,我们必须初始化m_data中所有的m_stateInNode为"FreeToUse",只有这样才能
	//保证当push发生时能够正确检测其接下来要读取的节点是FreeToUse的.
//	static_assert(is_default_constructible<helper::ZlockfreeCircularBufferNode<T>>::value,"T must be pod!!!");
}

template<typename T>
LockfreeCircularBuffer<T>::~LockfreeCircularBuffer(){}

template<typename T>
template<typename Func>
bool LockfreeCircularBuffer<T>::bound_push(const Func& func)
{
	const ReadWriteLock::ScopedReadLock scopedLock(m_reallocLock);

	for(;;) {
		const intc readPos=m_readPos.get();
		const intc k=m_writePos.get();
		unlikely_if(_isFull(k,readPos)) return false;
		unlikely_if( ! m_writePos.compareAndSetBool(k+1,k) ) continue;
		int const p = this->getDataInModIndex(k).m_stateInNode.compareAndSetValue(Pushing,FreeToUse);
		jassert( (p==FreeToUse)||(p==Pushing)||(p==HasNode) );
		unlikely_if(p != FreeToUse) { continue; }
		func(this->getDataInModIndex(k).m_dataInNode);
		this->getDataInModIndex(k).m_stateInNode = HasNode;
		break;
	}
	return true;
}

template<typename T>
template<typename Func>
bool LockfreeCircularBuffer<T>::pop(const Func& func)
{
	const ReadWriteLock::ScopedReadLock scopedLock(m_reallocLock);

	for(;;) {
		const intc k = m_readPos.get();
		unlikely_if(_isEmpty(m_writePos.get(),k)) { return false; }
		int const p = this->getDataInModIndex(k).m_stateInNode.get();
		unlikely_if(p == Pushing) { return false; }
		if(p == FreeToUse) { return false; }
		jassert(p==HasNode);
		unlikely_if(!m_readPos.compareAndSetBool(k + 1,k)) continue;
		func(this->getDataInModIndex(k).m_dataInNode);
		this->getDataInModIndex(k).m_stateInNode=FreeToUse;
		break;
	}
	return true;
}

template<typename T>
template<typename Func>
void LockfreeCircularBuffer<T>::lostable_push(const Func& func)
{
	unlikely_while(!bound_push(func)) {
		pop([](){});
	}
}

template<typename T>
template<typename Func>
void LockfreeCircularBuffer<T>::push(const Func& func)
{
	unlikely_while ( ! bound_push(func) ) {
		_Realloc();
	}
}

template<typename T>
forcedinline bool LockfreeCircularBuffer<T>::bound_push(const T& obj) { return bound_push([&](T& k){ k = obj; }); }

template<typename T>
forcedinline void LockfreeCircularBuffer<T>::lostable_push(const T& obj) { return lostable_push([&](T& k){ k=obj; }); }

template<typename T>
forcedinline void LockfreeCircularBuffer<T>::push(const T& obj) { return push([&](T& k){ k = obj; }); }

template<typename T>
forcedinline bool LockfreeCircularBuffer<T>::pop(T& obj) { return pop([&](const T& k){ obj = k; }); }

template<typename T>
bool LockfreeCircularBuffer<T>::isFull()
{
	const ReadWriteLock::ScopedReadLock scopedLock(m_reallocLock);
	return _isFull(m_writePos.get(),m_readPos.get());
}

template<typename T>
bool LockfreeCircularBuffer<T>::isEmpty()
{
	const ReadWriteLock::ScopedReadLock scopedLock(m_reallocLock);
	return _isEmpty(m_writePos.get(),m_readPos.get());
}

template<typename T>
forcedinline bool LockfreeCircularBuffer<T>::_isFull(intc writePos,intc startPos) const noexcept
{
	const intc k=this->mod(writePos+1);
	startPos = this->mod(startPos);
	return startPos==k;
}

template<typename T>
forcedinline bool LockfreeCircularBuffer<T>::_isEmpty(intc writePos,intc startPos) noexcept
{
	writePos = this->mod(writePos);
	startPos = this->mod(startPos);
	return startPos == writePos;
}





ZTD_NAMESPACE_END;



#endif // ztd_ZlockfreeCircularBuffer_h__
