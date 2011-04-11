/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#define UseExtendedThingResource 1
#include <AudioUnit/AudioUnit.r>

//==============================================================================
/*  The JucePluginCharacteristics.h file is supposed to live in your plugin-specific
    project directory, and has to contain info describing its name, type, etc. For
    more info, see the JucePluginCharacteristics.h that is included in the demo plugin.

    You may need to adjust the include path of your project to make sure it can be 
    found by this include statement. (Don't hack this file to change the include path)
*/
#include "JucePluginCharacteristics.h"


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
 #define COMP_MANUF		JucePlugin_AUManufacturerCode
 #define VERSION        JucePlugin_VersionCode
 #define NAME           JucePlugin_Manufacturer ": " JucePlugin_Name " View"
 #define DESCRIPTION    NAME
 #define ENTRY_POINT    JucePlugin_AUExportPrefixQuoted "ViewEntry"

 #include "AUResources.r"
#endif
