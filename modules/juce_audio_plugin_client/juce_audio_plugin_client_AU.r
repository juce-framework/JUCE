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

#define UseExtendedThingResource 1
#include <AudioUnit/AudioUnit.r>

//==============================================================================
/*  The AppConfig.h file should be a file in your project, containing info to describe the
    plugin's name, type, etc. The introjucer will generate this file automatically for you.

    You may need to adjust the include path of your project to make sure it can be
    found by this include statement. (Don't hack this file to change the include path)
*/
#include "AppConfig.h"


//==============================================================================
// component resources for Audio Unit
#define RES_ID          1000
#define COMP_TYPE       JucePlugin_AUMainType
#define COMP_SUBTYPE    JucePlugin_AUSubType
#define COMP_MANUF      JucePlugin_AUManufacturerCode
#define VERSION         JucePlugin_VersionCode
#define NAME            JucePlugin_Manufacturer ": " JucePlugin_Name
#define DESCRIPTION     JucePlugin_Desc
#define ENTRY_POINT     JucePlugin_AUExportPrefixQuoted "Entry"

#include "AUResources.r"

//==============================================================================
// component resources for Audio Unit Carbon View

#ifndef BUILD_AU_CARBON_UI
 #define BUILD_AU_CARBON_UI 1
#endif

#if BUILD_AU_CARBON_UI
 #define RES_ID         2000
 #define COMP_TYPE      kAudioUnitCarbonViewComponentType
 #define COMP_SUBTYPE   JucePlugin_AUSubType
 #define COMP_MANUF     JucePlugin_AUManufacturerCode
 #define VERSION        JucePlugin_VersionCode
 #define NAME           JucePlugin_Manufacturer ": " JucePlugin_Name " View"
 #define DESCRIPTION    NAME
 #define ENTRY_POINT    JucePlugin_AUExportPrefixQuoted "ViewEntry"

 #include "AUResources.r"
#endif
