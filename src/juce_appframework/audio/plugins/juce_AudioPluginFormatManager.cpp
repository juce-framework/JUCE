/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioPluginFormatManager.h"
#include "juce_PluginDescription.h"
#include "../../../juce_core/text/juce_LocalisedStrings.h"

#include "formats/juce_VSTPluginFormat.h"
#include "formats/juce_AudioUnitPluginFormat.h"
#include "formats/juce_DirectXPluginFormat.h"
#include "formats/juce_LADSPAPluginFormat.h"


//==============================================================================
AudioPluginFormatManager::AudioPluginFormatManager() throw()
{
}

AudioPluginFormatManager::~AudioPluginFormatManager() throw()
{
}

juce_ImplementSingleton_SingleThreaded (AudioPluginFormatManager);

//==============================================================================
void AudioPluginFormatManager::addDefaultFormats()
{
#ifdef JUCE_DEBUG
    // you should only call this method once!
    for (int i = formats.size(); --i >= 0;)
    {
  #if JUCE_PLUGINHOST_VST
        jassert (dynamic_cast <VSTPluginFormat*> (formats[i]) == 0);
  #endif

  #if JUCE_PLUGINHOST_AU && JUCE_MAC
        jassert (dynamic_cast <AudioUnitPluginFormat*> (formats[i]) == 0);
  #endif

  #if JUCE_PLUGINHOST_DX && JUCE_WIN32
        jassert (dynamic_cast <DirectXPluginFormat*> (formats[i]) == 0);
  #endif

  #if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX
        jassert (dynamic_cast <LADSPAPluginFormat*> (formats[i]) == 0);
  #endif
    }
#endif

#if JUCE_PLUGINHOST_AU && JUCE_MAC
    formats.add (new AudioUnitPluginFormat());
#endif

#if JUCE_PLUGINHOST_VST
    formats.add (new VSTPluginFormat());
#endif

#if JUCE_PLUGINHOST_DX && JUCE_WIN32
    formats.add (new DirectXPluginFormat());
#endif

#if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX
    formats.add (new LADSPAPluginFormat());
#endif
}

int AudioPluginFormatManager::getNumFormats() throw()
{
    return formats.size();
}

AudioPluginFormat* AudioPluginFormatManager::getFormat (const int index) throw()
{
    return formats [index];
}

void AudioPluginFormatManager::addFormat (AudioPluginFormat* const format) throw()
{
    formats.add (format);
}

AudioPluginInstance* AudioPluginFormatManager::createPluginInstance (const PluginDescription& description,
                                                                     String& errorMessage) const
{
    AudioPluginInstance* result = 0;

    for (int i = 0; i < formats.size(); ++i)
    {
        result = formats.getUnchecked(i)->createInstanceFromDescription (description);

        if (result != 0)
            break;
    }

    if (result == 0)
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


END_JUCE_NAMESPACE
