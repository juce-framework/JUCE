/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../juce_core/system/juce_TargetPlatform.h"
#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_RTAS

 #include "../utility/juce_IncludeSystemHeaders.h"
 #include "juce_RTAS_DigiCode_Header.h"

 #ifdef __clang__
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
  #pragma clang diagnostic ignored "-Wextra-tokens"
  #pragma clang diagnostic ignored "-Wreorder"
 #endif

 /*
    This file is used to include and build the required digidesign CPP files without your project
    needing to reference the files directly. Because these files will be found via your include path,
    this means that the project doesn't have to change to cope with people's SDKs being in different
    locations.

    Important note on Windows: In your project settings for the three juce_RTAS_DigiCode.cpp files and
    the juce_RTAS_Wrapper.cpp file, you need to set the calling convention to "__stdcall".
    If you don't do this, you'll get some unresolved externals and will spend a long time wondering what's
    going on... All the other files in your project can be set to use the normal __cdecl convention.

    If you get an error building the includes statements below, check your paths - there's a full
    list of the necessary Digidesign paths in juce_RTAS_Wrapper.cpp
 */

 #if WINDOWS_VERSION
  #undef _UNICODE
  #undef UNICODE
  #define DllMain DllMainRTAS
  #include <DLLMain.cpp>
  #undef DllMain
  #include <DefaultSwap.cpp>
 #else
  #include <PlugInInitialize.cpp>
  #include <Dispatcher.cpp>
 #endif

 #ifdef __clang__
  #pragma clang diagnostic pop
 #endif

#else

 #if _MSC_VER
  short __stdcall NewPlugIn (void*)                          { return 0; }
  short __stdcall _PI_GetRoutineDescriptor (long, void*)     { return 0; }
 #endif

#endif
