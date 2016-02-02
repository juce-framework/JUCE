/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#if _MSC_VER || defined (__MINGW32__) || defined (__MINGW64__)
 #include <windows.h>
#endif

#include "../../juce_core/system/juce_TargetPlatform.h"
#include "../utility/juce_CheckSettingMacros.h"
#include "juce_IncludeModuleHeaders.h"

#if _MSC_VER || JUCE_MINGW

#if JucePlugin_Build_RTAS
 extern "C" BOOL WINAPI DllMainRTAS (HINSTANCE, DWORD, LPVOID);
#endif

extern "C" BOOL WINAPI DllMain (HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
        Process::setCurrentModuleInstanceHandle (instance);

   #if JucePlugin_Build_RTAS
    if (GetModuleHandleA ("DAE.DLL") != 0)
    {
       #if JucePlugin_Build_AAX
        if (! File::getSpecialLocation (File::currentExecutableFile).hasFileExtension ("aaxplugin"))
       #endif
            return DllMainRTAS (instance, reason, reserved);
    }
   #endif

    ignoreUnused (reserved);
    return TRUE;
}

#endif

//==============================================================================
/** Somewhere in the codebase of your plugin, you need to implement this function
    and make it return a new instance of the filter subclass that you're building.
*/
extern AudioProcessor* JUCE_CALLTYPE createPluginFilter();

AudioProcessor* JUCE_CALLTYPE createPluginFilterOfType (AudioProcessor::WrapperType type)
{
    AudioProcessor::setTypeOfNextNewPlugin (type);
    AudioProcessor* const pluginInstance = createPluginFilter();
    AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Undefined);

    // your createPluginFilter() method must return an object!
    jassert (pluginInstance != nullptr && pluginInstance->wrapperType == type);

    return pluginInstance;
}
