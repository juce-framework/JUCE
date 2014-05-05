#ifndef ztd_SIMDdef_h__
#define ztd_SIMDdef_h__

#define ALIGNED(x) JUCE_ALIGN(x)

//#define makeAligned(x,k) (decltype(x))( ((uintptr_t)x+3)&16 );

#define makeSIMD(x,k) ( (x)&(~((decltype(x))(k-1))) )

#endif // ztd_SIMDdef_h__
