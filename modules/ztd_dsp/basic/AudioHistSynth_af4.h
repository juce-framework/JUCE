#ifndef ztd_SIMDaudioStream_h__
#define ztd_SIMDaudioStream_h__

ZTD_NAMESPACE_START;

class SIMDAudioHistorySynth
{
protected:
	SIMDAudioHistorySynth():m_bigBuffer(),m_minHoldSize(0),m_pushStart(0){}
	~SIMDAudioHistorySynth(){}
	forcedinline void render_input_implement(float*const aliPtr,const intc simdBlkSize) { SIMDmemzero(aliPtr,simdBlkSize); }
	forcedinline void render_process_implement(float*const aliPtr,const intc simdBlkSize) {}
public:
	inline void setSize(intc minHoldSize,intc maxBuffSize)
	{
		m_minHoldSize = minHoldSize;
		m_bigBuffer.setSize(minHoldSize+maxBuffSize);
		SIMDmemzero(m_bigBuffer);
		m_pushStart = m_bigBuffer+m_minHoldSize;
	}
	forcedinline intc getMaxPushSize() const { return m_bigBuffer.getSize()-m_minHoldSize; }
	template<typename InputFunc,typename ProcessFunc>
	inline void render(const intc blkSize,const InputFunc& inputFunc,const ProcessFunc& processFunc)
	{
		jassert(blkSize<=(m_bigBuffer.getSize()-m_minHoldSize));
		checkSizeSIMD(blkSize,8);
		checkPtrSIMD(ptr,16);
		inputFunc(m_pushStart,blkSize);
		processFunc(m_bigBuffer,blkSize);
		SIMDmemmove(m_bigBuffer,m_bigBuffer+blkSize,m_minHoldSize);
	}
private:
	AudioBuffer<float> m_bigBuffer; //比minHoldsize更大的缓存,整数倍,用来解决不需每次push都向前拷贝旧值的问题
	intc m_minHoldSize; //最小持有这么长的缓存
	float* m_pushStart;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SIMDAudioHistorySynth);
};

ZTD_NAMESPACE_END;

#endif // ztd_SIMDaudioStream_h__
