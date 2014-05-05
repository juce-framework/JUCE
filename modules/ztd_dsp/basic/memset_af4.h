#ifndef dsadddddddddddddddddddddddwwwwwwwwwww4444444444444444444444
#define dsadddddddddddddddddddddddwwwwwwwwwww4444444444444444444444

#include "memory.h"

inline void memset_af4( float*const restrict ptr_a , const size_t length_4 , float value )
{
	checkPtrSIMD(p,16);
	checkSizeSIMD(len,4);
	ASSERT(len>=0);
	__asm__(
        ".intel_syntax noprefix;"
		"lea %1,[%1*4];"
		"lea %0,[%0+%1];"
		"neg %1;"
		"jz END%=;"
		".align 16;"
		"START%=:"
			"add %1,16;"
			"movaps [%0+%1-16],%2;"
		"jnz START%=;"
		"END%=:"
		".att_syntax noprefix;"
		:
        :"r"(ptr_a),"r"(length_4),"x"(_mm_set1_ps(value));
        :"memory"
	);
}

#endif // simd_zeromem_h__
