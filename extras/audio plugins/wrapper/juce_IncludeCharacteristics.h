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

#ifndef __JUCE_INCLUDECHARACTERISTICS_JUCEHEADER__
#define __JUCE_INCLUDECHARACTERISTICS_JUCEHEADER__

//==============================================================================
/*  The JucePluginCharacteristics.h file is supposed to live in your plugin-specific
    project directory, and has to contain info describing its name, type, etc. For
    more info, see the JucePluginCharacteristics.h that is included in the demo plugin.

    You may need to adjust the include path of your project to make sure it can be
    found by this include statement. (Don't hack this file to change the include path)
*/
#include "JucePluginCharacteristics.h"


//==============================================================================
// The following stuff is just to cause a compile error if you've forgotten to
// define all your plugin settings properly.

#ifndef JucePlugin_IsSynth
 #error "You need to define the JucePlugin_IsSynth value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_ManufacturerCode
 #error "You need to define the JucePlugin_ManufacturerCode value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_PluginCode
 #error "You need to define the JucePlugin_PluginCode value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_ProducesMidiOutput
 #error "You need to define the JucePlugin_ProducesMidiOutput value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_WantsMidiInput
 #error "You need to define the JucePlugin_WantsMidiInput value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_MaxNumInputChannels
 #error "You need to define the JucePlugin_MaxNumInputChannels value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_MaxNumOutputChannels
 #error "You need to define the JucePlugin_MaxNumOutputChannels value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_PreferredChannelConfigurations
 #error "You need to define the JucePlugin_PreferredChannelConfigurations value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_Latency
 #error "You need to define the JucePlugin_Latency value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_SilenceInProducesSilenceOut
 #error "You need to define the JucePlugin_SilenceInProducesSilenceOut value in your JucePluginCharacteristics.h file!"
#endif

#ifndef JucePlugin_EditorRequiresKeyboardFocus
 #error "You need to define the JucePlugin_EditorRequiresKeyboardFocus value in your JucePluginCharacteristics.h file!"
#endif

#if JUCE_USE_VSTSDK_2_4 != 0 && JUCE_USE_VSTSDK_2_4 != 1
 #error "You need to define the JUCE_USE_VSTSDK_2_4 value in your JucePluginCharacteristics.h file!"
#endif

#endif   // __JUCE_INCLUDECHARACTERISTICS_JUCEHEADER__
