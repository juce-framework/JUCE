/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

template <typename Value>
struct ChannelInfo
{
    ChannelInfo() = default;
    ChannelInfo (Value** dataIn, int numChannelsIn)
        : data (dataIn), numChannels (numChannelsIn)  {}

    Value** data = nullptr;
    int numChannels = 0;
};

/** Sets up `channels` so that it contains channel pointers suitable for passing to
    an AudioProcessor's processBlock.

    On return, `channels` will hold `max (processorIns, processorOuts)` entries.
    The first `processorIns` entries will point to buffers holding input data.
    Any entries after the first `processorIns` entries will point to zeroed buffers.

    In the case that the system only provides a single input channel, but the processor
    has been initialised with multiple input channels, the system input will be copied
    to all processor inputs.

    In the case that the system provides no input channels, but the processor has
    been initialise with multiple input channels, the processor's input channels will
    all be zeroed.

    @param ins            the system inputs.
    @param outs           the system outputs.
    @param numSamples     the number of samples in the system buffers.
    @param processorIns   the number of input channels requested by the processor.
    @param processorOuts  the number of output channels requested by the processor.
    @param tempBuffer     temporary storage for inputs that don't have a corresponding output.
    @param channels       holds pointers to each of the processor's audio channels.
*/
static void initialiseIoBuffers (ChannelInfo<const float> ins,
                                 ChannelInfo<float> outs,
                                 const int numSamples,
                                 int processorIns,
                                 int processorOuts,
                                 AudioBuffer<float>& tempBuffer,
                                 std::vector<float*>& channels)
{
    jassert ((int) channels.size() >= jmax (processorIns, processorOuts));

    size_t totalNumChans = 0;
    const auto numBytes = (size_t) numSamples * sizeof (float);

    const auto prepareInputChannel = [&] (int index)
    {
        if (ins.numChannels == 0)
            zeromem (channels[totalNumChans], numBytes);
        else
            memcpy (channels[totalNumChans], ins.data[index % ins.numChannels], numBytes);
    };

    if (processorIns > processorOuts)
    {
        // If there aren't enough output channels for the number of
        // inputs, we need to use some temporary extra ones (can't
        // use the input data in case it gets written to).
        jassert (tempBuffer.getNumChannels() >= processorIns - processorOuts);
        jassert (tempBuffer.getNumSamples() >= numSamples);

        for (int i = 0; i < processorOuts; ++i)
        {
            channels[totalNumChans] = outs.data[i];
            prepareInputChannel (i);
            ++totalNumChans;
        }

        for (auto i = processorOuts; i < processorIns; ++i)
        {
            channels[totalNumChans] = tempBuffer.getWritePointer (i - outs.numChannels);
            prepareInputChannel (i);
            ++totalNumChans;
        }
    }
    else
    {
        for (int i = 0; i < processorIns; ++i)
        {
            channels[totalNumChans] = outs.data[i];
            prepareInputChannel (i);
            ++totalNumChans;
        }

        for (auto i = processorIns; i < processorOuts; ++i)
        {
            channels[totalNumChans] = outs.data[i];
            zeromem (channels[totalNumChans], (size_t) numSamples * sizeof (float));
            ++totalNumChans;
        }
    }
}

//==============================================================================
AudioProcessorPlayer::AudioProcessorPlayer (bool doDoublePrecisionProcessing)
    : isDoublePrecision (doDoublePrecisionProcessing)
{
}

AudioProcessorPlayer::~AudioProcessorPlayer()
{
    setProcessor (nullptr);
}

//==============================================================================
AudioProcessorPlayer::NumChannels AudioProcessorPlayer::findMostSuitableLayout (const AudioProcessor& proc) const
{
    if (proc.isMidiEffect())
        return {};

    std::vector<NumChannels> layouts { deviceChannels };

    if (deviceChannels.ins == 0 || deviceChannels.ins == 1)
    {
        layouts.emplace_back (defaultProcessorChannels.ins, deviceChannels.outs);
        layouts.emplace_back (deviceChannels.outs, deviceChannels.outs);
    }

    const auto it = std::find_if (layouts.begin(), layouts.end(), [&] (const NumChannels& chans)
    {
        return proc.checkBusesLayoutSupported (chans.toLayout());
    });

    return it != std::end (layouts) ? *it : layouts[0];
}

