/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

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
    listeners.call (&AudioIODeviceType::Listener::audioDeviceListChanged);
}

//==============================================================================
#if ! JUCE_MAC
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_CoreAudio()       { return nullptr; }
#endif

#if ! JUCE_IOS
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_iOSAudio()        { return nullptr; }
#endif

#if ! (JUCE_WINDOWS && JUCE_WASAPI)
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI (bool)     { return nullptr; }
#endif

#if ! (JUCE_WINDOWS && JUCE_DIRECTSOUND)
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_DirectSound()     { return nullptr; }
#endif

#if ! (JUCE_WINDOWS && JUCE_ASIO)
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ASIO()            { return nullptr; }
#endif

#if ! (JUCE_LINUX && JUCE_ALSA)
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_ALSA()            { return nullptr; }
#endif

#if ! (JUCE_LINUX && JUCE_JACK)
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_JACK()            { return nullptr; }
#endif

#if ! JUCE_ANDROID
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Android()         { return nullptr; }
#endif

#if ! (JUCE_ANDROID && JUCE_USE_ANDROID_OPENSLES)
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_OpenSLES()        { return nullptr; }
#endif
