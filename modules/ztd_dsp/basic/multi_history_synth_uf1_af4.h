#ifndef SIMDAudioProcessor_h__86555555555555555
#define SIMDAudioProcessor_h__86555555555555555

JUCE_NAMESPACE_START;

//安全的SIMD stream,无论push什么尺寸的信号,都能在Func中得到minBlkSize整数倍尺寸的处理,且指针为对齐的
class SIMDaudioStream
{
public:
	forcedinline SIMDaudioStream();
	forcedinline ~SIMDaudioStream();
	forcedinline void SetSize(const intc inputChNum,const intc outputChNum,const intc minBlkSize,const intc maxBlkNum);
	template<typename Func> forcedinline void push(AudioSampleBuffer& ,const Func& ){ jassertfalse;//TODO };
	template<typename Func> forcedinline void dummyPush(AudioSampleBuffer& buffer,const Func& func);
private:
	class MultiChAudioBuffer
	{
	public:
		forcedinline MultiChAudioBuffer()
			:m_buffer()
			,m_sizePreCh(0)
		{}
		forcedinline ~MultiChAudioBuffer(){};
		forcedinline void setSizeAndClear(intc sizePreCh,intc channelNum)
		{
			m_sizePreCh=sizePreCh;
			m_chNum=channelNum;
			m_buffer.setSize(sizePreCh*channelNum);
			m_buffer.clear();
		}
		forcedinline float* getData(intc chIndex) const 
		{
			jassert(chIndex>=0&&chIndex<m_chNum);
			return m_buffer.getPtr()+chIndex*m_sizePreCh; 
		}
		forcedinline intc getSizePreCh() const { return m_sizePreCh; }
	private:
		AudioBuffer<vec1f> m_buffer;
		intc m_sizePreCh;
		intc m_chNum;
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiChAudioBuffer);
	};
private:
	intc m_minBlkSize;
	intc m_maxBlkNum;
	intc m_allBlkSize;
	intc m_prcssdStart;
	intc m_prcssdSize;
	intc m_inputChNum;
	intc m_outputChNum;
	intc m_minChNum;
	intc m_maxChNum;
	intc m_bufferdCount;
	MultiChAudioBuffer m_buffer;
	HeapBlock<float*> m_chPtr;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SIMDaudioStream);
};


void SIMDaudioStream::SetSize(const intc inputChNum,const intc outputChNum,const intc minBlkSize,const intc maxBlkNum)
{
	m_minBlkSize = minBlkSize;
	m_maxBlkNum = maxBlkNum;
	m_prcssdStart = 0;
	m_prcssdSize = m_minBlkSize;
	m_outputChNum = outputChNum;
	m_inputChNum = inputChNum;
	m_maxChNum = std::max(inputChNum,outputChNum);
	m_minChNum = std::min(inputChNum,outputChNum);
	m_buffer.setSizeAndClear(m_maxBlkNum*m_minBlkSize,m_maxChNum);
	m_chPtr.malloc(m_maxChNum);
	m_allBlkSize = m_minBlkSize*m_maxBlkNum;
	m_bufferdCount = 0;
}

SIMDaudioStream::~SIMDaudioStream()
{}

SIMDaudioStream::SIMDaudioStream()
	: m_minBlkSize(0)
	,m_maxBlkNum(0)
	,m_prcssdStart(0)
	,m_prcssdSize(0)
	,m_inputChNum(0)
	,m_outputChNum(0)
	,m_minChNum(0)
	,m_maxChNum(0)
	,m_bufferdCount(0)
	,m_buffer()
	,m_chPtr()
{}


template<typename Func>
void SIMDaudioStream::dummyPush(AudioSampleBuffer& buffer,const Func& func)
{
	jassert(m_minChNum>0);
	jassert(m_maxChNum>0);
	jassert(m_prcssdSize>0);
	jassert(m_prcssdSize>0);

	intc adioBufCounter = 0;
	intc const blockSize=buffer.getNumSamples();

#	if JUCE_DEBUG
	intc saftCounter=0;
#	endif

	m_bufferdCount+=blockSize;

	while(adioBufCounter < blockSize) { //循环,直到本窗口被处理完毕

		const intc needWriteToInputBuffer = jmin( m_prcssdSize ,blockSize-adioBufCounter);

		//处理input的数据和output的数据写入buffer----------------------
		intc cha = 0; //cha是连续的,因为当通道大于inputNum时不需要写入input到buffer,所以有两次ch的遍历,因此cha的定义不能在循环中
		for(; cha < m_maxChNum ; ++cha) {
			float*const out = buffer.getSampleData(cha);
			float*const buf = m_buffer.getData(cha);
			for(intc i = 0; i < needWriteToInputBuffer; ++i) {
				vec1f a;
				a<<(buf + i + m_prcssdStart);
				a >> ( out + i + adioBufCounter );
			}
		}
		//-------------------------------------------------------------

		m_prcssdStart += needWriteToInputBuffer; //prcssdStart已经被使用,要增长
		m_prcssdSize -= needWriteToInputBuffer; //处理好的size变小了
		
		jassert(m_prcssdSize>=0&&m_prcssdSize<=m_allBlkSize);
		jassert(m_prcssdStart >= 0 && m_prcssdStart<=m_allBlkSize);

		if (m_prcssdSize==0) {
			m_prcssdStart = 0;
			intc K = m_bufferdCount / m_minBlkSize; //肯定是整数倍
			K = std::min(m_maxBlkNum,K)*m_minBlkSize;

			intc const sizeNeedProcess = std::min(K,m_allBlkSize);
			jassert(sizeNeedProcess%m_minBlkSize == 0);
			jassert(sizeNeedProcess >= 0);
			jassert(sizeNeedProcess <= m_allBlkSize);
			m_bufferdCount-=sizeNeedProcess;
			jassert(m_bufferdCount>=0);

			if(sizeNeedProcess > 0) { //如果需要处理新的数据,则处理新的数据,此时的新inputData已经在之前被写入了,所以ok
				for(intc i = 0; i < m_maxChNum; ++i) m_chPtr[i] = m_buffer.getData(i); //准备指针
				func(m_chPtr,sizeNeedProcess);
				m_prcssdSize += sizeNeedProcess;
				jassert(m_prcssdSize%m_minBlkSize == 0);
				jassert(m_prcssdSize <= m_allBlkSize);
			}
		}
		adioBufCounter += needWriteToInputBuffer; //计数器增长
		jassert(saftCounter++ < blockSize*10);
	}
}


JUCE_NAMESPACE_END;

#endif // SIMDAudioProcessor_h__
