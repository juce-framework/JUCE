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

// The following checks should cause a compile error if you've forgotten to
// define all your plugin settings properly..

#if ! (JucePlugin_Build_VST || JucePlugin_Build_VST3 \
        || JucePlugin_Build_AU || JucePlugin_Build_RTAS || JucePlugin_Build_AAX \
        || JucePlugin_Build_Standalone || JucePlugin_Build_LV2)
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

#ifdef JucePlugin_Latency
 #error "JucePlugin_Latency is now deprecated - instead, call the AudioProcessor::setLatencySamples() method if your plugin has a non-zero delay"
#endif

#ifndef JucePlugin_SilenceInProducesSilenceOut
 #error "You need to define the JucePlugin_SilenceInProducesSilenceOut value!"
#endif

#ifndef JucePlugin_EditorRequiresKeyboardFocus
 #error "You need to define the JucePlugin_EditorRequiresKeyboardFocus value!"
#endif

//==============================================================================
#if _WIN64 || (__LP64__ && (defined (__APPLE_CPP__) || defined (__APPLE_CC__)))
 #undef JucePlugin_Build_RTAS
 #define JucePlugin_Build_RTAS 0
#endif

#if ! (defined (_MSC_VER) || defined (__APPLE_CPP__) || defined (__APPLE_CC__))
 #undef JucePlugin_Build_VST3
 #define JucePlugin_Build_VST3 0
#endif

//==============================================================================
#if JucePlugin_Build_RTAS && _MSC_VER && ! defined (JucePlugin_WinBag_path)
 #error "You need to define the JucePlugin_WinBag_path value!"
#endif

#if JucePlugin_Build_LV2 && ! defined (JucePlugin_LV2URI)
 #error "You need to define the JucePlugin_LV2URI value!"
#endif

#if JucePlugin_Build_AAX && ! defined (JucePlugin_AAXIdentifier)
 #error "You need to define the JucePlugin_AAXIdentifier value!"
#endif

#if defined (__ppc__)
 #undef JucePlugin_Build_AAX
 #define JucePlugin_Build_AAX 0
#endif
