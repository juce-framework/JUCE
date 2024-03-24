/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

// The following checks should cause a compile error if you've forgotten to
// define all your plugin settings properly..

#if ! (JucePlugin_Build_VST || JucePlugin_Build_VST3 \
        || JucePlugin_Build_AU  || JucePlugin_Build_AUv3 \
        || JucePlugin_Build_AAX || JucePlugin_Build_Standalone \
        || JucePlugin_Build_LV2 || JucePlugin_Build_Unity)
 #error "You need to enable at least one plugin format!"
#endif

#ifdef JUCE_CHECKSETTINGMACROS_H
 #error "This header should never be included twice! Otherwise something is wrong."
#endif
#define JUCE_CHECKSETTINGMACROS_H

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

#ifndef JucePlugin_EditorRequiresKeyboardFocus
 #error "You need to define the JucePlugin_EditorRequiresKeyboardFocus value!"
#endif

//==============================================================================
#if JucePlugin_Build_AAX && ! defined (JucePlugin_AAXIdentifier)
 #error "You need to define the JucePlugin_AAXIdentifier value!"
#endif

#if defined (__ppc__)
 #undef JucePlugin_Build_AAX
 #define JucePlugin_Build_AAX 0
#endif
