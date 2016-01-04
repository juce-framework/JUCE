/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

AudioPluginFormatManager::AudioPluginFormatManager() {}
AudioPluginFormatManager::~AudioPluginFormatManager() {}

//==============================================================================
void AudioPluginFormatManager::addDefaultFormats()
{
   #if JUCE_DEBUG
    // you should only call this method once!
    for (int i = formats.size(); --i >= 0;)
    {
       #if JUCE_PLUGINHOST_VST
        jassert (dynamic_cast<VSTPluginFormat*> (formats[i]) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_VST3
        jassert (dynamic_cast<VST3PluginFormat*> (formats[i]) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_AU && JUCE_MAC
        jassert (dynamic_cast<AudioUnitPluginFormat*> (formats[i]) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX
        jassert (dynamic_cast<LADSPAPluginFormat*> (formats[i]) == nullptr);
       #endif
    }
   #endif

   #if JUCE_PLUGINHOST_AU && JUCE_MAC
    formats.add (new AudioUnitPluginFormat());
   #endif

   #if JUCE_PLUGINHOST_VST
    formats.add (new VSTPluginFormat());
   #endif

   #if JUCE_PLUGINHOST_VST3
    formats.add (new VST3PluginFormat());
   #endif

   #if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX
    formats.add (new LADSPAPluginFormat());
   #endif
}

int AudioPluginFormatManager::getNumFormats()
{
    return formats.size();
}

AudioPluginFormat* AudioPluginFormatManager::getFormat (const int index)
{
    return formats [index];
}

void AudioPluginFormatManager::addFormat (AudioPluginFormat* const format)
{
    formats.add (format);
}

AudioPluginInstance* AudioPluginFormatManager::createPluginInstance (const PluginDescription& description, double rate,
                                                                     int blockSize, String& errorMessage) const
{
    for (int i = 0; i < formats.size(); ++i)
        if (AudioPluginInstance* result = formats.getUnchecked(i)->createInstanceFromDescription (description, rate, blockSize))
            return result;

    errorMessage = doesPluginStillExist (description) ? TRANS ("This plug-in failed to load correctly")
                                                      : TRANS ("This plug-in file no longer exists");
    return nullptr;
}

bool AudioPluginFormatManager::doesPluginStillExist (const PluginDescription& description) const
{
    for (int i = 0; i < formats.size(); ++i)
        if (formats.getUnchecked(i)->getName() == description.pluginFormatName)
            return formats.getUnchecked(i)->doesPluginStillExist (description);

    return false;
}
