#ifndef ztd_AudioBuffer_h__gggvccccccccc
#define ztd_AudioBuffer_h__gggvccccccccc

ZTD_NAMESPACE_START;

template<typename T>
class MultiAudioBuffer
{
public:
	forcedinline MultiAudioBuffer():m_data(),m_size(0){};
	forcedinline ~MultiAudioBuffer(){};
	forcedinline void setSize(intc newSize,intc channel) { if ( (m_size*m_channelNum)!= (newSize*channel)) m_data.malloc(newSize*channel); m_size=newSize; m_channelNum=channel; }
	forcedinline operator T*() const { return m_data.getData(); }
	template<typename IntType> forcedinline T* operator+(IntType i) const { return m_data+i*m_size; }
	forcedinline intptr_t getRawPtr(intc i) const { return (intptr_t)(m_data+i); }
	forcedinline T* getPtr(intc i) const { return m_data+i*m_size; }
	forcedinline T& operator[](const intc i) { return m_data+i*m_size; }
	forcedinline intc getSizePreCh() const { return m_size; }
	forcedinline intc getChNum() const { return m_channelNum; }
	forcedinline intc getRawSize() const { return m_channelNum*m_size; }
private:
	AlignedHeapBlock<T> m_data;
	intc m_size;
	intc m_channelNum;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiAudioBuffer);
};

forcedinline void SIMDmemzero(const MultiAudioBuffer<float>& vec)
{
	SIMDmemzero(vec[0],vec.getRawSize());
};

forcedinline void SIMDmemmove(const MultiAudioBuffer<float>& vecY,const MultiAudioBuffer<float>& vecX)
{
    jassert(vecY.getSizePreCh()==vecX.getSizePreCh());
	jassert(vecY.getChNum()==vecX.getChNum());
	for(intc i = 0; i < vec.getChNum(); ++i) {
		SIMDmemmove(vecY[i],vecX[i],vecX.getSizePreCh());
	}
};

forcedinline void SIMDreverse(const MultiAudioBuffer<float>& vec)
{
	for(intc i = 0; i < vec.getChNum(); ++i) {
		SIMDreverse(vec[i],vec.getSizePreCh());
	}
};

ZTD_NAMESPACE_END;

#endif // ztd_AudioBuffer_h__
