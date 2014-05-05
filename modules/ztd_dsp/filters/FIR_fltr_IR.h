#ifndef ztd_FIRfilterIR_h__
#define ztd_FIRfilterIR_h__

ZTD_NAMESPACE_START;

class FIRfilterIR
{
public:
	forcedinline FIRfilterIR():m_data(),m_irSize(0){};
	forcedinline ~FIRfilterIR(){};
	forcedinline operator float*() const { return m_data; }
	forcedinline intc getIRSize() const { jassert(m_irSize>0); return m_irSize; }
	template<typename Func> void set(intc newSize,const Func& func)
	{
		intc bigSize = (newSize+7) & 0xFFFFFFF8; //因为要求是8的整数倍
		jassert(newSize>0&&newSize%8==0);
		m_data.calloc(bigSize); //此处已经将IR全部归零,如果newSize不是8的整数倍,则ir的尾部将有一些0
		m_irSize = bigSize;
		func(m_data.getData());
		SIMDreverse(m_data,bigSize); //尾部的0将翻转到头部
	}
	void setOne(intc newSize)
	{
		set(newSize,[](float*const ptr){
			ptr[0]=1.0f;
		});
	}
private:
	AlignedHeapBlock<float> m_data;
	intc m_irSize;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FIRfilterIR);
};

ZTD_NAMESPACE_END;

#endif // ztd_FIRfilterIR_h__
