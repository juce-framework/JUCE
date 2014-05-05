#ifndef ztd_TargetPlatform_h__
#define ztd_TargetPlatform_h__


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


#endif // ztd_TargetPlatform_h__
