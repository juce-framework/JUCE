#ifndef ScopedDenormalFlag_h__
#define ScopedDenormalFlag_h__


//自动设置SSE指令集切换为denormal模式
class ScopedDenormalFlag
{
public:
	STACK_CLASS(ScopedDenormalFlag);
	inline static bool IsCpuCanDAZ()
	{
		JUCE_ALIGN(64) uint8 data[512];
		checkPtrSIMD(&data,64);
		zeromem(data,512);
		#if JUCE_MSVC && (!JUCE_64BIT)
			int* h=(int*)&data;
			__asm {
				mov esi,[h]
				fxsave [esi]
			}
		#elif JUCE_MSVC && JUCE_64BIT
			//return true; //x64 must can DAZ...
			_fxsave(data);
		#else
			__asm__ __volatile__(" fxsave; "::"m"(data));
		#endif
		uint32 f=*(uint32*)&data[28];
		const uint32 k=32;
		f=f&k;
		return f==32;
	}
	enum Rounding
	{
		ROUND_NEAREST = 0,
		ROUND_NEGATIVE,
		ROUND_POSITIVE,
		ROUND_TO_ZERO
	};
	forcedinline ScopedDenormalFlag(Rounding mode = ROUND_NEAREST)
					:sse_control_store(_mm_getcsr())
					,needProcess()
	{
		const unsigned int k=(0x9fc0u | (mode << 13));
		if (sse_control_store!=k) {
			_mm_setcsr(k);
			needProcess=true;
		 } else {
			needProcess=false;
		}
	};
	forcedinline ~ScopedDenormalFlag()
	{
		if (needProcess) {
			_mm_setcsr(sse_control_store);
		}
	};
private:
	const unsigned int sse_control_store;
	bool needProcess;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopedDenormalFlag);
};


#endif // ScopedDenormalFlag_h__
