/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include <AudioUnit/AudioUnit.r>
#include <AudioUnit/AudioUnitCarbonView.r>

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
#define RES_ID			1000
#define COMP_TYPE		JucePlugin_AUMainType
#define COMP_SUBTYPE	JucePlugin_AUSubType
#define COMP_MANUF		JucePlugin_AUManufacturerCode
#define VERSION			JucePlugin_VersionCode
#define NAME			JucePlugin_Manufacturer ": " JucePlugin_Name
#define DESCRIPTION		JucePlugin_Desc
#define ENTRY_POINT		JucePlugin_AUExportPrefixQuoted "Entry"

#include "/Developer/Examples/CoreAudio/AudioUnits/AUPublic/AUBase/AUResources.r"

//==============================================================================
// component resources for Audio Unit Carbon View
#define RES_ID			2000
#define COMP_TYPE		kAudioUnitCarbonViewComponentType
#define COMP_SUBTYPE	JucePlugin_AUSubType
#define COMP_MANUF		JucePlugin_AUManufacturerCode
#define VERSION			JucePlugin_VersionCode
#define NAME			JucePlugin_Manufacturer ": " JucePlugin_Name " View"
#define DESCRIPTION		NAME
#define ENTRY_POINT		JucePlugin_AUExportPrefixQuoted "ViewEntry"

#include "/Developer/Examples/CoreAudio/AudioUnits/AUPublic/AUBase/AUResources.r"
