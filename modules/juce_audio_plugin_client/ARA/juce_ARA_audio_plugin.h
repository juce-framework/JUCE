#pragma once

// Include juce preamble
#include "AppConfig.h"
#include <juce_core/juce_core.h>

#if JucePlugin_Enable_ARA

namespace juce
{

// Configure ARA debug support prior to including ARA headers
#if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS) || JUCE_LOG_ASSERTIONS

    #define ARA_ENABLE_INTERNAL_ASSERTS 1

    extern JUCE_API void JUCE_CALLTYPE handleARAAssertion (const char* file, const int line, const char* diagnosis) noexcept;
    #define ARA_HANDLE_ASSERT(file, line, diagnosis)    handleARAAssertion (file, line, diagnosis)

   #if JUCE_LOG_ASSERTIONS
    #define ARA_ENABLE_DEBUG_OUTPUT 1
   #endif

#else

    #define ARA_ENABLE_INTERNAL_ASSERTS 0

#endif // (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS) || JUCE_LOG_ASSERTIONS
 
}

// Include ARA headers
#include <ARA_Library/PlugIn/ARAPlug.h>

namespace juce
{
    using ARAContentUpdateScopes = ARA::ContentUpdateScopes;
}

#endif
