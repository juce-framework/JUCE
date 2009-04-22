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
#include "../juce_IncludeCharacteristics.h"


//==============================================================================
AudioFilterStreamer::AudioFilterStreamer (AudioProcessor& filterToUse)
    : filter (filterToUse),
      isPlaying (false),
      sampleRate (0),
      emptyBuffer (1, 32)
{
    filter.setPlayConfigDetails (JucePlugin_MaxNumInputChannels, JucePlugin_MaxNumOutputChannels, 0, 0);

    filter.setPlayHead (this);
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

        if (filter.isSuspended())
        {
            output.clear();
        }
        else
        {
            for (int i = jmin (output.getNumChannels(), input.getNumChannels()); --i >= 0;)
                output.copyFrom (i, 0, input, i, 0, numSamples);

            filter.processBlock (output, midiBuffer);
        }
    }

    while (numOutsWanted < numActiveOutChans)
        zeromem (outChans[numOutsWanted++], sizeof (float) * numSamples);
}

void AudioFilterStreamer::audioDeviceAboutToStart (AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();

    isPlaying = true;

    emptyBuffer.setSize (1 + filter.getNumOutputChannels(),
                         jmax (2048, device->getCurrentBufferSizeSamples() * 2));
    emptyBuffer.clear();

    midiCollector.reset (sampleRate);

    filter.prepareToPlay (device->getCurrentSampleRate(),
                          device->getCurrentBufferSizeSamples());
}

void AudioFilterStreamer::audioDeviceStopped()
{
    isPlaying = false;
    filter.releaseResources();
    midiCollector.reset (sampleRate > 0 ? sampleRate : 44100.0);
    emptyBuffer.setSize (1, 32);
}

void AudioFilterStreamer::handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message)
{
#if JucePlugin_WantsMidiInput
    midiCollector.addMessageToQueue (message);
#endif
}

bool AudioFilterStreamer::getCurrentPosition (AudioPlayHead::CurrentPositionInfo& info)
{
    return false;
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

void AudioFilterStreamingDeviceManager::setFilter (AudioProcessor* filterToStream)
{
    if (streamer != 0)
    {
        removeMidiInputCallback (String::empty, streamer);
        removeAudioCallback (streamer);

        delete streamer;
        streamer = 0;
    }

    if (filterToStream != 0)
    {
        streamer = new AudioFilterStreamer (*filterToStream);

        addAudioCallback (streamer);
        addMidiInputCallback (String::empty, streamer);
    }
}

juce_ImplementSingleton (AudioFilterStreamingDeviceManager);
