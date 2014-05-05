#ifndef ____header__dsaaaaaaaaaaaaaaaaaaaztdsadsa
#define ____header__dsaaaaaaaaaaaaaaaaaaaztdsadsa

#if JUCE_ARM
#	error "ztd does not support arm processor...yet!"
#endif

#if JUCE_MSVC
//#	error "fuck msvc! no asm in x64! why?!!!"
#endif

#if JUCE_GCC
#	if JUCE_COMPILER_SUPPORTS_NOEXCEPT && JUCE_COMPILER_SUPPORTS_NULLPTR && JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS && JUCE_COMPILER_SUPPORTS_OVERRIDE_AND_FINAL
#	else
#		error "! what we need is C++11!"
#	endif
#endif


#include <wchar.h>
//#include <io.h>
#include <fcntl.h>

#include <emmintrin.h>
#include <type_traits>
#include <tuple>
#include <functional>
#include <new> 

using std::is_same;
using std::is_pointer;
using std::is_base_of;
using std::is_standard_layout;
using std::is_trivial;
using std::remove_cv;
using std::remove_const;
using std::remove_pointer;
using std::is_reference;
using std::tuple;
using std::get;
using std::initializer_list;
using std::function;

typedef unsigned int uint;

typedef __m128 m128;
typedef __m128d m128d;
typedef __m128i m128i;
typedef ptrdiff_t intc;

/******************************************************
 * 非C++标准的,但我觉得比较有用的跟编译器有关的宏
 *******************************************************/
#if JUCE_MSVC
#	define noinline __declspec(noinline)
#	define SELECT_ANY __declspec( selectany )
#	define RESTRICT __restrict
#	define ASSUME(cond) __assume(cond)
#	define ASSUME_PTR_SIMD(x) __assume(((intptr_t)x)%16==0)
#	define ASSUME_SIZE_SIMD(x) __assume(x%4==0)
#	define likely(x) (x)
#	define unlikely(x) (x)
#	define likely_if(x) if(x)
#	define unlikely_if(x) if(x)
#	define likely_while(x) while(likely(x))
#	define unlikely_while(x) while(unlikely(x))
#	define ALIGNOF(x) __alignof(x)
#else
#	define noinline __attribute__ ((noinline))
#	define SELECT_ANY __attribute__((selectany))
#	define RESTRICT __restrict__
#	define ASSUME(cond) do { if (!(cond)) __builtin_unreachable(); } while (0) //¥À¥¶ø…ƒ‹”–Œ Ã‚
#	define ASSUME_PTR_SIMD(x) __builtin_assume_aligned(x,16)
#	define ASSUME_SIZE_SIMD(x) ASSUME(x%4==0)
#	define likely(x) __builtin_expect(!!(x),1)
#	define unlikely(x)  __builtin_expect(!!(x),0)
#	define likely_if(x) if(likely(x))
#	define unlikely_if(x) if(unlikely(x))
#	define likely_while(x) while(likely(x))
#	define unlikely_while(x) while(unlikely(x))
#	define ALIGNOF(x) __alignof__(x)
#endif

#if JUCE_CLANG
#	undef SELECT_ANY
#	define SELECT_ANY
#endif

#define ZASSERT(x) if(!(x)) { juce_breakDebugger; JUCE_ANALYZER_NORETURN }
#define DEBUG_BREAK juce_breakDebugger

/********************************************************
 *	NEVER_TOUCH宏
 ********************************************************/
#if JUCE_DEBUG
#	define NEVER_TOUCH jassertfalse
#else
#	define NEVER_TOUCH ASSUME(0)
#endif

/*******************************************************************
 * 基础检查
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
#define IS_DEFAULTCOPYABLE(classname) IS_TRIVIAL(classname) //todo: mingw这里有问题,得研究一下标准库

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


/*****************************************************************
 * 各种检查宏
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

#define ALIGNED(x) JUCE_ALIGN(x)

//#define makeAligned(x,k) (decltype(x))( ((uintptr_t)x+3)&16 );

#define makeSIMD(x,k) ( (x)&(~((decltype(x))(k-1))) )

/*********************************************************
 * 命名空间的宏
 *********************************************************/
#define NAMESPACE_START(namespaceName) namespace namespaceName {
#define NAMESPACE_END }


#endif //____header__dsaaaaaaaaaaaaaaaaaaaztdsadsa
