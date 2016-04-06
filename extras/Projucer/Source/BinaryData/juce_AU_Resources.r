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

#define UseExtendedThingResource 1
#include <AudioUnit/AudioUnit.r>

//==============================================================================
/*  The AppConfig.h file should be a file in your project, containing info to describe the
    plugin's name, type, etc. The Projucer will generate this file automatically for you.

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

#include <CoreServices/CoreServices.r>

resource 'STR ' (RES_ID, purgeable) { NAME };
resource 'STR ' (RES_ID + 1, purgeable) { DESCRIPTION };
resource 'dlle' (RES_ID) { ENTRY_POINT };

resource 'thng' (RES_ID, NAME)
{
    COMP_TYPE, COMP_SUBTYPE, COMP_MANUF,
    0, 0, 0, 0,
    'STR ', RES_ID,
    'STR ', RES_ID + 1,
    0, 0, VERSION,
    componentHasMultiplePlatforms | componentDoAutoVersion, 0,
    {
        0x10000000, 'dlle', RES_ID, platformIA32NativeEntryPoint,
        0x10000000, 'dlle', RES_ID, 8
    }
};

#undef RES_ID
#undef COMP_TYPE
#undef COMP_SUBTYPE
#undef COMP_MANUF
#undef VERSION
#undef NAME
#undef DESCRIPTION
#undef ENTRY_POINT
#undef NeedLeadingComma

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

    resource 'STR ' (RES_ID, purgeable) { NAME };
    resource 'STR ' (RES_ID + 1, purgeable) { DESCRIPTION };
    resource 'dlle' (RES_ID) { ENTRY_POINT };

    resource 'thng' (RES_ID, NAME)
    {
        COMP_TYPE, COMP_SUBTYPE, COMP_MANUF,
        0, 0, 0, 0,
        'STR ', RES_ID,
        'STR ', RES_ID + 1,
        0, 0, VERSION,
        componentHasMultiplePlatforms | componentDoAutoVersion, 0,
        {
            0x10000000, 'dlle', RES_ID, platformIA32NativeEntryPoint,
            0x10000000, 'dlle', RES_ID, 8
        }
    };
#endif
