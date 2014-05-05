#ifndef ztd_AudioBuffer_h__
#define ztd_AudioBuffer_h__

ZTD_NAMESPACE_START;

template<typename T>
class AudioBuffer
{
public:
	forcedinline AudioBuffer():m_data(),m_size(0){};
	forcedinline ~AudioBuffer(){};
	forcedinline void setSize(intc newSize) { if (m_size!=newSize) m_data.malloc(newSize); m_size=newSize; }
	forcedinline operator T*() const { return m_data.getData(); }
	template<typename IntType> forcedinline T* operator+(IntType i) const { return m_data+i; }
	forcedinline intptr_t getRawPtr() const { return (intptr_t)(float*)m_data; }
	forcedinline T* getPtr() const { return m_data.getData(); }
	forcedinline T& operator[](const intc i) { return m_data[i]; }
	forcedinline intc getSize() const { return m_size; }
private:
	AlignedHeapBlock<T> m_data;
	intc m_size;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioBuffer);
};

forcedinline void SIMDmemzero(const AudioBuffer<float>& vec)
{
	SIMDmemzero(vec.getPtr(),vec.getSize());
};

forcedinline void SIMDmemmove(const AudioBuffer<float>& vecY,const AudioBuffer<float>& vecX)
{
    jassert(vecY.getSize()==vecX.getSize());
	SIMDmemmove(vecY.getPtr(),vecX.getPtr(),vecY.getSize());
};

forcedinline void SIMDreverse(const AudioBuffer<float>& vec)
{
	SIMDmemzero(vec.getPtr(),vec.getSize());
};

ZTD_NAMESPACE_END;

#endif // ztd_AudioBuffer_h__
