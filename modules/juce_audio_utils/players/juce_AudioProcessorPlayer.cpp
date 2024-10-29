/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** Sets up `channels` so that it contains channel pointers suitable for passing to
    an AudioProcessor's processBlock.

    On return, `channels` will hold `max (processorIns, processorOuts)` entries.
    The first `processorIns` entries will point to buffers holding input data.
    Any entries after the first `processorIns` entries will point to zeroed buffers.

    In the case that the system only provides a single input channel, but the processor
    has been initialised with multiple input channels, the system input will be copied
    to all processor inputs.

    In the case that the system provides no input channels, but the processor has
    been initialised with multiple input channels, the processor's input channels will
    all be zeroed.

    @param ins            the system inputs.
    @param outs           the system outputs.
    @param numSamples     the number of samples in the system buffers.
    @param processorIns   the number of input channels requested by the processor.
    @param processorOuts  the number of output channels requested by the processor.
    @param tempBuffer     temporary storage for inputs that don't have a corresponding output.
    @param channels       holds pointers to each of the processor's audio channels.
*/
static void initialiseIoBuffers (Span<const float* const> ins,
                                 Span<float* const> outs,
                                 const int numSamples,
                                 size_t processorIns,
                                 size_t processorOuts,
                                 AudioBuffer<float>& tempBuffer,
                                 std::vector<float*>& channels)
{
    const auto totalNumChannels = jmax (processorIns, processorOuts);

    jassert (channels.capacity() >= totalNumChannels);
    jassert ((size_t) tempBuffer.getNumChannels() >= totalNumChannels);
    jassert (tempBuffer.getNumSamples() >= numSamples);

    channels.resize (totalNumChannels);

    const auto numBytes = (size_t) numSamples * sizeof (float);

    size_t tempBufferIndex = 0;

    for (size_t i = 0; i < totalNumChannels; ++i)
    {
        auto*& channelPtr = channels[i];
        channelPtr = i < outs.size()
                   ? outs[i]
                   : tempBuffer.getWritePointer ((int) tempBufferIndex++);

        // If there's a single input channel, route it to all inputs on the processor
        if (ins.size() == 1 && i < processorIns)
            memcpy (channelPtr, ins.front(), numBytes);

        // Otherwise, if there's a system input corresponding to this channel, use that
        else if (i < ins.size())
            memcpy (channelPtr, ins[i], numBytes);

        // Otherwise, silence the channel
        else
            zeromem (channelPtr, numBytes);
    }

    // Zero any output channels that won't be written by the processor
    for (size_t i = totalNumChannels; i < outs.size(); ++i)
        zeromem (outs[i], numBytes);
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

    if (it == layouts.end())
        return defaultProcessorChannels;

    return *it;
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

    sampleCount = 0;
    currentWorkgroup.reset();

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

        currentWorkgroup.reset();

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
void AudioProcessorPlayer::audioDeviceIOCallbackWithContext (const float* const* const inputChannelData,
                                                             const int numInputChannels,
                                                             float* const* const outputChannelData,
                                                             const int numOutputChannels,
                                                             const int numSamples,
                                                             const AudioIODeviceCallbackContext& context)
{
    const ScopedLock sl (lock);

    jassert (currentDevice != nullptr);

    // These should have been prepared by audioDeviceAboutToStart()...
    jassert (sampleRate > 0 && blockSize > 0);

    incomingMidi.clear();
    messageCollector.removeNextBlockOfMessages (incomingMidi, numSamples);

    initialiseIoBuffers ({ inputChannelData,  (size_t) numInputChannels },
                         { outputChannelData, (size_t) numOutputChannels },
                         numSamples,
                         (size_t) actualProcessorChannels.ins,
                         (size_t) actualProcessorChannels.outs,
                         tempBuffer,
                         channels);

    const auto totalNumChannels = jmax (actualProcessorChannels.ins, actualProcessorChannels.outs);
    AudioBuffer<float> buffer (channels.data(), (int) totalNumChannels, numSamples);

    if (processor != nullptr)
    {
        const ScopedLock sl2 (processor->getCallbackLock());

        if (std::exchange (currentWorkgroup, currentDevice->getWorkgroup()) != currentDevice->getWorkgroup())
            processor->audioWorkgroupContextChanged (currentWorkgroup);

        class PlayHead final : private AudioPlayHead
        {
        public:
            PlayHead (AudioProcessor& proc,
                      Optional<uint64_t> hostTimeIn,
                      uint64_t sampleCountIn,
                      double sampleRateIn)
                : processor (proc),
                  hostTimeNs (hostTimeIn),
                  sampleCount (sampleCountIn),
                  seconds ((double) sampleCountIn / sampleRateIn)
            {
                if (useThisPlayhead)
                    processor.setPlayHead (this);
            }

            ~PlayHead() override
            {
                if (useThisPlayhead)
                    processor.setPlayHead (nullptr);
            }

        private:
            Optional<PositionInfo> getPosition() const override
            {
                PositionInfo info;
                info.setHostTimeNs (hostTimeNs);
                info.setTimeInSamples ((int64_t) sampleCount);
                info.setTimeInSeconds (seconds);
                return info;
            }

            AudioProcessor& processor;
            Optional<uint64_t> hostTimeNs;
            uint64_t sampleCount;
            double seconds;
            bool useThisPlayhead = processor.getPlayHead() == nullptr;
        };

        PlayHead playHead { *processor,
                            context.hostTimeNs != nullptr ? makeOptional (*context.hostTimeNs) : nullopt,
                            sampleCount,
                            sampleRate };

        sampleCount += (uint64_t) numSamples;

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
    currentDevice = device;
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

    currentWorkgroup.reset();

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

    currentDevice = nullptr;
    currentWorkgroup.reset();
}

void AudioProcessorPlayer::handleIncomingMidiMessage (MidiInput*, const MidiMessage& message)
{
    messageCollector.addMessageToQueue (message);
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct AudioProcessorPlayerTests final : public UnitTest
{
    struct Layout
    {
        int numIns, numOuts;
    };

    AudioProcessorPlayerTests()
        : UnitTest ("AudioProcessorPlayer", UnitTestCategories::audio) {}

    void runTest() override
    {
        beginTest ("Buffers are prepared correctly for a variety of channel layouts");
        {
            const Layout processorLayouts[] { Layout { 0, 0 },
                                              Layout { 1, 1 },
                                              Layout { 4, 4 },
                                              Layout { 4, 8 },
                                              Layout { 8, 4 } };

            const Layout systemLayouts[] { Layout { 0, 1 },
                                           Layout { 0, 2 },
                                           Layout { 1, 1 },
                                           Layout { 1, 2 },
                                           Layout { 1, 0 },
                                           Layout { 2, 2 },
                                           Layout { 2, 0 } };

            for (const auto& processorLayout : processorLayouts)
            {
                for (const auto& systemLayout : systemLayouts)
                    runTest (systemLayout, processorLayout);
            }
        }
    }

    void runTest (Layout systemLayout, Layout processorLayout)
    {
        const int numSamples = 256;
        const auto systemIns = getTestBuffer (systemLayout.numIns, numSamples);
        auto systemOuts = getTestBuffer (systemLayout.numOuts, numSamples);
        AudioBuffer<float> tempBuffer (jmax (processorLayout.numIns, processorLayout.numOuts), numSamples);
        std::vector<float*> channels ((size_t) tempBuffer.getNumChannels());

        initialiseIoBuffers ({ systemIns.getArrayOfReadPointers(),   (size_t) systemIns.getNumChannels() },
                             { systemOuts.getArrayOfWritePointers(), (size_t) systemOuts.getNumChannels() },
                             numSamples,
                             (size_t) processorLayout.numIns,
                             (size_t) processorLayout.numOuts,
                             tempBuffer,
                             channels);

        for (const auto [index, channel] : enumerate (channels, int{}))
        {
            const auto value = [&, channelIndex = index]
            {
                // Any channels past the number of processor inputs should be silent.
                if (processorLayout.numIns <= channelIndex)
                    return 0.0f;

                // If there's one input, all input channels should copy from that input.
                if (systemLayout.numIns == 1)
                    return 1.0f;

                // If there's not exactly one input, any channels past the number of system inputs should be silent.
                if (systemLayout.numIns <= channelIndex)
                    return 0.0f;

                // Otherwise, each processor input should match the corresponding system input.
                return (float) (channelIndex + 1);
            }();

            expect (FloatVectorOperations::findMinAndMax (channel, numSamples) == Range<float> (value, value));
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
