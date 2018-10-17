#pragma once

// Include juce preamble
#include "AppConfig.h"
#include <juce_core/juce_core.h>

#if JucePlugin_Enable_ARA

 // Configure ARA debug support prior to including ARA headers
 #if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS) || JUCE_LOG_ASSERTIONS
  // define a juce assert and define the ARA_INTERNAL_ASSERT macro to use the function below
  namespace juce
  {
      extern JUCE_API void JUCE_CALLTYPE handleARAAssertion (const char* file, const int line, const char* diagnosis) noexcept;
  }
  // Enable ARA_INTERNAL_ASSERT and define it to be juce::handleARAAssertion
  #define ARA_ENABLE_INTERNAL_ASSERTS 1
  #define ARA_HANDLE_ASSERT(file, line, diagnosis)    juce::handleARAAssertion(file, line, diagnosis)
  
  #if JUCE_LOG_ASSERTIONS
   #define ARA_ENABLE_DEBUG_OUTPUT 1
  #endif
 #else
  // Otherwise disable ARA internal asserts
  #define ARA_ENABLE_INTERNAL_ASSERTS 0
 #endif
 
 // Include ARA headers
 #include <ARA_Library/PlugIn/ARAPlug.h>

#endif
