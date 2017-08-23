/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wparentheses"
 #pragma clang diagnostic ignored "-Wextra-tokens"
 #pragma clang diagnostic ignored "-Wcomment"
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Wunused-parameter"
 #pragma clang diagnostic ignored "-Wunused"
 #pragma clang diagnostic ignored "-Wextra-semi"
#endif

// From MacOS 10.13 and iOS 11 Apple has (sensibly!) stopped defining a whole
// set of functions with rather generic names. However, we still need a couple
// of them to compile the files below.
#ifndef verify
 #define verify(assertion) __Verify(assertion)
#endif
#ifndef verify_noerr
 #define verify_noerr(errorCode)  __Verify_noErr(errorCode)
#endif

#include "AU/CoreAudioUtilityClasses/AUBase.cpp"
#include "AU/CoreAudioUtilityClasses/AUBuffer.cpp"
#include "AU/CoreAudioUtilityClasses/AUCarbonViewBase.cpp"
#include "AU/CoreAudioUtilityClasses/AUCarbonViewControl.cpp"
#include "AU/CoreAudioUtilityClasses/AUCarbonViewDispatch.cpp"
#include "AU/CoreAudioUtilityClasses/AUDispatch.cpp"
#include "AU/CoreAudioUtilityClasses/AUInputElement.cpp"
#include "AU/CoreAudioUtilityClasses/AUMIDIBase.cpp"
#include "AU/CoreAudioUtilityClasses/AUOutputBase.cpp"
#include "AU/CoreAudioUtilityClasses/AUOutputElement.cpp"
#include "AU/CoreAudioUtilityClasses/AUScopeElement.cpp"
#include "AU/CoreAudioUtilityClasses/CAAUParameter.cpp"
#include "AU/CoreAudioUtilityClasses/CAAudioChannelLayout.cpp"
#include "AU/CoreAudioUtilityClasses/CAMutex.cpp"
#include "AU/CoreAudioUtilityClasses/CAStreamBasicDescription.cpp"
#include "AU/CoreAudioUtilityClasses/CAVectorUnit.cpp"
#include "AU/CoreAudioUtilityClasses/CarbonEventHandler.cpp"
#include "AU/CoreAudioUtilityClasses/ComponentBase.cpp"
#include "AU/CoreAudioUtilityClasses/MusicDeviceBase.cpp"

#undef verify
#undef verify_noerr

#ifdef __clang__
 #pragma clang diagnostic pop
#endif
