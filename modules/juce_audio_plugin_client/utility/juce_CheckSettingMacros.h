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

// The following checks should cause a compile error if you've forgotten to
// define all your plugin settings properly..

#if ! (JucePlugin_Build_VST || JucePlugin_Build_AU || JucePlugin_Build_RTAS || JucePlugin_Build_Standalone)
 #error "You need to enable at least one plugin format!"
#endif

#ifndef JucePlugin_IsSynth
 #error "You need to define the JucePlugin_IsSynth value!"
#endif

#ifndef JucePlugin_ManufacturerCode
 #error "You need to define the JucePlugin_ManufacturerCode value!"
#endif

#ifndef JucePlugin_PluginCode
 #error "You need to define the JucePlugin_PluginCode value!"
#endif

#ifndef JucePlugin_ProducesMidiOutput
 #error "You need to define the JucePlugin_ProducesMidiOutput value!"
#endif

#ifndef JucePlugin_WantsMidiInput
 #error "You need to define the JucePlugin_WantsMidiInput value!"
#endif

#ifndef JucePlugin_MaxNumInputChannels
 #error "You need to define the JucePlugin_MaxNumInputChannels value!"
#endif

#ifndef JucePlugin_MaxNumOutputChannels
 #error "You need to define the JucePlugin_MaxNumOutputChannels value!"
#endif

#ifndef JucePlugin_PreferredChannelConfigurations
 #error "You need to define the JucePlugin_PreferredChannelConfigurations value!"
#endif

#ifdef JucePlugin_Latency
 #error "JucePlugin_Latency is now deprecated - instead, call the AudioProcessor::setLatencySamples() method if your plugin has a non-zero delay"
#endif

#ifndef JucePlugin_SilenceInProducesSilenceOut
 #error "You need to define the JucePlugin_SilenceInProducesSilenceOut value!"
#endif

#ifndef JucePlugin_EditorRequiresKeyboardFocus
 #error "You need to define the JucePlugin_EditorRequiresKeyboardFocus value!"
#endif

#ifndef JucePlugin_TailLengthSeconds
 #error "You need to define the JucePlugin_TailLengthSeconds value!"
#endif

//==============================================================================
#if __LP64__ && (defined(__APPLE_CPP__) || defined(__APPLE_CC__))  // (disable VSTs and RTAS in a 64-bit mac build)
 #undef JucePlugin_Build_RTAS
#endif

#if _WIN64    // (disable RTAS in a 64-bit windows build)
 #undef JucePlugin_Build_RTAS
#endif

//==============================================================================
#if JucePlugin_Build_RTAS && _MSC_VER && ! defined (JucePlugin_WinBag_path)
 #error "You need to define the JucePlugin_WinBag_path value!"
#endif

#if JucePlugin_Build_AU && ! defined (JucePlugin_AUCocoaViewClassName)
 #error "You need to define the JucePlugin_AUCocoaViewClassName value!"
#endif

#if (defined(__APPLE_CPP__) || defined(__APPLE_CC__)) && ! defined (JUCE_ObjCExtraSuffix)
 #error "To avoid objective-C name clashes with other plugins, you need to define the JUCE_ObjCExtraSuffix value as a global definition for your project!"
#endif
