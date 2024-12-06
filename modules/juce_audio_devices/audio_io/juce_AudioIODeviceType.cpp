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

#if (JUCE_LINUX || JUCE_BSD || JUCE_MAC || JUCE_WINDOWS) && JUCE_JACK
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
