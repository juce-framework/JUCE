#ifndef SIMDAudioProcessor_h__
#define SIMDAudioProcessor_h__

ZTD_NAMESPACE_START;

/**************************************************************************
* AlignAudioHistorySynth,可记录一段音频的历史,在render时,render可使用任意的
  尺寸,任意的output指针
***********************************************************************************/
template<intc minBlkSize>
class MakeAlignedAudioHistorySynth
{
public:
	forcedinline AlignAudioSource();
	forcedinline ~AlignAudioSource(){};
	forcedinline void ResetAndClean(const intc minHoldBlkNum,const intc maxBlkNum);
	template<typename InputFunc,typename ProcessFunc>
	inline void render(const intc blkSize,const InputFunc& inputFunc,const ProcessFunc& processFunc);
private:
	intc m_minHoldBlkSize;
	intc m_lastProcedPos;
	intc m_leftUnproedNum;
	AudioBuffer m_buffer;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MakeAlignedAudioHistorySynth);
};

template<intc minBlkSize,typename ChildClass>
void MakeAlignedAudioHistorySynth<minBlkSize,ChildClass>::ResetAndClean(const intc minHoldBlkNum,const intc maxBlkNum)
{
	jassert(minHoldBlkNum<maxBlkNum);
	m_buffer.setSize(maxBlkNum*minBlkSize);

	m_minHoldBlkSize=minHoldBlkNum*minBlkSize;
	m_lastProcedPos = m_buffer.getSize();
}

template<intc minBlkSize,typename ChildClass>
MakeAlignedAudioHistorySynth<minBlkSize,ChildClass>::MakeAlignedAudioHistorySynth()
	:m_minHoldBlkSize(0)
	,m_lastProcedPos(0)
	,m_buffer()
{
	static_assert(isPowerOfTwo(minBlkSize),"...");
}


template<intc minBlkSize,typename ChildClass>
void MakeAlignedAudioHistorySynth<minBlkSize,ChildClass>::render(intc blockSize)
{
	jassert(blockSize<=m_buffer.getSize());

	Zmemmove( m_buffer , m_buffer + m_lastProcedPos - m_minHoldBlkSize , m_minHoldBlkSize + m_leftUnproedNum );
	
	intc aliBlkSize=floorPowTwo(blockSize+(minBlkSize-1),minBlkSize);
	
	ChildClass::render_implement(m_buffer.getPtr(),aliBlkSize);

	m_lastProcedPos=blockSize;
	m_leftUnproedNum=(aliBlkSize-blockSize);
}


ZTD_NAMESPACE_END;

#endif // SIMDAudioProcessor_h__
