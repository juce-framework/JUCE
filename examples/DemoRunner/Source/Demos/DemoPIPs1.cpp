/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../../../Assets/DemoUtilities.h"
#include "JUCEDemos.h"

#include "../../../Assets/AudioLiveScrollingDisplay.h"

//==============================================================================
#include "IntroScreen.h"
#include "../../../Audio/AudioAppDemo.h"
#include "../../../Audio/AudioLatencyDemo.h"
#include "../../../Audio/AudioPlaybackDemo.h"
#include "../../../Audio/AudioRecordingDemo.h"
#include "../../../Audio/AudioSettingsDemo.h"
#include "../../../Audio/AudioSynthesiserDemo.h"
#include "../../../Audio/AudioWorkgroupDemo.h"
#include "../../../Audio/CapabilityInquiryDemo.h"
#include "../../../Audio/MidiDemo.h"
#include "../../../Audio/MPEDemo.h"
#include "../../../Audio/PluckedStringsDemo.h"
#include "../../../Audio/SimpleFFTDemo.h"

#include "../../../DSP/ConvolutionDemo.h"
#include "../../../DSP/FIRFilterDemo.h"
#include "../../../DSP/GainDemo.h"
#include "../../../DSP/IIRFilterDemo.h"
#include "../../../DSP/OscillatorDemo.h"
#include "../../../DSP/OverdriveDemo.h"
#if JUCE_USE_SIMD
 #include "../../../DSP/SIMDRegisterDemo.h"
#endif
#include "../../../DSP/StateVariableFilterDemo.h"
#include "../../../DSP/WaveShaperTanhDemo.h"

#include "../../../Utilities/Box2DDemo.h"
#include "../../../Utilities/CryptographyDemo.h"
#include "../../../Utilities/JavaScriptDemo.h"
#include "../../../Utilities/LiveConstantDemo.h"
#include "../../../Utilities/MultithreadingDemo.h"
#include "../../../Utilities/NetworkingDemo.h"
#include "../../../Utilities/OSCDemo.h"
#include "../../../Utilities/SystemInfoDemo.h"
#include "../../../Utilities/TimersAndEventsDemo.h"
#include "../../../Utilities/UnitTestsDemo.h"
#include "../../../Utilities/ValueTreesDemo.h"
#include "../../../Utilities/XMLandJSONDemo.h"

void registerDemos_One() noexcept
{
    REGISTER_DEMO (AudioAppDemo,            Audio,     false)
    REGISTER_DEMO (AudioLatencyDemo,        Audio,     false)
    REGISTER_DEMO (AudioPlaybackDemo,       Audio,     false)
    REGISTER_DEMO (AudioRecordingDemo,      Audio,     false)
    REGISTER_DEMO (AudioSettingsDemo,       Audio,     false)
    REGISTER_DEMO (AudioSynthesiserDemo,    Audio,     false)
    REGISTER_DEMO (AudioWorkgroupDemo,      Audio,     false)
    REGISTER_DEMO (CapabilityInquiryDemo,   Audio,     false)
    REGISTER_DEMO (MidiDemo,                Audio,     false)
    REGISTER_DEMO (MPEDemo,                 Audio,     false)
    REGISTER_DEMO (PluckedStringsDemo,      Audio,     false)
    REGISTER_DEMO (SimpleFFTDemo,           Audio,     false)

    REGISTER_DEMO (ConvolutionDemo,         DSP,       false)
    REGISTER_DEMO (FIRFilterDemo,           DSP,       false)
    REGISTER_DEMO (GainDemo,                DSP,       false)
    REGISTER_DEMO (IIRFilterDemo,           DSP,       false)
    REGISTER_DEMO (OscillatorDemo,          DSP,       false)
    REGISTER_DEMO (OverdriveDemo,           DSP,       false)
   #if JUCE_USE_SIMD
    REGISTER_DEMO (SIMDRegisterDemo,        DSP,       false)
   #endif
    REGISTER_DEMO (StateVariableFilterDemo, DSP,       false)
    REGISTER_DEMO (WaveShaperTanhDemo,      DSP,       false)

    REGISTER_DEMO (Box2DDemo,               Utilities, false)
   #if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
    REGISTER_DEMO (ChildProcessDemo,        Utilities, false)
   #endif
    REGISTER_DEMO (CryptographyDemo,        Utilities, false)
    REGISTER_DEMO (JavaScriptDemo,          Utilities, false)
    REGISTER_DEMO (LiveConstantDemo,        Utilities, false)
    REGISTER_DEMO (MultithreadingDemo,      Utilities, false)
    REGISTER_DEMO (NetworkingDemo,          Utilities, false)
    REGISTER_DEMO (OSCDemo,                 Utilities, false)
    REGISTER_DEMO (SystemInfoDemo,          Utilities, false)
    REGISTER_DEMO (TimersAndEventsDemo,     Utilities, false)
    REGISTER_DEMO (UnitTestsDemo,           Utilities, false)
    REGISTER_DEMO (ValueTreesDemo,          Utilities, false)
    REGISTER_DEMO (XMLandJSONDemo,          Utilities, false)
}

Component* createIntroDemo()
{
    return new IntroScreen();
}

bool isComponentIntroDemo (Component* comp) noexcept
{
    return (dynamic_cast<IntroScreen*> (comp) != nullptr);
}