void AudioProcessorPlayer::resizeChannels()
{
    const auto maxChannels = jmax (deviceChannels.ins,
                                   deviceChannels.outs,
                                   actualProcessorChannels.ins,
                                   actualProcessorChannels.outs);
    channels.resize ((size_t) maxChannels);
    tempBuffer.setSize (maxChannels, blockSize);
}

void AudioProcessorPlayer::setProcessor (AudioProcessor* const processorToPlay)
{
    const ScopedLock sl (lock);

    if (processor == processorToPlay)
        return;

    if (processorToPlay != nullptr && sampleRate > 0 && blockSize > 0)
    {
        defaultProcessorChannels = NumChannels { processorToPlay->getBusesLayout() };
        actualProcessorChannels  = findMostSuitableLayout (*processorToPlay);

        if (processorToPlay->isMidiEffect())
            processorToPlay->setRateAndBufferSizeDetails (sampleRate, blockSize);
        else
            processorToPlay->setPlayConfigDetails (actualProcessorChannels.ins,
                                                   actualProcessorChannels.outs,
                                                   sampleRate,
                                                   blockSize);

        auto supportsDouble = processorToPlay->supportsDoublePrecisionProcessing() && isDoublePrecision;

        processorToPlay->setProcessingPrecision (supportsDouble ? AudioProcessor::doublePrecision
                                                                : AudioProcessor::singlePrecision);
        processorToPlay->prepareToPlay (sampleRate, blockSize);
    }

    AudioProcessor* oldOne = nullptr;

    oldOne = isPrepared ? processor : nullptr;
    processor = processorToPlay;
    isPrepared = true;
    resizeChannels();

    if (oldOne != nullptr)
        oldOne->releaseResources();
}

void AudioProcessorPlayer::setDoublePrecisionProcessing (bool doublePrecision)
{
    if (doublePrecision != isDoublePrecision)
    {
        const ScopedLock sl (lock);

        if (processor != nullptr)
        {
            processor->releaseResources();

            auto supportsDouble = processor->supportsDoublePrecisionProcessing() && doublePrecision;

            processor->setProcessingPrecision (supportsDouble ? AudioProcessor::doublePrecision
                                                              : AudioProcessor::singlePrecision);
            processor->prepareToPlay (sampleRate, blockSize);
        }

        isDoublePrecision = doublePrecision;
    }
}

void AudioProcessorPlayer::setMidiOutput (MidiOutput* midiOutputToUse)
{
    if (midiOutput != midiOutputToUse)
    {
        const ScopedLock sl (lock);
        midiOutput = midiOutputToUse;
    }
}

//==============================================================================
void AudioProcessorPlayer::audioDeviceIOCallback (const float** const inputChannelData,
                                                  const int numInputChannels,
                                                  float** const outputChannelData,
                                                  const int numOutputChannels,
                                                  const int numSamples)
{
    const ScopedLock sl (lock);

    // These should have been prepared by audioDeviceAboutToStart()...
    jassert (sampleRate > 0 && blockSize > 0);

    incomingMidi.clear();
    messageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);

    initialiseIoBuffers ({ inputChannelData,  numInputChannels },
                         { outputChannelData, numOutputChannels },
                         numSamples,
                         actualProcessorChannels.ins,
                         actualProcessorChannels.outs,
                         tempBuffer,
                         channels);

    const auto totalNumChannels = jmax (actualProcessorChannels.ins, actualProcessorChannels.outs);
    AudioBuffer<float> buffer (channels.data(), (int) totalNumChannels, numSamples);

    if (processor != nullptr)
    {
        // The processor should be prepared to deal with the same number of output channels
        // as our output device.
        jassert (processor->isMidiEffect() || numOutputChannels == actualProcessorChannels.outs);

        const ScopedLock sl2 (processor->getCallbackLock());

        if (! processor->isSuspended())
        {
            if (processor->isUsingDoublePrecision())
            {
                conversionBuffer.makeCopyOf (buffer, true);
                processor->processBlock (conversionBuffer, incomingMidi);
                buffer.makeCopyOf (conversionBuffer, true);
            }
            else
            {
                processor->processBlock (buffer, incomingMidi);
            }

            if (midiOutput != nullptr)
            {
                if (midiOutput->isBackgroundThreadRunning())
                {
                    midiOutput->sendBlockOfMessages (incomingMidi,
                                                     Time::getMillisecondCounterHiRes(),
                                                     sampleRate);
                }
                else
                {
                    midiOutput->sendBlockOfMessagesNow (incomingMidi);
                }
            }

            return;
        }
    }

    for (int i = 0; i < numOutputChannels; ++i)
        FloatVectorOperations::clear (outputChannelData[i], numSamples);
}

