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

AudioProcessorPlayer::AudioProcessorPlayer(bool doDoublePrecisionProcessing)
    : processor (nullptr),
      sampleRate (0),
      blockSize (0),
      isPrepared (false),
      isDoublePrecision (doDoublePrecisionProcessing),
      numInputChans (0),
      numOutputChans (0)
{
}

AudioProcessorPlayer::~AudioProcessorPlayer()
{
    setProcessor (nullptr);
}

//==============================================================================
void AudioProcessorPlayer::setProcessor (AudioProcessor* const processorToPlay)
{
    if (processor != processorToPlay)
    {
        if (processorToPlay != nullptr && sampleRate > 0 && blockSize > 0)
        {
            const int numInBuses  = processorToPlay->busArrangement.inputBuses. size();
            const int numOutBuses = processorToPlay->busArrangement.outputBuses.size();

            for (int i = 1; i < numInBuses; ++i)
            {
                bool success = processorToPlay->setPreferredBusArrangement (true, i, AudioChannelSet::disabled());

                // if using in audio processor player, it must be possible to disable sidechains
                jassert (success);

                ignoreUnused (success);
            }

            for (int i = 1; i < numOutBuses; ++i)
            {
                bool success = processorToPlay->setPreferredBusArrangement (false, i, AudioChannelSet::disabled());

                // if using in audio processor player, it must be possible to disable aux outputs
                jassert (success);

                ignoreUnused(success);
            }

            if (numInBuses > 0 && processorToPlay->busArrangement.inputBuses.getReference(0).channels.size() != numInputChans)
                processorToPlay->setPreferredBusArrangement (true,  0, AudioChannelSet::canonicalChannelSet(numInputChans));

            if (numOutBuses > 0 && processorToPlay->busArrangement.outputBuses.getReference(0).channels.size() != numOutputChans)
                processorToPlay->setPreferredBusArrangement (false,  0, AudioChannelSet::canonicalChannelSet(numOutputChans));

            jassert (processorToPlay->getTotalNumInputChannels()  == numInputChans);
            jassert (processorToPlay->getTotalNumOutputChannels() == numOutputChans);

            processorToPlay->setRateAndBufferSizeDetails (sampleRate, blockSize);

            const bool supportsDouble = processorToPlay->supportsDoublePrecisionProcessing() && isDoublePrecision;
            AudioProcessor::ProcessingPrecision precision = supportsDouble ? AudioProcessor::doublePrecision
                                                                           : AudioProcessor::singlePrecision;

            processorToPlay->setProcessingPrecision (precision);
            processorToPlay->prepareToPlay (sampleRate, blockSize);
        }

        AudioProcessor* oldOne;

        {
            const ScopedLock sl (lock);
            oldOne = isPrepared ? processor : nullptr;
            processor = processorToPlay;
            isPrepared = true;
        }

        if (oldOne != nullptr)
            oldOne->releaseResources();
    }
}

void AudioProcessorPlayer::setDoublePrecisionProcessing (bool doublePrecision)
{
    if (doublePrecision != isDoublePrecision)
    {
        const ScopedLock sl (lock);

        if (processor != nullptr)
        {
            processor->releaseResources();

            const bool supportsDouble = processor->supportsDoublePrecisionProcessing() && doublePrecision;
            AudioProcessor::ProcessingPrecision precision = supportsDouble ? AudioProcessor::doublePrecision
                                                                           : AudioProcessor::singlePrecision;

            processor->setProcessingPrecision (precision);
            processor->prepareToPlay (sampleRate, blockSize);
        }

        isDoublePrecision = doublePrecision;
    }
}

//==============================================================================
void AudioProcessorPlayer::audioDeviceIOCallback (const float** const inputChannelData,
                                                  const int numInputChannels,
                                                  float** const outputChannelData,
                                                  const int numOutputChannels,
                                                  const int numSamples)
{
    // these should have been prepared by audioDeviceAboutToStart()...
    jassert (sampleRate > 0 && blockSize > 0);

    incomingMidi.clear();
    messageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);
    int totalNumChans = 0;

    if (numInputChannels > numOutputChannels)
    {
        // if there aren't enough output channels for the number of
        // inputs, we need to create some temporary extra ones (can't
        // use the input data in case it gets written to)
        tempBuffer.setSize (numInputChannels - numOutputChannels, numSamples,
                            false, false, true);

        for (int i = 0; i < numOutputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
            ++totalNumChans;
        }

        for (int i = numOutputChannels; i < numInputChannels; ++i)
        {
            channels[totalNumChans] = tempBuffer.getWritePointer (i - numOutputChannels);
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
            ++totalNumChans;
        }
    }
    else
    {
        for (int i = 0; i < numInputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            memcpy (channels[totalNumChans], inputChannelData[i], sizeof (float) * (size_t) numSamples);
            ++totalNumChans;
        }

        for (int i = numInputChannels; i < numOutputChannels; ++i)
        {
            channels[totalNumChans] = outputChannelData[i];
            zeromem (channels[totalNumChans], sizeof (float) * (size_t) numSamples);
            ++totalNumChans;
        }
    }

    AudioSampleBuffer buffer (channels, totalNumChans, numSamples);

    {
        const ScopedLock sl (lock);

        if (processor != nullptr)
        {
            const ScopedLock sl2 (processor->getCallbackLock());

            if (! processor->isSuspended())
            {
                if (processor->isUsingDoublePrecision())
                {
                    conversionBuffer.makeCopyOf (buffer);
                    processor->processBlock (conversionBuffer, incomingMidi);
                    buffer.makeCopyOf (conversionBuffer);
                }
                else
                {
                    processor->processBlock (buffer, incomingMidi);
                }

                return;
            }
        }
    }

    for (int i = 0; i < numOutputChannels; ++i)
        FloatVectorOperations::clear (outputChannelData[i], numSamples);
}

void AudioProcessorPlayer::audioDeviceAboutToStart (AudioIODevice* const device)
{
    const double newSampleRate = device->getCurrentSampleRate();
    const int newBlockSize     = device->getCurrentBufferSizeSamples();
    const int numChansIn       = device->getActiveInputChannels().countNumberOfSetBits();
    const int numChansOut      = device->getActiveOutputChannels().countNumberOfSetBits();

    const ScopedLock sl (lock);

    sampleRate = newSampleRate;
    blockSize  = newBlockSize;
    numInputChans  = numChansIn;
    numOutputChans = numChansOut;

    messageCollector.reset (sampleRate);
    channels.calloc ((size_t) jmax (numChansIn, numChansOut) + 2);

    if (processor != nullptr)
    {
        if (isPrepared)
            processor->releaseResources();

        AudioProcessor* const oldProcessor = processor;
        setProcessor (nullptr);
        setProcessor (oldProcessor);
    }
}

void AudioProcessorPlayer::audioDeviceStopped()
{
    const ScopedLock sl (lock);

    if (processor != nullptr && isPrepared)
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
