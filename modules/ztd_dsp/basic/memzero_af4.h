#ifndef simd_zeromem_h__gfdsgfds
#define simd_zeromem_h__gfdsgfds

#include "memory.h"

inline void memzero_af4( float*const restrict ptr_a , const size_t length_4 )
{
	checkPtrSIMD(p,16);
	checkSizeSIMD(len,4);
	jassert(len>=0);
	__asm__(
        ".intel_syntax noprefix;"
		"xorps xmm0,xmm0;"
		"lea %1,[%1*4];"
		"lea %0,[%0+%1];"
		"neg %1;"
		"jz END%=;"
		".align 16;"
		"START%=:"
			"add %1,16;"
			"movaps [%0+%1-16],xmm0;"
		"jnz START%=;"
		"END%=:"
		".att_syntax noprefix;"
		:
        :"r"(ptr_a),"r"(length_4)
        :"xmm0","memory"
	);
}

#endif // simd_zeromem_h__
