#ifndef ztd_int24_h__
#define ztd_int24_h__

ZTD_NAMESPACE_START;

typedef char int24[3];

forcedinline m128 cvt_vec4f_i24(int24*const input)
{
	int a = *(int32*)input; //这四个均有些脏数据
	int b= *(int32*)(input+1);
	int c= *(int32*)(input+2);
	int d= *(int32*)(input+3);
	m128i A=_mm_set_epi32(a,b,c,d);
	A=_mm_and_si128(A,_mm_set1_epi32(0x0FFFFFF)); //清除脏数据
	A=_mm_sub_epi32(A,_mm_set1_epi32(8388608)); //这...减法...减一半
	m128 k=_mm_cvtepi32_ps(A); //转换
	k = _mm_mul_ps(k,_mm_set1_ps(1.0f / 8388608.f)); //归一化
	return k;
}

inline void SIMDmemmove(float*const ptrY,int24*const ptrX,int len)
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
        :[input]"r"(ptrX),[output]"r"(ptrY),[length]"r"(len),[temp1]"x"(_mm_set1_epi32(0x00FFFFFF)),[temp2]"x"(_mm_set1_epi32(8388608)),[temp3]"x"(_mm_set1_ps(1.0f/8388608.f))
        :"memory","xmm4","xmm5","xmm6","xmm0"
    );
}

ZTD_NAMESPACE_END;

#endif // ztd_int24_h__
