// TODO formalize ARA includes

#include "juce_ARA_interface.h"

#include <ARA_Library/PlugIn/ARAPlug.cpp>
#include <ARA_Library/Debug/ARADebug.c>
#include <ARA_Library/Dispatch/ARAPlugInDispatch.cpp>

// Include these source files directly for now
#include "juce_ARADocumentController.cpp"
#include "juce_ARARegionSequence.cpp"
#include "juce_ARAAudioSource.cpp"

namespace juce
{

#if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS) || JUCE_LOG_ASSERTIONS
     JUCE_API void JUCE_CALLTYPE handleARAAssertion(const char* file, const int line, const char* diagnosis) noexcept
     {
 #if (JUCE_DEBUG && ! JUCE_DISABLE_ASSERTIONS)
         DBG(diagnosis);
 #endif
 
         juce::logAssertion (file, line);
 
         jassertfalse;
     }
#endif

} // namespace juce