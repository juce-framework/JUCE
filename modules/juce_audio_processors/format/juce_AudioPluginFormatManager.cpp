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
        jassert (dynamic_cast <VSTPluginFormat*> (formats[i]) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_AU && JUCE_MAC
        jassert (dynamic_cast <AudioUnitPluginFormat*> (formats[i]) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_DX && JUCE_WINDOWS
        jassert (dynamic_cast <DirectXPluginFormat*> (formats[i]) == nullptr);
       #endif

       #if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX
        jassert (dynamic_cast <LADSPAPluginFormat*> (formats[i]) == nullptr);
       #endif
    }
   #endif

   #if JUCE_PLUGINHOST_AU && JUCE_MAC
    formats.add (new AudioUnitPluginFormat());
   #endif

   #if JUCE_PLUGINHOST_VST
    formats.add (new VSTPluginFormat());
   #endif

   #if JUCE_PLUGINHOST_DX && JUCE_WINDOWS
    formats.add (new DirectXPluginFormat());
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

AudioPluginInstance* AudioPluginFormatManager::createPluginInstance (const PluginDescription& description,
                                                                     String& errorMessage) const
{
    AudioPluginInstance* result = nullptr;

    for (int i = 0; i < formats.size(); ++i)
    {
        result = formats.getUnchecked(i)->createInstanceFromDescription (description);

        if (result != nullptr)
            break;
    }

    if (result == nullptr)
    {
        if (! doesPluginStillExist (description))
            errorMessage = TRANS ("This plug-in file no longer exists");
        else
            errorMessage = TRANS ("This plug-in failed to load correctly");
    }

    return result;
}

bool AudioPluginFormatManager::doesPluginStillExist (const PluginDescription& description) const
{
    for (int i = 0; i < formats.size(); ++i)
        if (formats.getUnchecked(i)->getName() == description.pluginFormatName)
            return formats.getUnchecked(i)->doesPluginStillExist (description);

    return false;
}
