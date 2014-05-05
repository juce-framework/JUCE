#ifndef ztd_SIMDmemmove_h__
#define ztd_SIMDmemmove_h__

inline void memmove_af4( float*const opt_a , float*const ipt_a , const size_t length_4 )
{
	checkSizeSIMD(len,4);
	checkPtrSIMD(y,16);
	checkPtrSIMD(x,16);
	jassert(y!=x&&std::abs(y-x)>=4);
	jassert(len>=0);
	__asm__(
		".intel_syntax noprefix;"
		"lea %0,[%0*4];"
		"lea %1,[%1+%0];"
		"lea %2,[%2+%0];"
		"neg %0;"
		"jz END%=;"
		".align 16;"
		"LOOP_START%=:"
			"movaps xmm0,[%1+%0];"
			"add %0,16;"
			"movaps [%2+%0-16],xmm0;"
		"jnz LOOP_START%=;"
		"END%=:"
		".att_syntax noprefix;"
		:
		:"r"(length_4),"r"(ipt_a),"r"(opt_a)
		:"xmm0","memory"
	);
};

#endif // ztd_SIMDmemmove_h__