void AudioProcessorPlayer::audioDeviceAboutToStart (AudioIODevice* const device)
{
    auto newSampleRate = device->getCurrentSampleRate();
    auto newBlockSize  = device->getCurrentBufferSizeSamples();
    auto numChansIn    = device->getActiveInputChannels().countNumberOfSetBits();
    auto numChansOut   = device->getActiveOutputChannels().countNumberOfSetBits();

    const ScopedLock sl (lock);

    sampleRate = newSampleRate;
    blockSize  = newBlockSize;
    deviceChannels = { numChansIn, numChansOut };

    resizeChannels();

    messageCollector.reset (sampleRate);

    if (processor != nullptr)
    {
        if (isPrepared)
            processor->releaseResources();

        auto* oldProcessor = processor;
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

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct AudioProcessorPlayerTests  : public UnitTest
{
    AudioProcessorPlayerTests()
        : UnitTest ("AudioProcessorPlayer", UnitTestCategories::audio) {}

    void runTest() override
    {
        struct Layout
        {
            int numIns, numOuts;
        };

        const Layout processorLayouts[] { Layout { 0, 0 },
                                          Layout { 1, 1 },
                                          Layout { 4, 4 },
                                          Layout { 4, 8 },
                                          Layout { 8, 4 } };

        beginTest ("Buffers are prepared correctly for a variety of channel layouts");
        {
            for (const auto& layout : processorLayouts)
            {
                for (const auto numSystemInputs : { 0, 1, layout.numIns })
                {
                    const int numSamples = 256;
                    const auto systemIns = getTestBuffer (numSystemInputs, numSamples);
                    auto systemOuts = getTestBuffer (layout.numOuts, numSamples);
                    AudioBuffer<float> tempBuffer (jmax (layout.numIns, layout.numOuts), numSamples);
                    std::vector<float*> channels ((size_t) jmax (layout.numIns, layout.numOuts), nullptr);

                    initialiseIoBuffers ({ systemIns.getArrayOfReadPointers(),   systemIns.getNumChannels() },
                                         { systemOuts.getArrayOfWritePointers(), systemOuts.getNumChannels() },
                                         numSamples,
                                         layout.numIns,
                                         layout.numOuts,
                                         tempBuffer,
                                         channels);

                    int channelIndex = 0;

                    for (const auto& channel : channels)
                    {
                        const auto value = [&]
                        {
                            // Any channels past the number of inputs should be silent.
                            if (layout.numIns <= channelIndex)
                                return 0.0f;

                            // If there's no input, all input channels should be silent.
                            if (numSystemInputs == 0)       return 0.0f;

                            // If there's one input, all input channels should copy from that input.
                            if (numSystemInputs == 1)       return 1.0f;

                            // Otherwise, each processor input should match the corresponding system input.
                            return (float) (channelIndex + 1);
                        }();

                        expect (FloatVectorOperations::findMinAndMax (channel, numSamples) == Range<float> (value, value));

                        channelIndex += 1;
                    }
                }
            }
        }
    }

    static AudioBuffer<float> getTestBuffer (int numChannels, int numSamples)
    {
        AudioBuffer<float> result (numChannels, numSamples);

        for (int i = 0; i < result.getNumChannels(); ++i)
            FloatVectorOperations::fill (result.getWritePointer (i), (float) i + 1, result.getNumSamples());

        return result;
    }
};

static AudioProcessorPlayerTests audioProcessorPlayerTests;

#endif

} // namespace juce
