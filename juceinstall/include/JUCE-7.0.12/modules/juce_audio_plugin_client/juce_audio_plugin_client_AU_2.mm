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

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wambiguous-reversed-operator",
                                     "-Wc99-extensions",
                                     "-Wcast-align",
                                     "-Wcomment",
                                     "-Wconversion",
                                     "-Wdeprecated-anon-enum-enum-conversion",
                                     "-Wextra-semi",
                                     "-Wextra-tokens",
                                     "-Wfloat-equal",
                                     "-Wformat-pedantic",
                                     "-Wfour-char-constants",
                                     "-Wgnu-zero-variadic-macro-arguments",
                                     "-Wignored-qualifiers",
                                     "-Wimplicit-fallthrough",
                                     "-Wmissing-prototypes",
                                     "-Wnullable-to-nonnull-conversion",
                                     "-Wparentheses",
                                     "-Wshadow-all",
                                     "-Wswitch-enum",
                                     "-Wunknown-attributes",
                                     "-Wunused",
                                     "-Wunused-parameter",
                                     "-Wzero-as-null-pointer-constant")

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

#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUBase.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUBuffer.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUBufferAllocator.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUEffectBase.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUInputElement.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUMIDIBase.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUMIDIEffectBase.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUOutputElement.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUPlugInDispatch.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/AUScopeElement.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/ComponentBase.cpp>
#include <juce_audio_plugin_client/AU/AudioUnitSDK/MusicDeviceBase.cpp>

#undef verify
#undef verify_noerr

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif
