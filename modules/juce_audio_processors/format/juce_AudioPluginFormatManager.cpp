/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

AudioPluginFormatManager::AudioPluginFormatManager() {}
AudioPluginFormatManager::~AudioPluginFormatManager() {}

//==============================================================================
void AudioPluginFormatManager::addDefaultFormats()
{
   #if JUCE_DEBUG
    // you should only call this method once!
    for (auto* format : formats)
    {
        ignoreUnused (format);

       #if JUCE_PLUGINHOST_VST && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_IOS)
        jassert (dynamic_cast<VSTPluginFormat*> (format) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_VST3 && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)
        jassert (dynamic_cast<VST3PluginFormat*> (format) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_AU && (JUCE_MAC || JUCE_IOS)
        jassert (dynamic_cast<AudioUnitPluginFormat*> (format) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX
        jassert (dynamic_cast<LADSPAPluginFormat*> (format) == nullptr);
       #endif
    }
   #endif

   #if JUCE_PLUGINHOST_AU && (JUCE_MAC || JUCE_IOS)
    formats.add (new AudioUnitPluginFormat());
   #endif

   #if JUCE_PLUGINHOST_VST && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_IOS)
    formats.add (new VSTPluginFormat());
   #endif

   #if JUCE_PLUGINHOST_VST3 && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)
    formats.add (new VST3PluginFormat());
   #endif

   #if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX
    formats.add (new LADSPAPluginFormat());
   #endif
}

int AudioPluginFormatManager::getNumFormats() const                         { return formats.size(); }
AudioPluginFormat* AudioPluginFormatManager::getFormat (int index) const    { return formats[index]; }

Array<AudioPluginFormat*> AudioPluginFormatManager::getFormats() const
{
    Array<AudioPluginFormat*> a;
    a.addArray (formats);
    return a;
}

void AudioPluginFormatManager::addFormat (AudioPluginFormat* format)
{
    formats.add (format);
}

std::unique_ptr<AudioPluginInstance> AudioPluginFormatManager::createPluginInstance (const PluginDescription& description,
                                                                                     double rate, int blockSize,
                                                                                     String& errorMessage) const
{
    if (auto* format = findFormatForDescription (description, errorMessage))
        return format->createInstanceFromDescription (description, rate, blockSize, errorMessage);

    return {};
}

void AudioPluginFormatManager::createPluginInstanceAsync (const PluginDescription& description,
                                                          double initialSampleRate, int initialBufferSize,
                                                          AudioPluginFormat::PluginCreationCallback callback)
{
    String error;

    if (auto* format = findFormatForDescription (description, error))
        return format->createPluginInstanceAsync (description, initialSampleRate, initialBufferSize, std::move (callback));

    struct DeliverError  : public CallbackMessage
    {
        DeliverError (AudioPluginFormat::PluginCreationCallback c, const String& e)
            : call (std::move (c)), error (e)
        {
            post();
        }

        void messageCallback() override          { call (nullptr, error); }

        AudioPluginFormat::PluginCreationCallback call;
        String error;
    };

    new DeliverError (std::move (callback), error);
}

AudioPluginFormat* AudioPluginFormatManager::findFormatForDescription (const PluginDescription& description,
                                                                       String& errorMessage) const
{
    errorMessage = {};

    for (auto* format : formats)
        if (format->getName() == description.pluginFormatName
              && format->fileMightContainThisPluginType (description.fileOrIdentifier))
            return format;

    errorMessage = NEEDS_TRANS ("No compatible plug-in format exists for this plug-in");

    return {};
}

bool AudioPluginFormatManager::doesPluginStillExist (const PluginDescription& description) const
{
    for (auto* format : formats)
        if (format->getName() == description.pluginFormatName)
            return format->doesPluginStillExist (description);

    return false;
}

} // namespace juce
