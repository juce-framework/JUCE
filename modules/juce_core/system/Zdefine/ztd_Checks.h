#ifndef ztd_Checks_h__
#define ztd_Checks_h__

/*******************************************************************
以下为针对simd的一些检查函数,包括2的指数倍,指针16byte对齐等等
********************************************************************/
#define checkPowerOfTwo(number) jassert(number>1); jassert( (number&(number-1))==0 );

#define checkPtrSIMD(ptr,size) jassert( ((uintptr_t)(ptr))%size==0 );

#define checkPtrSIMD_nonNullptr(ptr,size) checkPtrSIMD(ptr,size); jassert(ptr!=nullptr);

#define checkSizeSIMD(x,size) jassert(x%size==0);

#define checkPtrSIMD_nonZero(x,size) checkSizeSIMD(x,size); jassert(x>0);

#define checkInRange(start,end,x) jassert(x>=start&&x<end)

#define IS_TRIVIAL(classname) std::is_trivial<classname>::value
#define IS_POD(classname) std::is_pod<classname>::value
#define IS_STANDARDLAYOUT(classname) std::is_standard_layout<classname>::value
#define IS_DEFAULTCOPYABLE(classname) IS_TRIVIAL(classname) //todo: 由于mingw的问题,这里用不了is_trivially_copy_assignable

#define checkTRIVIAL(classname) static_assert(IS_TRIVIAL(classname),"this class must be Trivial")
#define checkPOD(classname) static_assert(IS_POD(classname),"this class must be POD")
#define checkSTANDARDLAYOUT(classname) static_assert(IS_STANDARDLAYOUT(classname),"this class must be standard layout")
#define checkDefaultCopyable(classname) static_assert( IS_DEFAULTCOPYABLE(classname),"this class must be default copyable" )

#if JUCE_DEBUG
#	define checkTRIVIAL_WHEN_RELEASE(classname)
#	define checkPOD_WHEN_RELEASE(classname)
#	define checkSTANDARDLAYOUT_WHEN_RELEASE(classname)
#	define checkThisPtrAligned jassert(((uintptr_t)this)%16==0);
#	define checkDebugRunRelease(x) jassert((x))
#else
#	define checkTRIVIAL_WHEN_RELEASE(classname) checkTRIVIAL(classname)
#	define checkPOD_WHEN_RELEASE(classname) checkPOD(classname)
#	define checkSTANDARDLAYOUT_WHEN_RELEASE(classname) checkSTANDARDLAYOUT(classname)
#	define checkThisPtrAligned
#	define checkDebugRunRelease(x) (x)
#endif


/******************************************************************
* 下面是针对内联汇编定义的一些宏:
*******************************************************************/
#ifdef JUCE_MSVC
#	define ASM_BLOCK __asm
#else
#	define ASM_BLOCK __asm__
#endif

#ifdef JUCE_64BIT
#	define VAX rax
#	define VBX rbx
#	define VCX rcx
#	define VDX rdx
#	define VSI rsi
#	define VDI rdi
#else
#	define VAX eax
#	define VBX ebx
#	define VCX ecx
#	define VDX edx
#	define VSI esi
#	define VDI edi
#endif


/******************************************************************
* 关于分支检测的一些定义
*******************************************************************/


/*****************************************************************
* 一些有效的数学函数:
******************************************************************/
#define powerTwo(x) (1u<<(x))
#define modPowerTwo(x,number) ((x)&(number-1))
#define divPowerTwo(x,number) ((x)>>(number-1))
#define floorPowTwo(x,number) ((x)&(~(number-1)))

#define FLOAT_NORMAL_MIN std::numeric_limits<float>::min()
#define DOUBLE_NORMAL_MIN std::numeric_limits<double>::min()
#define FLOAT_REAL_MIN std::numeric_limits<float>::denorm_min()
#define DOUBLE_REAL_MIN std::numeric_limits<double>::denorm_min()

#define isDenormalFloat(x) (std::fabs(x)==0.f||std::fabs(x)>=FLOAT_NORMAL_MIN)
#define isDenormalDouble(x) (std::abs(x)==0.0||std::abs(x)>=DOUBLE_NORMAL_MIN)

#define checkDenormalFloat(x) jassert(isDenormalFloat(x))
#define checkDenormalDouble(x) jassert(isDenormalDouble(x))

#endif // ztd_Checks_h__
