#include "juce_ARA_audio_plugin.h"

#if JucePlugin_Enable_ARA

#define ARA_DEBUG_MESSAGE_PREFIX JucePlugin_Name

#include <ARA_Library/PlugIn/ARAPlug.cpp>
#include <ARA_Library/Debug/ARADebug.c>
#include <ARA_Library/Dispatch/ARAPlugInDispatch.cpp>
#include <ARA_Library/Utilities/ARAPitchInterpretation.cpp>

// Include these source files directly for now
#include "juce_ARAModelObjects.cpp"
#include "juce_ARADocumentController.cpp"
#include "juce_ARAAudioReaders.cpp"
#include "juce_ARAPlugInInstanceRoles.cpp"
#include "juce_AudioProcessor_ARAExtensions.cpp"

namespace juce
{

#if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS) || JUCE_LOG_ASSERTIONS
JUCE_API void JUCE_CALLTYPE handleARAAssertion (const char* file, const int line, const char* diagnosis) noexcept
{
   #if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS)
    DBG (diagnosis);
   #endif

    logAssertion (file, line);

   #if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS)
    if (juce_isRunningUnderDebugger())
        JUCE_BREAK_IN_DEBUGGER;
    JUCE_ANALYZER_NORETURN
   #endif
  }
#endif

 } // namespace juce

#endif
