#ifndef ztd_reverse_h__
#define ztd_reverse_h__

//必须是8的整数倍
inline void reverse_af4( float*const ptr_a , const size_t length_8 )
{
	jassert(size%8==0&&size>=8);
	checkPtrSIMD(ptr,16);
    const size_t halfSize=length_8>>1;
    float*const last_a = ptr_a + length_8 - 4 ;
    __asm__(
        ".intel_syntax noprefix;"
        "lea %2,[%2*4];"
        "lea %0,[%0+%2];"
        "neg %2;"
        "jz END%=;"
        "START%=:"
        ".align 16;"
        "movaps xmm0,[%0+%2];"
        "movaps xmm1,[%1];"
        "add %2,16;"
        "lea %1,[%1-16];"
        "pshufd xmm0,xmm0,0b00011011;"
        "movaps [%1+16],xmm0;"
        "pshufd xmm1,xmm1,0b00011011;"
        "movaps [%0+%2-16],xmm1;"
        "jnz START%=;"
        "END%=:"
        ".att_syntax noprefix;"
        :
        :"r"(ptr_a),"r"(last_a),"r"(halfSize)
        :"xmm0","xmm1","memory"
    );
};

#endif // ztd_reverse_h__
