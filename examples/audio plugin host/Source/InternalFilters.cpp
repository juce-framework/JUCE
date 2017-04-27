/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
