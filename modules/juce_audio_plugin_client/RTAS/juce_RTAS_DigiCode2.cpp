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

#include "../../juce_core/system/juce_TargetPlatform.h"
#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_RTAS

#include "juce_RTAS_DigiCode_Header.h"

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wcomment"
 #pragma clang diagnostic ignored "-Wextra-tokens"
 #pragma clang diagnostic ignored "-Wnon-virtual-dtor"
 #pragma clang diagnostic ignored "-Wreorder"
 #pragma clang diagnostic ignored "-Wdeprecated"
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

#include <CEffectProcessMIDI.cpp>
#include <PlugInUtils.cpp>

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

#endif
