#ifndef JUCE_HEAPBLOCK_H_INCLUDEDccaaaaaaaaaaaaaaa
#define JUCE_HEAPBLOCK_H_INCLUDEDccaaaaaaaaaaaaaaa

ZTD_NAMESPACE_START;

/*************************************************************************
* 动态分配的AlignedHeapBlock,可调整内存对齐尺寸,默认为64
*************************************************************************/
template <typename T>
class AlignedHeapBlock
{
public:
	forcedinline AlignedHeapBlock() noexcept
		: m_data(nullptr)
	{
		//static_assert( IS_TRIVIAL(T),"T must be trivial,so it could used by AlignedHeapBlock" );
	}
	forcedinline explicit AlignedHeapBlock(const size_t numElements,const size_t aliSize = 64,const bool setToZero = false) noexcept
		: AlignedHeapBlock()
	{
		if(setToZero) calloc(numElements,aliSize);
		else malloc(numElements,aliSize);
	}
	forcedinline ~AlignedHeapBlock()
	{
		free();
	}
	forcedinline operator T*() const noexcept{ return m_data; }
	forcedinline T* getData() const noexcept{ return m_data; }
	template <typename IndexType> forcedinline T& operator[] (IndexType index) const noexcept{ return m_data[index]; };
	template <typename IndexType> forcedinline T* operator+ (IndexType index) const noexcept{ return m_data+index; };
	forcedinline void malloc(const size_t newNumElements,const size_t aliSize = 64)
	{
		aligned_free(m_data);
		m_data = static_cast <T*> ( aligned_malloc(newNumElements * sizeof ( T ),aliSize) );
		if(m_data == nullptr) throw std::bad_alloc();
		call_if<IS_TRIVIAL(T)>::call([&](){new(m_data) T[newNumElements];},[](){});
	}
	forcedinline void calloc(const size_t newNumElements,const size_t aliSize = 64)
	{
		aligned_free(m_data);
		m_data = static_cast <T*> ( aligned_malloc(newNumElements * sizeof ( T ),aliSize) );
		if(m_data == nullptr) throw std::bad_alloc();
		clear(newNumElements);
		call_if<IS_TRIVIAL(T)>::call([&](){new(m_data) T[newNumElements];},[](){});
	}
	forcedinline void free()
	{
		aligned_free(m_data);
		m_data = nullptr;
	}
	forcedinline void clear(size_t numElements) noexcept{ zeromem(m_data,sizeof (T)* numElements); }
	forcedinline void swapWith(AlignedHeapBlock<T>& other) noexcept
	{
		std::swap(m_data,other.m_data);
	}
private:
	//==============================================================================
	T* m_data;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AlignedHeapBlock);
};

ZTD_NAMESPACE_END;


#endif   // JUCE_HEAPBLOCK_H_INCLUDED
