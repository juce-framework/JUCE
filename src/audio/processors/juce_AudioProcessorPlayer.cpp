/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioProcessorPlayer.h"
#include "../../threads/juce_ScopedLock.h"


//==============================================================================
AudioProcessorPlayer::AudioProcessorPlayer()
    : processor (0),
      sampleRate (0),
      blockSize (0),
      isPrepared (false),
      numInputChans (0),
      numOutputChans (0),
      tempBuffer (1, 1)
{
}

AudioProcessorPlayer::~AudioProcessorPlayer()
{
    setProcessor (0);
}

//==============================================================================
void AudioProcessorPlayer::setProcessor (AudioProcessor* const processorToPlay)
{
    if (processor != processorToPlay)
    {
        if (processorToPlay != 0 && sampleRate > 0 && blockSize > 0)
        {
            processorToPlay->setPlayConfigDetails (numInputChans, numOutputChans,
                                                   sampleRate, blockSize);

            processorToPlay->prepareToPlay (sampleRate, blockSize);
        }

        lock.enter();
        AudioProcessor* const oldOne = isPrepared ? processor : 0;
        processor = processorToPlay;
        isPrepared = true;
        lock.exit();

        if (oldOne != 0)
            oldOne->releaseResources();
    }
}

//==============================================================================
void AudioProcessorPlayer::audioDeviceIOCallback (const float** inputChannelData,
                                                  int numInputChannels,
                                                  float** outputChannelData,
                                                  int numOutputChannels,
                                                  int numSamples)
{
    // these should have been prepared by audioDeviceAboutToStart()...
    jassert (sampleRate > 0 && blockSize > 0);

    incomingMidi.clear();
    messageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
    int i, totalNumChans = 0;

    if (numInputChannels > numOutputChannels)
    {
        // if there aren't enough output channels for the number of
        // inputs, we need to create some temporary extra ones (can't
        // use the input data in case it gets written to)
        tempBuffer.setSize (numInputChannels - numOutputChannels, numSamples,
                            false, false, true);

        for (i = 0; i < numOutputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * numSamples);
            ++totalNumChans;
        }

        for (i = numOutputChannels; i < numInputChannels; ++i)
        {
            channels[totalNumChans] = tempBuffer.getSampleData (i - numOutputChannels, 0);
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * numSamples);
            ++totalNumChans;
        }
    }
    else
    {
        for (i = 0; i < numInputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * numSamples);
            ++totalNumChans;
        }

        for (i = numInputChannels; i < numOutputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            zeromem (channels[totalNumChans], sizeof (float) * numSamples);
            ++totalNumChans;
        }
    }

    AudioSampleBuffer buffer (channels, totalNumChans, numSamples);

    const ScopedLock sl (lock);

    if (processor != 0)
        processor->processBlock (buffer, incomingMidi);
}

void AudioProcessorPlayer::audioDeviceAboutToStart (AudioIODevice* device)
{
    const ScopedLock sl (lock);

    sampleRate = device->getCurrentSampleRate();
    blockSize = device->getCurrentBufferSizeSamples();
    numInputChans = device->getActiveInputChannels().countNumberOfSetBits();
    numOutputChans = device->getActiveOutputChannels().countNumberOfSetBits();

    messageCollector.reset (sampleRate);
    zeromem (channels, sizeof (channels));

    if (processor != 0)
    {
        if (isPrepared)
            processor->releaseResources();

        AudioProcessor* const oldProcessor = processor;
        setProcessor (0);
        setProcessor (oldProcessor);
    }
}

void AudioProcessorPlayer::audioDeviceStopped()
{
    const ScopedLock sl (lock);

    if (processor != 0 && isPrepared)
        processor->releaseResources();

    sampleRate = 0.0;
    blockSize = 0;
    isPrepared = false;
    tempBuffer.setSize (1, 1);
}

void AudioProcessorPlayer::handleIncomingMidiMessage (MidiInput*, const MidiMessage& message)
{
    messageCollector.addMessageToQueue (message);
}


END_JUCE_NAMESPACE
