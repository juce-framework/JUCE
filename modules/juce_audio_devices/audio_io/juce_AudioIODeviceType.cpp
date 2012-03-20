/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

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
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_WASAPI()          { return nullptr; }
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
