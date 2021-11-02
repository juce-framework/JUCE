#include <juce_core/system/juce_TargetPlatform.h>
#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Enable_ARA

#include "../utility/juce_IncludeSystemHeaders.h"
#include "../utility/juce_IncludeModuleHeaders.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-parameter", "-Wgnu-zero-variadic-macro-arguments")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100)

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

JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

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
