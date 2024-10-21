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
