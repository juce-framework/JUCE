/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
