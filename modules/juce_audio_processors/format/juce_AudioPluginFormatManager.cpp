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

namespace juce
{

AudioPluginFormatManager::AudioPluginFormatManager() {}
AudioPluginFormatManager::~AudioPluginFormatManager() {}

//==============================================================================
void AudioPluginFormatManager::addDefaultFormats()
{
   #if JUCE_PLUGINHOST_VST && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD || JUCE_IOS)
    #define HAS_VST 1
   #else
    #define HAS_VST 0
   #endif

   #if JUCE_PLUGINHOST_VST3 && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD)
    #define HAS_VST3 1
   #else
    #define HAS_VST3 0
   #endif

   #if JUCE_PLUGINHOST_AU && (JUCE_MAC || JUCE_IOS)
    #define HAS_AU 1
   #else
    #define HAS_AU 0
   #endif

   #if JUCE_PLUGINHOST_LADSPA && (JUCE_LINUX || JUCE_BSD)
    #define HAS_LADSPA 1
   #else
    #define HAS_LADSPA 0
   #endif

   #if JUCE_PLUGINHOST_LV2 && (JUCE_MAC || JUCE_LINUX || JUCE_BSD || JUCE_WINDOWS)
    #define HAS_LV2 1
   #else
    #define HAS_LV2 0
   #endif

   #if JUCE_DEBUG
    // you should only call this method once!
    for (auto* format : formats)
    {
        ignoreUnused (format);

       #if HAS_VST
        jassert (dynamic_cast<VSTPluginFormat*> (format) == nullptr);
       #endif

       #if HAS_VST3
        jassert (dynamic_cast<VST3PluginFormat*> (format) == nullptr);
       #endif

       #if HAS_AU
        jassert (dynamic_cast<AudioUnitPluginFormat*> (format) == nullptr);
       #endif

       #if HAS_LADSPA
        jassert (dynamic_cast<LADSPAPluginFormat*> (format) == nullptr);
       #endif

       #if HAS_LV2
        jassert (dynamic_cast<LV2PluginFormat*> (format) == nullptr);
       #endif
    }
   #endif

   #if HAS_AU
    formats.add (new AudioUnitPluginFormat());
   #endif

   #if HAS_VST
    formats.add (new VSTPluginFormat());
   #endif

   #if HAS_VST3
    formats.add (new VST3PluginFormat());
   #endif

   #if HAS_LADSPA
    formats.add (new LADSPAPluginFormat());
   #endif

   #if HAS_LV2
    formats.add (new LV2PluginFormat());
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

void AudioPluginFormatManager::createARAFactoryAsync (const PluginDescription& description,
                                                      AudioPluginFormat::ARAFactoryCreationCallback callback) const
{
    String errorMessage;

    if (auto* format = findFormatForDescription (description, errorMessage))
    {
        format->createARAFactoryAsync (description, callback);
    }
    else
    {
        errorMessage = NEEDS_TRANS ("Couldn't find format for the provided description");
        callback ({ {}, std::move (errorMessage) });
    }
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
