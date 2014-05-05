#ifndef ASMdef_h__
#define ASMdef_h__

#include "../../ztd_core.h"

#if defined(JUCE_MSVC) && defined(JUCE_32BIT)
#	define ZTD_ASM_X86_MSVC 1
#	define ZTD_ASM_X64_MSVC 0
#	define ZTD_ASM_X86_GCC 0
#	define ZTD_ASM_X64_GCC 0
#elif defined(JUCE_MSVC) && defined(JUCE_64BIT)
#	define ZTD_ASM_X86_MSVC 0
#	define ZTD_ASM_X64_MSVC 1
#	define ZTD_ASM_X86_GCC 0
#	define ZTD_ASM_X64_GCC 0
#elif defined(JUCE_GCC) && defined(JUCE_32BIT)
#	define ZTD_ASM_X86_MSVC 0
#	define ZTD_ASM_X64_MSVC 0
#	define ZTD_ASM_X86_GCC 1
#	define ZTD_ASM_X64_GCC 0
#elif defined(JUCE_GCC) && defined(JUCE_64BIT)
#	define ZTD_ASM_X86_MSVC 0
#	define ZTD_ASM_X64_MSVC 0
#	define ZTD_ASM_X86_GCC 1
#	define ZTD_ASM_X64_GCC 0
#else 
#	error "dont know what to do "
#endif

#endif // ASMdef_h__
