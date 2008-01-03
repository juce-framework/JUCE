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

#include "juce_AudioProcessorPlayer.h"
#include "../../../juce_core/threads/juce_ScopedLock.h"


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
                                                  int totalNumInputChannels,
                                                  float** outputChannelData,
                                                  int totalNumOutputChannels,
                                                  int numSamples)
{
    // these should have been prepared by audioDeviceAboutToStart()...
    jassert (sampleRate > 0 && blockSize > 0);

    incomingMidi.clear();
    messageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);

    int i, numActiveChans = 0, numInputs = 0, numOutputs = 0;

    // messy stuff needed to compact the channels down into an array
    // of non-zero pointers..
    for (i = 0; i < totalNumInputChannels; ++i)
    {
        if (inputChannelData[i] != 0)
        {
            inputChans [numInputs++] = inputChannelData[i];
            if (numInputs >= numElementsInArray (inputChans))
                break;
        }
    }

    for (i = 0; i < totalNumOutputChannels; ++i)
    {
        if (outputChannelData[i] != 0)
        {
            outputChans [numOutputs++] = outputChannelData[i];
            if (numOutputs >= numElementsInArray (outputChans))
                break;
        }
    }

    if (numInputs > numOutputs)
    {
        // if there aren't enough output channels for the number of
        // inputs, we need to create some temporary extra ones (can't
        // use the input data in case it gets written to)
        tempBuffer.setSize (numInputs - numOutputs, numSamples,
                            false, false, true);

        for (i = 0; i < numOutputs; ++i)
        {
            channels[numActiveChans] = outputChans[i];
            memcpy (channels[numActiveChans], inputChans[i], sizeof (float) * numSamples);
            ++numActiveChans;
        }

        for (i = numOutputs; i < numInputs; ++i)
        {
            channels[numActiveChans] = tempBuffer.getSampleData (i - numOutputs, 0);
            memcpy (channels[numActiveChans], inputChans[i], sizeof (float) * numSamples);
            ++numActiveChans;
        }
    }
    else
    {
        for (i = 0; i < numInputs; ++i)
        {
            channels[numActiveChans] = outputChans[i];
            memcpy (channels[numActiveChans], inputChans[i], sizeof (float) * numSamples);
            ++numActiveChans;
        }

        for (i = numInputs; i < numOutputs; ++i)
        {
            channels[numActiveChans] = outputChans[i];
            zeromem (channels[numActiveChans], sizeof (float) * numSamples);
            ++numActiveChans;
        }
    }

    AudioSampleBuffer buffer (channels, numActiveChans, numSamples);

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
