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

#include "../JuceLibraryCode/JuceHeader.h"
#include "InternalFilters.h"
#include "FilterGraph.h"


//==============================================================================
InternalPluginFormat::InternalPluginFormat()
{
    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
        p.fillInPluginDescription (audioOutDesc);
    }

    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);
        p.fillInPluginDescription (audioInDesc);
    }

    {
        AudioProcessorGraph::AudioGraphIOProcessor p (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
        p.fillInPluginDescription (midiInDesc);
    }
}

void InternalPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                 double /*initialSampleRate*/,
                                                 int /*initialBufferSize*/,
                                                 void* userData,
                                                 void (*callback) (void*, AudioPluginInstance*, const String&))
{
    AudioPluginInstance* retval = nullptr;
    if (desc.name == audioOutDesc.name)
        retval = new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

    if (desc.name == audioInDesc.name)
        retval = new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode);

    if (desc.name == midiInDesc.name)
        retval = new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);

    callback (userData, retval, retval == nullptr ? NEEDS_TRANS ("Invalid internal filter name") : String());
}

bool InternalPluginFormat::requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const noexcept
{
    return false;
}

const PluginDescription* InternalPluginFormat::getDescriptionFor (const InternalFilterType type)
{
    switch (type)
    {
        case audioInputFilter:      return &audioInDesc;
        case audioOutputFilter:     return &audioOutDesc;
        case midiInputFilter:       return &midiInDesc;
        default:                    break;
    }

    return 0;
}

void InternalPluginFormat::getAllTypes (OwnedArray <PluginDescription>& results)
{
    for (int i = 0; i < (int) endOfFilterTypes; ++i)
        results.add (new PluginDescription (*getDescriptionFor ((InternalFilterType) i)));
}
