#ifndef ZObjPofdsfdseeee344ol_h__
#define ZObjPofdsfdseeee344ol_h__

ZTD_NAMESPACE_START;

template<typename T,bool multiThread=true,int allocPreSize=4*1024*16/sizeof(T)>
class ObjPool
{
public:
	typedef typename type_if<multiThread,LockfreeCircularBuffer<T*>,CircularBuffer<T*>>::type QueueType;
public:
	forcedinline ObjPool(int preAllocBlkNum=1);
	//* 析构函数,会删除这个pool中持有的内存,在执行这个函数之后,从pool中放出的元素则不能再使用
	forcedinline ~ObjPool();
	forcedinline T* Pop();
	forcedinline void Push(T* k);
	//* 手动创建一些新元素,并加入池中
	forcedinline void CreateSome(int allocBlkNum = 1);
private:
	class ObjBlock
	{
	public:
		ObjBlock();
		~ObjBlock();
		forcedinline T* operator[](const int i);
	private:
		T m_elements[allocPreSize];
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ObjBlock);
	};
private:
	typedef typename type_if<multiThread,LockfreeCircularBuffer<ObjBlock*>,CircularBuffer<ObjBlock*>>::type ObjGrpBuffer;
private:
	ObjGrpBuffer m_objBlks;
	QueueType    m_reuseQueue;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ObjPool);
};


//==================================================================================================

template<typename T,bool multiThread,int allocPreSize>
ObjPool<T,multiThread,allocPreSize>::ObjPool(int preAllocBlkNum) 
	:m_objBlks(14)
	,m_reuseQueue(14)
{
	CreateSome(preAllocBlkNum);
}

template<typename T,bool multiThread,int allocPreSize>
ObjPool<T,multiThread,allocPreSize>::~ObjPool()
{
	ObjBlock* k;
	likely_while(m_objBlks.pop(k)) { delete k; }
}

template<typename T,bool multiThread,int allocPreSize>
T* ObjPool<T,multiThread,allocPreSize>::Pop()
{
	T* k;
	unlikely_while(!m_reuseQueue.pop(k)) { 
		CreateSome(); 
	}
	return k;
}

template<typename T,bool multiThread,int allocPreSize>
void ObjPool<T,multiThread,allocPreSize>::Push(T* k)
{
	m_reuseQueue.push(k);
}

template<typename T,bool multiThread,int allocPreSize>
void ObjPool<T,multiThread,allocPreSize>::CreateSome(int allocBlkNum)
{
	for(int i = 0; i < allocBlkNum; i++) {
		ObjBlock*const m = new ObjBlock;
		m_objBlks.push(m);
		for(int i = 0; i < allocPreSize; ++i) {
			m_reuseQueue.push( (*m)[i] );
		}
	}
}

template<typename T,bool multiThread,int allocPreSize>
T* ObjPool<T,multiThread,allocPreSize>::ObjBlock::operator[](const int i)
{
	return &m_elements[i];
}

template<typename T,bool multiThread,int allocPreSize>
ObjPool<T,multiThread,allocPreSize>::ObjBlock::ObjBlock()
{}


template<typename T,bool multiThread,int allocPreSize>
ObjPool<T,multiThread,allocPreSize>::ObjBlock::~ObjBlock()
{}

ZTD_NAMESPACE_END;


#endif // ZObjPool_h__
