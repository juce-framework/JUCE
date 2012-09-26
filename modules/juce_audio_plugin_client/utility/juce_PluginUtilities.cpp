/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#if _MSC_VER
#include <windows.h>

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

#include "../utility/juce_CheckSettingMacros.h"
#include "juce_IncludeModuleHeaders.h"

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

    (void) reserved;
    return TRUE;
}

#endif
