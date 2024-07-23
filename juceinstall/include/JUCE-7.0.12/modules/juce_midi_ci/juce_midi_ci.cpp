/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifdef JUCE_MIDI_CI_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "juce_midi_ci.h"

#include <juce_midi_ci/detail/juce_CIMessageMeta.h>
#include <juce_midi_ci/detail/juce_CIMarshalling.h>
#include <juce_midi_ci/detail/juce_CIPropertyDataMessageChunker.h>
#include <juce_midi_ci/detail/juce_CIResponder.h>
#include <juce_midi_ci/detail/juce_CIMessageTypeUtils.h>
#include <juce_midi_ci/detail/juce_CIPropertyHostUtils.h>

#include <juce_midi_ci/detail/juce_CIPropertyDataMessageChunker.cpp>
#include <juce_midi_ci/detail/juce_CIResponder.cpp>

#include <juce_midi_ci/ci/juce_CIDevice.cpp>
#include <juce_midi_ci/ci/juce_CIEncodings.cpp>
#include <juce_midi_ci/ci/juce_CIParser.cpp>
#include <juce_midi_ci/ci/juce_CIProfileHost.cpp>
#include <juce_midi_ci/ci/juce_CIProfileStates.cpp>
#include <juce_midi_ci/ci/juce_CIPropertyDelegate.cpp>
#include <juce_midi_ci/ci/juce_CIPropertyExchangeCache.cpp>
#include <juce_midi_ci/ci/juce_CIPropertyHost.cpp>
#include <juce_midi_ci/ci/juce_CIResponderOutput.cpp>
#include <juce_midi_ci/ci/juce_CISubscriptionManager.cpp>
