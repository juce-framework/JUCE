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

#include "juce_AudioFilterStreamer.h"
#include "../../juce_IncludeCharacteristics.h"


//==============================================================================
AudioFilterStreamer::AudioFilterStreamer (AudioFilterBase& filterToUse)
    : filter (filterToUse),
      isPlaying (false),
      sampleRate (0),
      emptyBuffer (1, 32)
{
    filter.numInputChannels = JucePlugin_MaxNumInputChannels;
    filter.numOutputChannels = JucePlugin_MaxNumOutputChannels;

    filter.initialiseInternal (this);
}

AudioFilterStreamer::~AudioFilterStreamer()
{
    audioDeviceStopped();
}

void AudioFilterStreamer::audioDeviceIOCallback (const float** inputChannelData,
                                                 int totalNumInputChannels,
                                                 float** outputChannelData,
                                                 int totalNumOutputChannels,
                                                 int numSamples)
{
    MidiBuffer midiBuffer;
    midiCollector.removeNextBlockOfMessages (midiBuffer, numSamples);

    int i, numActiveInChans = 0, numActiveOutChans = 0;
    int numOutsWanted = filter.getNumOutputChannels();
    const int numInsWanted = filter.getNumInputChannels();

    for (i = 0; i < totalNumInputChannels; ++i)
        if (inputChannelData[i] != 0)
            inChans [numActiveInChans++] = (float*) inputChannelData[i];

    while (numActiveInChans < numInsWanted)
        inChans [numActiveInChans++] = emptyBuffer.getSampleData (0, 0);

    for (i = 0; i < totalNumOutputChannels; ++i)
        if (outputChannelData[i] != 0)
            outChans [numActiveOutChans++] = outputChannelData[i];

    i = 0;
    while (numActiveOutChans < numOutsWanted)
        outChans [numActiveOutChans++] = emptyBuffer.getSampleData (++i, 0);

    AudioSampleBuffer input (inChans, jmin (numInsWanted, numActiveInChans), numSamples);
    AudioSampleBuffer output (outChans, jmin (numOutsWanted, numActiveOutChans), numSamples);

    {
        const ScopedLock sl (filter.getCallbackLock());

        if (filter.suspended)
            output.clear();
        else
            filter.processBlock (input, output, false, midiBuffer);
    }

    while (numOutsWanted < numActiveOutChans)
        zeromem (outChans[numOutsWanted++], sizeof (float) * numSamples);
}

void AudioFilterStreamer::audioDeviceAboutToStart (double sampleRate_,
                                                   int numSamplesPerBlock)
{
    sampleRate = sampleRate_;

    isPlaying = true;

    emptyBuffer.setSize (1 + filter.getNumOutputChannels(),
                         jmax (2048, numSamplesPerBlock * 2));
    emptyBuffer.clear();

    midiCollector.reset (sampleRate);

    filter.prepareToPlay (sampleRate, numSamplesPerBlock);
}

void AudioFilterStreamer::audioDeviceStopped()
{
    isPlaying = false;
    filter.releaseResources();
    midiCollector.reset (sampleRate);
    emptyBuffer.setSize (1, 32);
}

void AudioFilterStreamer::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message)
{
#if JucePlugin_WantsMidiInput
    midiCollector.addMessageToQueue (message);
#endif
}

bool AudioFilterStreamer::getCurrentPositionInfo (AudioFilterBase::CurrentPositionInfo& info)
{
    return false;
}

void AudioFilterStreamer::informHostOfParameterChange (int index, float newValue)
{
    filter.setParameter (index, newValue);
}



//==============================================================================
AudioFilterStreamingDeviceManager::AudioFilterStreamingDeviceManager()
    : streamer (0)
{
}

AudioFilterStreamingDeviceManager::~AudioFilterStreamingDeviceManager()
{
    setFilter (0);
    clearSingletonInstance();
}

void AudioFilterStreamingDeviceManager::setFilter (AudioFilterBase* filterToStream)
{
    if (streamer != 0)
    {
        removeMidiInputCallback (streamer);
        setAudioCallback (0);

        delete streamer;
        streamer = 0;
    }

    if (filterToStream != 0)
    {
        streamer = new AudioFilterStreamer (*filterToStream);

        setAudioCallback (streamer);
        addMidiInputCallback (String::empty, streamer);
    }
}

juce_ImplementSingleton (AudioFilterStreamingDeviceManager);
