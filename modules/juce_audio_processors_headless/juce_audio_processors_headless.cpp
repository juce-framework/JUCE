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

#ifdef JUCE_AUDIO_PROCESSORS_HEADLESS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1

#include "juce_audio_processors_headless.h"

#include <juce_audio_processors_headless/processors/juce_AudioProcessorListener.cpp>
#include <juce_audio_processors_headless/utilities/juce_AAXClientExtensions.cpp>
#include <juce_audio_processors_headless/utilities/juce_VST2ClientExtensions.cpp>
#include <juce_audio_processors_headless/utilities/juce_VST3ClientExtensions.cpp>
#include <juce_audio_processors_headless/processors/juce_AudioProcessorParameter.cpp>
#include <juce_audio_processors_headless/processors/juce_AudioProcessorParameterGroup.cpp>
#include <juce_audio_processors_headless/processors/juce_AudioProcessor.cpp>
#include <juce_audio_processors_headless/processors/juce_PluginDescription.cpp>
#include <juce_audio_processors_headless/processors/juce_AudioPluginInstance.cpp>
#include <juce_audio_processors_headless/processors/juce_AudioProcessorGraph.cpp>
#include <juce_audio_processors_headless/format/juce_AudioPluginFormat.cpp>
#include <juce_audio_processors_headless/utilities/juce_AudioProcessorParameterWithID.cpp>
#include <juce_audio_processors_headless/utilities/juce_RangedAudioParameter.cpp>
#include <juce_audio_processors_headless/utilities/juce_AudioParameterFloat.cpp>
#include <juce_audio_processors_headless/utilities/juce_AudioParameterInt.cpp>
#include <juce_audio_processors_headless/utilities/juce_AudioParameterBool.cpp>
#include <juce_audio_processors_headless/utilities/juce_AudioParameterChoice.cpp>
#include <juce_audio_processors_headless/utilities/ARA/juce_ARA_utils.cpp>
#include <juce_audio_processors_headless/format_types/juce_AudioUnitPluginFormatHeadless.mm>
#include <juce_audio_processors_headless/format_types/juce_LADSPAPluginFormatHeadless.cpp>
#include <juce_audio_processors_headless/format_types/juce_LV2PluginFormatHeadless.cpp>
#include <juce_audio_processors_headless/format_types/juce_VST3PluginFormatHeadless.cpp>
#include <juce_audio_processors_headless/format_types/juce_VSTPluginFormatHeadless.cpp>
#include <juce_audio_processors_headless/format_types/juce_ARAHosting.cpp>
#include <juce_audio_processors_headless/format/juce_AudioPluginFormatManager.cpp>
