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

#include <juce_core/system/juce_TargetPlatform.h>

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
                                     "-Wimplicit-fallthrough",
                                     "-Wzero-as-null-pointer-constant",
                                     "-Wnullable-to-nonnull-conversion",
                                     "-Wignored-qualifiers",
                                     "-Wfour-char-constants",
                                     "-Wmissing-prototypes",
                                     "-Wdeprecated-anon-enum-enum-conversion",
                                     "-Wambiguous-reversed-operator")

// From MacOS 10.13 and iOS 11 Apple has (sensibly!) stopped defining a whole
// set of functions with rather generic names. However, we still need a couple
// of them to compile the files below.
#ifndef verify
 #define verify(assertion) __Verify(assertion)
#endif
#ifndef verify_noerr
 #define verify_noerr(errorCode)  __Verify_noErr(errorCode)
#endif

#if ! defined (MAC_OS_VERSION_11_0) || MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_VERSION_11_0
// These constants are only defined in the macOS 11+ SDKs

enum MIDICVStatus : unsigned int
{
    kMIDICVStatusNoteOff                = 0x8,
    kMIDICVStatusNoteOn                 = 0x9,
    kMIDICVStatusPolyPressure           = 0xA,
    kMIDICVStatusControlChange          = 0xB,
    kMIDICVStatusProgramChange          = 0xC,
    kMIDICVStatusChannelPressure        = 0xD,
    kMIDICVStatusPitchBend              = 0xE,
    kMIDICVStatusRegisteredPNC          = 0x0,
    kMIDICVStatusAssignablePNC          = 0x1,
    kMIDICVStatusRegisteredControl      = 0x2,
    kMIDICVStatusAssignableControl      = 0x3,
    kMIDICVStatusRelRegisteredControl   = 0x4,
    kMIDICVStatusRelAssignableControl   = 0x5,
    kMIDICVStatusPerNotePitchBend       = 0x6,
    kMIDICVStatusPerNoteMgmt            = 0xF
};

#endif

#include "AU/AudioUnitSDK/AUBase.cpp"
#include "AU/AudioUnitSDK/AUBuffer.cpp"
#include "AU/AudioUnitSDK/AUBufferAllocator.cpp"
#include "AU/AudioUnitSDK/AUEffectBase.cpp"
#include "AU/AudioUnitSDK/AUInputElement.cpp"
#include "AU/AudioUnitSDK/AUMIDIBase.cpp"
#include "AU/AudioUnitSDK/AUMIDIEffectBase.cpp"
#include "AU/AudioUnitSDK/AUOutputElement.cpp"
#include "AU/AudioUnitSDK/AUPlugInDispatch.cpp"
#include "AU/AudioUnitSDK/AUScopeElement.cpp"
#include "AU/AudioUnitSDK/ComponentBase.cpp"
#include "AU/AudioUnitSDK/MusicDeviceBase.cpp"

#undef verify
#undef verify_noerr

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif
