/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if (JucePlugin_Enable_ARA || (JUCE_PLUGINHOST_ARA && (JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU))) && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)
namespace juce
{
 #if ARA_ENABLE_INTERNAL_ASSERTS
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
}
#endif

#if JucePlugin_Enable_ARA
#include "juce_ARADocumentControllerCommon.cpp"
#include "juce_ARADocumentController.cpp"
#include "juce_ARAModelObjects.cpp"
#include "juce_ARAPlugInInstanceRoles.cpp"
#include "juce_AudioProcessor_ARAExtensions.cpp"

ARA_SETUP_DEBUG_MESSAGE_PREFIX (JucePlugin_Name);
#endif
