/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if JucePlugin_Build_AU

#include <juce_core/system/juce_CompilerWarnings.h>

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wparentheses",
                                     "-Wextra-tokens",
                                     "-Wcomment",
                                     "-Wconversion",
                                     "-Wunused-parameter",
                                     "-Wunused",
                                     "-Wextra-semi",
                                     "-Wformat-pedantic",
                                     "-Wgnu-zero-variadic-macro-arguments",
                                     "-Wshadow-all",
                                     "-Wcast-align",
                                     "-Wswitch-enum",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wnullable-to-nonnull-conversion",
                                     "-Wignored-qualifiers",
                                     "-Wfour-char-constants")

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

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif
