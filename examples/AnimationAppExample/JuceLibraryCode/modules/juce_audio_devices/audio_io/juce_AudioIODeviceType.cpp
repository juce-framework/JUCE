/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
