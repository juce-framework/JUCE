#ifndef ztd_int24_h__
#define ztd_int24_h__

inline void memmove_af4_uw4( float*const opt_a , int24*const ipt_u ,int length_4 )
{
	jassert(ptrX != nullptr);
	jassert(ptrY != nullptr);
	jassert(len >= 0);
	jassert(len % 4 == 0);
    __asm__(
        ".intel_syntax noprefix;"
        "lea %[length],[ %[length]*4 ];"
        "lea %[output],[ %[output]+%[length] ];"
        "neg %[length];"
        "jz END%=;"
        ".align 16;"
        "LOOP_START%=:"
            "movd xmm4,[ %[input]+0x3];"
            "movd xmm5,[ %[input] ];"
            "movd xmm0,[ %[input]+0x9];"
            "movd xmm6,[ %[input]+0x6];"
            "punpckldq xmm4,xmm5;"
            "lea %[length],[ %[length]+16 ];"
            "lea %[output],[ %[output]+12 ];"
            "punpckldq xmm0,xmm6;"
            "punpcklqdq xmm0,xmm4;"
            "andps xmm0,%[temp1];"
            "subps xmm0,%[temp2];"
            "cvtdq2ps xmm0,xmm0;"
            "movaps [ %[output]+%[length]-16 ],xmm0;"
        "jnz LOOP_START%=;"
        "END%=:"
        ".att_syntax noprefix;"
        :
        :[input]"r"(ipt_u),[output]"r"(opt_a),[length]"r"(length_4),[temp1]"x"(_mm_set1_epi32(0x00FFFFFF)),[temp2]"x"(_mm_set1_epi32(8388608)),[temp3]"x"(_mm_set1_ps(1.0f/8388608.f))
        :"memory","xmm4","xmm5","xmm6","xmm0"
    );
}

#endif // ztd_int24_h__
