/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#define UseExtendedThingResource 1
#include <AudioUnit.r>

//==============================================================================
/*  The JucePluginDefines file should be a file in your project, containing info to describe the
    plugin's name, type, etc. The Projucer will generate this file automatically for you.

    You may need to adjust the include path of your project to make sure it can be
    found by this include statement. (Don't hack this file to change the include path)
*/
#include "JucePluginDefines.h"


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
