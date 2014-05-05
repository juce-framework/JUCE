#ifndef ztd_CircularBffffffffffffffffffffffuffer_h__dsadsaaaaaaa
#define ztd_CircularBffffffffffffffffffffffuffer_h__dsadsaaaaaaa


ZTD_NAMESPACE_START;

NAMESPACE_START(helper);

template<typename T>
class CircularBufferBase
{
protected:
	inline CircularBufferBase(const int pow2Size = 12,const bool setToZero = true) noexcept
		: m_sizeDec(( 1u << pow2Size ) - 1)
		, m_data(( 1u << pow2Size ),64,setToZero)
	{
		jassert(pow2Size>0);
		checkPowerOfTwo(m_sizeDec + 1);
		static_assert(IS_TRIVIAL(T),"T must be pod!");
	}
	inline ~CircularBufferBase(){}
	forcedinline intc mod(intc i) const noexcept{ return i&m_sizeDec; }
	forcedinline intc getUsedSize(intc writeStart,intc readStart) const
	{
			const intc temp2 = this->mod(writeStart) - this->mod(readStart);
			return this->mod(temp2);
	}
	forcedinline T& getDataInModIndex(intc index) noexcept
	{
		checkPowerOfTwo(m_sizeDec + 1);
		return m_data[index&m_sizeDec];
	}
public:
	forcedinline T& operator[] (intc index) const noexcept
	{
		checkPowerOfTwo(m_sizeDec + 1);
		return m_data[index&m_sizeDec];
	}
	forcedinline void clear(size_t numElements) noexcept{ m_data.clear(numElements); }
	forcedinline intc realloc(const intc newPow2Size,intc startPos,intc writePos,bool const setToZero = true)
	{
		const intc newSize = ( 1u << newPow2Size );
		jassert(newSize > ( m_sizeDec + 1 ));

		intc const oldSize = getUsedSize(writePos,startPos);
		jassert(oldSize <= m_sizeDec);

		AlignedHeapBlock<T> temp(newSize,64,setToZero);
		for(intc i = 0; i < oldSize; ++i) {
			temp[i] = m_data[( i + startPos )&m_sizeDec];
		}
		m_data.swapWith(temp);
		m_sizeDec = newSize - 1;
		checkPowerOfTwo(m_sizeDec + 1);
		return oldSize;
	}
private:
	intc m_sizeDec;
	AlignedHeapBlock<T> m_data;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularBufferBase);
};

NAMESPACE_END;


template<typename T>
class CircularBuffer :public helper::CircularBufferBase<T>
{
public:
	forcedinline CircularBuffer(const int pow2Size=12) noexcept
		:helper::CircularBufferBase<T>(pow2Size)
		,m_readStart(0)
		,m_writeStart(0)
		,m_pow2Size(pow2Size)
	{}

	forcedinline ~CircularBuffer(){}

	template<typename Func> forcedinline bool bound_push(const Func& func)
	{
		const bool k= ! isFull();
		likely_if(k) func( (*this)[m_writeStart++] );
		return k;
	}
	forcedinline bool bound_push(const T& obj)
	{
		return bound_push([&](T& k){ k=obj; });
	}

	template<typename Func> forcedinline void push(const Func& func)
	{
		while( unlikely(!bound_push(func)) ) {
			this->realloc(++m_pow2Size,m_readStart,m_writeStart);
		}
	}

	forcedinline void push(const T& obj)
	{
		return push([&](T& k){ k = obj; });
	}

	template<typename Func> forcedinline bool pop(const Func& func)
	{
		const bool k = !isEmpty();
		likely_if(k) func(( *this )[m_readStart++]);
		return k;
	}
	forcedinline bool pop(T& obj)
	{
		return pop([&](T& k){ obj=k; });
	}
	forcedinline bool isEmpty() const { return this->mod(m_readStart)==this->mod(m_writeStart); }
	forcedinline bool isFull() const { return this->mod(m_readStart)==this->mod(m_writeStart+1); }
	forcedinline intc getUsedSize() const
	{
		return getUsedSize(m_writeStart,m_readStart);
	}
private:
	intc m_readStart;
	intc m_writeStart;
	intc m_pow2Size;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CircularBuffer);
};

ZTD_NAMESPACE_END;

#endif // ztd_CircularBuffer_h__
