/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AudioIODeviceType::AudioIODeviceType (const String& name)
    : typeName (name)
{
}

AudioIODeviceType::~AudioIODeviceType()
{
}

//==============================================================================
void AudioIODeviceType::addListener (Listener* l)      { listeners.add (l); }
void AudioIODeviceType::removeListener (Listener* l)   { listeners.remove (l); }

void AudioIODeviceType::callDeviceChangeListeners()
{
    listeners.call ([] (Listener& l) { l.audioDeviceListChanged(); });
}

//==============================================================================
#if JUCE_MAC
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_CoreAudio()  { return new CoreAudioClasses::CoreAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_CoreAudio()  { return nullptr; }
#endif

#if JUCE_IOS
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_iOSAudio()   { return new iOSAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_iOSAudio()   { return nullptr; }
#endif

#if JUCE_WINDOWS && JUCE_WASAPI
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode deviceMode)
 {
     auto windowsVersion = SystemStats::getOperatingSystemType();

     if (windowsVersion < SystemStats::WinVista
         || (WasapiClasses::isLowLatencyMode (deviceMode) && windowsVersion < SystemStats::Windows10))
         return nullptr;

     return new WasapiClasses::WASAPIAudioIODeviceType (deviceMode);
 }

 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (bool exclusiveMode)
 {
     return createAudioIODeviceType_WASAPI (exclusiveMode ? WASAPIDeviceMode::exclusive
                                                          : WASAPIDeviceMode::shared);
 }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (WASAPIDeviceMode)  { return nullptr; }
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (bool)              { return nullptr; }
#endif

#if JUCE_WINDOWS && JUCE_DIRECTSOUND
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_DirectSound()  { return new DSoundAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_DirectSound()  { return nullptr; }
#endif

#if JUCE_WINDOWS && JUCE_ASIO
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ASIO()         { return new ASIOAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ASIO()         { return nullptr; }
#endif

#if (JUCE_LINUX || JUCE_BSD) && JUCE_ALSA
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ALSA()         { return createAudioIODeviceType_ALSA_PCMDevices(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ALSA()         { return nullptr; }
#endif

#if (JUCE_LINUX || JUCE_BSD) && JUCE_JACK
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_JACK()         { return new JackAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_JACK()         { return nullptr; }
#endif

#if JUCE_LINUX && JUCE_BELA
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Bela()         { return new BelaAudioIODeviceType(); }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Bela()         { return nullptr; }
#endif

#if JUCE_ANDROID
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Android()
 {
    #if JUCE_USE_ANDROID_OBOE
     if (isOboeAvailable())
         return nullptr;
    #endif

    #if JUCE_USE_ANDROID_OPENSLES
     if (isOpenSLAvailable())
         return nullptr;
    #endif

     return new AndroidAudioIODeviceType();
 }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Android()   { return nullptr; }
#endif

#if JUCE_ANDROID && JUCE_USE_ANDROID_OPENSLES
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_OpenSLES()
 {
     return isOpenSLAvailable() ? new OpenSLAudioDeviceType() : nullptr;
 }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_OpenSLES()  { return nullptr; }
#endif

#if JUCE_ANDROID && JUCE_USE_ANDROID_OBOE
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Oboe()
 {
     return isOboeAvailable() ? new OboeAudioIODeviceType() : nullptr;
 }
#else
 AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Oboe()      { return nullptr; }
#endif

} // namespace juce
