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

namespace juce
{
namespace dsp
{

/** This class is the convolution engine itself, processing only one channel at
    a time of input signal.
*/
struct ConvolutionEngine
{
    ConvolutionEngine() = default;

    //==============================================================================
    struct ProcessingInformation
    {
        enum class SourceType
        {
            sourceBinaryData,
            sourceAudioFile,
            sourceAudioBuffer,
            sourceNone
        };

        SourceType sourceType = SourceType::sourceNone;

        const void* sourceData;
        int sourceDataSize;
        File fileImpulseResponse;

        double originalSampleRate;
        int originalSize = 0;
        int originalNumChannels = 1;

        AudioBuffer<float>* buffer;

        bool wantsStereo = true;
        bool wantsTrimming = true;
        bool wantsNormalization = true;
        int64 wantedSize = 0;
        int finalSize = 0;

        double sampleRate = 0;
        size_t maximumBufferSize = 0;
    };

    //==============================================================================
    void reset()
    {
        bufferInput.clear();
        bufferOverlap.clear();
        bufferTempOutput.clear();

        for (auto i = 0; i < buffersInputSegments.size(); ++i)
            buffersInputSegments.getReference (i).clear();

        currentSegment = 0;
        inputDataPos = 0;
    }

    /** Initalize all the states and objects to perform the convolution. */
    void initializeConvolutionEngine (ProcessingInformation& info, int channel)
    {
        blockSize = (size_t) nextPowerOfTwo ((int) info.maximumBufferSize);

        FFTSize = blockSize > 128 ? 2 * blockSize
                                  : 4 * blockSize;

        numSegments = ((size_t) info.finalSize) / (FFTSize - blockSize) + 1u;

        numInputSegments = (blockSize > 128 ? numSegments : 3 * numSegments);

        FFTobject.reset (new FFT (roundToInt (std::log2 (FFTSize))));

        bufferInput.setSize      (1, static_cast<int> (FFTSize));
        bufferOutput.setSize     (1, static_cast<int> (FFTSize * 2));
        bufferTempOutput.setSize (1, static_cast<int> (FFTSize * 2));
        bufferOverlap.setSize    (1, static_cast<int> (FFTSize));

        buffersInputSegments.clear();
        buffersImpulseSegments.clear();
        bufferOutput.clear();

        for (size_t i = 0; i < numInputSegments; ++i)
        {
            AudioBuffer<float> newInputSegment;
            newInputSegment.setSize (1, static_cast<int> (FFTSize * 2));
            buffersInputSegments.add (newInputSegment);
        }

        for (auto i = 0u; i < numSegments; ++i)
        {
            AudioBuffer<float> newImpulseSegment;
            newImpulseSegment.setSize (1, static_cast<int> (FFTSize * 2));
            buffersImpulseSegments.add (newImpulseSegment);
        }

        std::unique_ptr<FFT> FFTTempObject (new FFT (roundToInt (std::log2 (FFTSize))));

        auto* channelData = info.buffer->getWritePointer (channel);

        for (size_t n = 0; n < numSegments; ++n)
        {
            buffersImpulseSegments.getReference (static_cast<int> (n)).clear();

            auto* impulseResponse = buffersImpulseSegments.getReference (static_cast<int> (n)).getWritePointer (0);

            if (n == 0)
                impulseResponse[0] = 1.0f;

            for (size_t i = 0; i < FFTSize - blockSize; ++i)
                if (i + n * (FFTSize - blockSize) < (size_t) info.finalSize)
                    impulseResponse[i] = channelData[i + n * (FFTSize - blockSize)];

            FFTTempObject->performRealOnlyForwardTransform (impulseResponse);
            prepareForConvolution (impulseResponse);
        }

        reset();

        isReady = true;
    }

    /** Copy the states of another engine. */
    void copyStateFromOtherEngine (const ConvolutionEngine& other)
    {
        if (FFTSize != other.FFTSize)
        {
            FFTobject.reset (new FFT (roundToInt (std::log2 (other.FFTSize))));
            FFTSize = other.FFTSize;
        }

        currentSegment      = other.currentSegment;
        numInputSegments    = other.numInputSegments;
        numSegments         = other.numSegments;
        blockSize           = other.blockSize;
        inputDataPos        = other.inputDataPos;

        bufferInput         = other.bufferInput;
        bufferTempOutput    = other.bufferTempOutput;
        bufferOutput        = other.bufferOutput;

        buffersInputSegments    = other.buffersInputSegments;
        buffersImpulseSegments  = other.buffersImpulseSegments;
        bufferOverlap           = other.bufferOverlap;

        isReady = true;
    }

    /** Performs the uniform partitioned convolution using FFT. */
    void processSamples (const float* input, float* output, size_t numSamples)
    {
        if (! isReady)
            return;

        // Overlap-add, zero latency convolution algorithm with uniform partitioning
        size_t numSamplesProcessed = 0;

        auto indexStep = numInputSegments / numSegments;

        auto* inputData      = bufferInput.getWritePointer (0);
        auto* outputTempData = bufferTempOutput.getWritePointer (0);
        auto* outputData     = bufferOutput.getWritePointer (0);
        auto* overlapData    = bufferOverlap.getWritePointer (0);

        while (numSamplesProcessed < numSamples)
        {
            const bool inputDataWasEmpty = (inputDataPos == 0);
            auto numSamplesToProcess = jmin (numSamples - numSamplesProcessed, blockSize - inputDataPos);

            // copy the input samples
            FloatVectorOperations::copy (inputData + inputDataPos, input + numSamplesProcessed, static_cast<int> (numSamplesToProcess));

            auto* inputSegmentData = buffersInputSegments.getReference (static_cast<int> (currentSegment)).getWritePointer (0);
            FloatVectorOperations::copy (inputSegmentData, inputData, static_cast<int> (FFTSize));

            // Forward FFT
            FFTobject->performRealOnlyForwardTransform (inputSegmentData);
            prepareForConvolution (inputSegmentData);

            // Complex multiplication
            if (inputDataWasEmpty)
            {
                FloatVectorOperations::fill (outputTempData, 0, static_cast<int> (FFTSize + 1));

                auto index = currentSegment;

                for (size_t i = 1; i < numSegments; ++i)
                {
                    index += indexStep;

                    if (index >= numInputSegments)
                        index -= numInputSegments;

                    convolutionProcessingAndAccumulate (buffersInputSegments.getReference (static_cast<int> (index)).getWritePointer (0),
                                                        buffersImpulseSegments.getReference (static_cast<int> (i)).getWritePointer (0),
                                                        outputTempData);
                }
            }

            FloatVectorOperations::copy (outputData, outputTempData, static_cast<int> (FFTSize + 1));

            convolutionProcessingAndAccumulate (buffersInputSegments.getReference (static_cast<int> (currentSegment)).getWritePointer (0),
                                                buffersImpulseSegments.getReference (0).getWritePointer (0),
                                                outputData);

            // Inverse FFT
            updateSymmetricFrequencyDomainData (outputData);
            FFTobject->performRealOnlyInverseTransform (outputData);

            // Add overlap
            for (size_t i = 0; i < numSamplesToProcess; ++i)
                output[i + numSamplesProcessed] = outputData[inputDataPos + i] + overlapData[inputDataPos + i];

            // Input buffer full => Next block
            inputDataPos += numSamplesToProcess;

            if (inputDataPos == blockSize)
            {
                // Input buffer is empty again now
                FloatVectorOperations::fill (inputData, 0.0f, static_cast<int> (FFTSize));

                inputDataPos = 0;

                // Extra step for segSize > blockSize
                FloatVectorOperations::add (&(outputData[blockSize]), &(overlapData[blockSize]), static_cast<int> (FFTSize - 2 * blockSize));

                // Save the overlap
                FloatVectorOperations::copy (overlapData, &(outputData[blockSize]), static_cast<int> (FFTSize - blockSize));

                // Update current segment
                currentSegment = (currentSegment > 0) ? (currentSegment - 1) : (numInputSegments - 1);
            }

            numSamplesProcessed += numSamplesToProcess;
        }
    }

    /** After each FFT, this function is called to allow convolution to be performed with only 4 SIMD functions calls. */
    void prepareForConvolution (float *samples) noexcept
    {
        auto FFTSizeDiv2 = FFTSize / 2;

        for (size_t i = 0; i < FFTSizeDiv2; i++)
            samples[i] = samples[2 * i];

        samples[FFTSizeDiv2] = 0;

        for (size_t i = 1; i < FFTSizeDiv2; i++)
            samples[i + FFTSizeDiv2] = -samples[2 * (FFTSize - i) + 1];
    }

    /** Does the convolution operation itself only on half of the frequency domain samples. */
    void convolutionProcessingAndAccumulate (const float *input, const float *impulse, float *output)
    {
        auto FFTSizeDiv2 = FFTSize / 2;

        FloatVectorOperations::addWithMultiply      (output, input, impulse, static_cast<int> (FFTSizeDiv2));
        FloatVectorOperations::subtractWithMultiply (output, &(input[FFTSizeDiv2]), &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));

        FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), input, &(impulse[FFTSizeDiv2]), static_cast<int> (FFTSizeDiv2));
        FloatVectorOperations::addWithMultiply      (&(output[FFTSizeDiv2]), &(input[FFTSizeDiv2]), impulse, static_cast<int> (FFTSizeDiv2));

        output[FFTSize] += input[FFTSize] * impulse[FFTSize];
    }

    /** Undo the re-organization of samples from the function prepareForConvolution.
        Then, takes the conjugate of the frequency domain first half of samples, to fill the
        second half, so that the inverse transform will return real samples in the time domain.
    */
    void updateSymmetricFrequencyDomainData (float* samples) noexcept
    {
        auto FFTSizeDiv2 = FFTSize / 2;

        for (size_t i = 1; i < FFTSizeDiv2; i++)
        {
            samples[2 * (FFTSize - i)] = samples[i];
            samples[2 * (FFTSize - i) + 1] = -samples[FFTSizeDiv2 + i];
        }

        samples[1] = 0.f;

        for (size_t i = 1; i < FFTSizeDiv2; i++)
        {
            samples[2 * i] = samples[2 * (FFTSize - i)];
            samples[2 * i + 1] = -samples[2 * (FFTSize - i) + 1];
        }
    }

    //==============================================================================
    std::unique_ptr<FFT> FFTobject;

    size_t FFTSize = 0;
    size_t currentSegment = 0, numInputSegments = 0, numSegments = 0, blockSize = 0, inputDataPos = 0;

    AudioBuffer<float> bufferInput, bufferOutput, bufferTempOutput, bufferOverlap;
    Array<AudioBuffer<float>> buffersInputSegments, buffersImpulseSegments;

    bool isReady = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConvolutionEngine)
};



//==============================================================================
/** Manages all the changes requested by the main convolution engine, to minimize
    the number of calls of the convolution engine initialization, and the potential
    consequences of multiple quick calls to the function Convolution::loadImpulseResponse.
*/
struct Convolution::Pimpl  : private Thread
{
    enum class ChangeRequest
    {
        changeEngine = 0,
        changeSampleRate,
        changeMaximumBufferSize,
        changeSource,
        changeImpulseResponseSize,
        changeStereo,
        changeTrimming,
        changeNormalization,
        changeIgnore,
        numChangeRequestTypes
    };

    using SourceType = ConvolutionEngine::ProcessingInformation::SourceType;

    //==============================================================================
    Pimpl()  : Thread ("Convolution"), abstractFifo (fifoSize)
    {
        abstractFifo.reset();
        fifoRequestsType.resize (fifoSize);
        fifoRequestsParameter.resize (fifoSize);

        requestsType.resize (fifoSize);
        requestsParameter.resize (fifoSize);

        for (auto i = 0; i < 4; ++i)
            engines.add (new ConvolutionEngine());

        currentInfo.maximumBufferSize = 0;
        currentInfo.buffer = &impulseResponse;

        temporaryBuffer.setSize (2, static_cast<int> (maximumTimeInSamples), false, false, true);
        impulseResponseOriginal.setSize (2, static_cast<int> (maximumTimeInSamples), false, false, true);
        impulseResponse.setSize (2, static_cast<int> (maximumTimeInSamples), false, false, true);
    }

    ~Pimpl()
    {
        stopThread (10000);
    }

    //==============================================================================
    /** Inits the size of the interpolation buffer. */
    void initProcessing (int maximumBufferSize)
    {
        stopThread (1000);

        interpolationBuffer.setSize (1, maximumBufferSize, false, false, true);
        mustInterpolate = false;
    }

    //==============================================================================
    /** Adds a new change request. */
    void addToFifo (ChangeRequest type, juce::var parameter)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite (1, start1, size1, start2, size2);

        if (size1 > 0)
        {
            fifoRequestsType.setUnchecked (start1, type);
            fifoRequestsParameter.setUnchecked (start1, parameter);
        }

        if (size2 > 0)
        {
            fifoRequestsType.setUnchecked (start2, type);
            fifoRequestsParameter.setUnchecked (start2, parameter);
        }

        abstractFifo.finishedWrite (size1 + size2);
    }

    /** Adds a new array of change requests. */
    void addToFifo (ChangeRequest* types, juce::var* parameters, int numEntries)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite (numEntries, start1, size1, start2, size2);

        if (size1 > 0)
        {
            for (auto i = 0; i < size1; ++i)
            {
                fifoRequestsType.setUnchecked (start1 + i, types[i]);
                fifoRequestsParameter.setUnchecked (start1 + i, parameters[i]);
            }
        }

        if (size2 > 0)
        {
            for (auto i = 0; i < size2; ++i)
            {
                fifoRequestsType.setUnchecked (start2 + i, types[i + size1]);
                fifoRequestsParameter.setUnchecked (start2 + i, parameters[i + size1]);
            }
        }

        abstractFifo.finishedWrite (size1 + size2);
    }

    /** Reads requests from the fifo */
    void readFromFifo (ChangeRequest& type, juce::var& parameter)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead (1, start1, size1, start2, size2);

        if (size1 > 0)
        {
            type = fifoRequestsType[start1];
            parameter = fifoRequestsParameter[start1];
        }

        if (size2 > 0)
        {
            type = fifoRequestsType[start2];
            parameter = fifoRequestsParameter[start2];
        }

        abstractFifo.finishedRead (size1 + size2);
    }

    /** Returns the number of requests that still need to be processed */
    int getNumRemainingEntries() const noexcept
    {
        return abstractFifo.getNumReady();
    }

    //==============================================================================
    /** This function processes all the change requests to remove all the the
        redundant ones, and to tell what kind of initialization must be done.

        Depending on the results, the convolution engines might be reset, or
        simply updated, or they might not need any change at all.
    */
    void processFifo()
    {
        if (getNumRemainingEntries() == 0 || isThreadRunning() || mustInterpolate)
            return;

        auto numRequests = 0;

        // retrieve the information from the FIFO for processing
        while (getNumRemainingEntries() > 0)
        {
            ChangeRequest type = ChangeRequest::changeEngine;
            juce::var parameter;

            readFromFifo (type, parameter);

            requestsType.setUnchecked (numRequests, type);
            requestsParameter.setUnchecked (numRequests, parameter);

            numRequests++;
        }

        // remove any useless messages
        for (auto i = 0; i < (int) ChangeRequest::numChangeRequestTypes; ++i)
        {
            bool exists = false;

            for (auto n = numRequests; --n >= 0;)
            {
                if (requestsType[n] == (ChangeRequest) i)
                {
                    if (! exists)
                        exists = true;
                    else
                        requestsType.setUnchecked (n, ChangeRequest::changeIgnore);
                }
            }
        }

        changeLevel = 0;

        for (auto n = 0; n < numRequests; ++n)
        {
            switch (requestsType[n])
            {
                case ChangeRequest::changeEngine:
                    changeLevel = 3;
                    break;

                case ChangeRequest::changeSampleRate:
                {
                    double newSampleRate = requestsParameter[n];

                    if (currentInfo.sampleRate != newSampleRate)
                        changeLevel = 3;

                    currentInfo.sampleRate = newSampleRate;
                }
                break;

                case ChangeRequest::changeMaximumBufferSize:
                {
                    int newMaximumBufferSize = requestsParameter[n];

                    if (currentInfo.maximumBufferSize != (size_t) newMaximumBufferSize)
                        changeLevel = 3;

                    currentInfo.maximumBufferSize = (size_t) newMaximumBufferSize;
                }
                break;

                case ChangeRequest::changeSource:
                {
                    auto* arrayParameters = requestsParameter[n].getArray();
                    auto newSourceType = static_cast<SourceType> (static_cast<int> (arrayParameters->getUnchecked (0)));

                    if (currentInfo.sourceType != newSourceType)
                        changeLevel = jmax (2, changeLevel);

                    if (newSourceType == SourceType::sourceBinaryData)
                    {
                        auto& prm = arrayParameters->getRawDataPointer()[1];
                        auto* newMemoryBlock = prm.getBinaryData();

                        auto* newPtr = newMemoryBlock->getData();
                        auto newSize = (int) newMemoryBlock->getSize();

                        if (currentInfo.sourceData != newPtr || currentInfo.sourceDataSize != newSize)
                            changeLevel = jmax (2, changeLevel);

                        currentInfo.sourceType = SourceType::sourceBinaryData;
                        currentInfo.sourceData = newPtr;
                        currentInfo.sourceDataSize = newSize;
                        currentInfo.fileImpulseResponse = File();
                    }
                    else if (newSourceType == SourceType::sourceAudioFile)
                    {
                        File newFile (arrayParameters->getUnchecked (1).toString());

                        if (currentInfo.fileImpulseResponse != newFile)
                            changeLevel = jmax (2, changeLevel);

                        currentInfo.sourceType = SourceType::sourceAudioFile;
                        currentInfo.fileImpulseResponse = newFile;
                        currentInfo.sourceData = nullptr;
                        currentInfo.sourceDataSize = 0;
                    }
                    else if (newSourceType == SourceType::sourceAudioBuffer)
                    {
                        double originalSampleRate (arrayParameters->getUnchecked (1));
                        changeLevel = jmax (2, changeLevel);

                        currentInfo.sourceType = SourceType::sourceAudioBuffer;
                        currentInfo.originalSampleRate = originalSampleRate;
                        currentInfo.fileImpulseResponse = File();
                        currentInfo.sourceData = nullptr;
                        currentInfo.sourceDataSize = 0;
                    }
                }
                break;

                case ChangeRequest::changeImpulseResponseSize:
                {
                    int64 newSize = requestsParameter[n];

                    if (currentInfo.wantedSize != newSize)
                        changeLevel = jmax (1, changeLevel);

                    currentInfo.wantedSize = newSize;
                }
                break;

                case ChangeRequest::changeStereo:
                {
                    bool newWantsStereo = requestsParameter[n];

                    if (currentInfo.wantsStereo != newWantsStereo)
                        changeLevel = jmax (0, changeLevel);

                    currentInfo.wantsStereo = newWantsStereo;
                }
                break;

                case ChangeRequest::changeTrimming:
                {
                    bool newWantsTrimming = requestsParameter[n];

                    if (currentInfo.wantsTrimming != newWantsTrimming)
                        changeLevel = jmax (1, changeLevel);

                    currentInfo.wantsTrimming = newWantsTrimming;
                }
                break;

                case ChangeRequest::changeNormalization:
                {
                    bool newWantsNormalization = requestsParameter[n];

                    if (currentInfo.wantsNormalization != newWantsNormalization)
                        changeLevel = jmax (1, changeLevel);

                    currentInfo.wantsNormalization = newWantsNormalization;
                }
                break;

                case ChangeRequest::changeIgnore:
                    break;

                default:
                    jassertfalse;
                    break;
            }
        }

        if (currentInfo.sourceType == SourceType::sourceNone)
        {
            currentInfo.sourceType = SourceType::sourceAudioBuffer;

            if (currentInfo.sampleRate == 0)
                currentInfo.sampleRate = 44100;

            if (currentInfo.maximumBufferSize == 0)
                currentInfo.maximumBufferSize = 128;

            currentInfo.originalSampleRate = currentInfo.sampleRate;
            currentInfo.wantedSize = 1;
            currentInfo.fileImpulseResponse = File();
            currentInfo.sourceData = nullptr;
            currentInfo.sourceDataSize = 0;

            AudioBuffer<float> newBuffer;
            newBuffer.setSize (1, 1);
            newBuffer.setSample (0, 0, 1.f);

            copyBufferToTemporaryLocation (newBuffer);
        }

        // action depending on the change level
        if (changeLevel == 3)
        {
            loadImpulseResponse();
            processImpulseResponse();
            initializeConvolutionEngines();
        }
        else if (changeLevel > 0)
        {
            startThread();
        }
    }

    //==============================================================================
    /** This function copies a buffer to a temporary location, so that any external
        audio source can be processed then in the dedicated thread.
    */
    void copyBufferToTemporaryLocation (dsp::AudioBlock<float> block)
    {
        const SpinLock::ScopedLockType sl (processLock);

        currentInfo.originalNumChannels = (block.getNumChannels() > 1 ? 2 : 1);
        currentInfo.originalSize = (int) jmin ((size_t) maximumTimeInSamples, block.getNumSamples());

        for (auto channel = 0; channel < currentInfo.originalNumChannels; ++channel)
            temporaryBuffer.copyFrom (channel, 0, block.getChannelPointer ((size_t) channel), (int) currentInfo.originalSize);
    }

    //==============================================================================
    /** Resets the convolution engines states. */
    void reset()
    {
        for (auto* e : engines)
            e->reset();
    }

    /** Convolution processing handling interpolation between previous and new states
        of the convolution engines.
    */
    void processSamples (const AudioBlock<float>& input, AudioBlock<float>& output)
    {
        processFifo();

        size_t numChannels = jmin (input.getNumChannels(), (size_t) (currentInfo.wantsStereo ? 2 : 1));
        size_t numSamples  = jmin (input.getNumSamples(), output.getNumSamples());

        if (mustInterpolate == false)
        {
            for (size_t channel = 0; channel < numChannels; ++channel)
                engines[(int) channel]->processSamples (input.getChannelPointer (channel), output.getChannelPointer (channel), numSamples);
        }
        else
        {
            auto interpolated = dsp::AudioBlock<float> (interpolationBuffer).getSubBlock (0, numSamples);

            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                auto&& buffer = output.getSingleChannelBlock (channel);

                interpolationBuffer.copyFrom (0, 0, input.getChannelPointer (channel), (int) numSamples);

                engines[(int) channel]->processSamples (input.getChannelPointer (channel), buffer.getChannelPointer (0), numSamples);
                changeVolumes[channel].applyGain (buffer.getChannelPointer (0), (int) numSamples);

                auto* interPtr = interpolationBuffer.getWritePointer (0);
                engines[(int) channel + 2]->processSamples (interPtr, interPtr, numSamples);
                changeVolumes[channel + 2].applyGain (interPtr, (int) numSamples);

                buffer += interpolated;
            }

            if (input.getNumChannels() > 1 && currentInfo.wantsStereo == false)
            {
                auto&& buffer = output.getSingleChannelBlock (1);

                changeVolumes[1].applyGain (buffer.getChannelPointer (0), (int) numSamples);
                changeVolumes[3].applyGain (buffer.getChannelPointer (0), (int) numSamples);
            }

            if (changeVolumes[0].isSmoothing() == false)
            {
                mustInterpolate = false;

                for (auto channel = 0; channel < 2; ++channel)
                    engines[channel]->copyStateFromOtherEngine (*engines[channel + 2]);
            }
        }

        if (input.getNumChannels() > 1 && currentInfo.wantsStereo == false)
            output.getSingleChannelBlock (1).copy (output.getSingleChannelBlock (0));
    }

    //==============================================================================
    const int64 maximumTimeInSamples = 10 * 96000;

private:
    //==============================================================================
    /** This the thread run function which does the preparation of data depending
        on the requested change level.
    */
    void run() override
    {
        if (changeLevel == 2)
        {
            loadImpulseResponse();

            if (isThreadRunning() && threadShouldExit())
                return;
        }

        processImpulseResponse();

        if (isThreadRunning() && threadShouldExit())
            return;

        initializeConvolutionEngines();
    }

    /** Loads the impulse response from the requested audio source. */
    void loadImpulseResponse()
    {
        if (currentInfo.sourceType == SourceType::sourceBinaryData)
        {
            if (! (copyAudioStreamInAudioBuffer (new MemoryInputStream (currentInfo.sourceData, (size_t) currentInfo.sourceDataSize, false))))
                return;
        }
        else if (currentInfo.sourceType == SourceType::sourceAudioFile)
        {
            if (! (copyAudioStreamInAudioBuffer (new FileInputStream (currentInfo.fileImpulseResponse))))
                return;
        }
        else if (currentInfo.sourceType == SourceType::sourceAudioBuffer)
        {
            copyBufferFromTemporaryLocation();
        }
    }

    /** Processes the impulse response data with the requested treatments
        and resampling if needed.
    */
    void processImpulseResponse()
    {
        trimAndResampleImpulseResponse (currentInfo.originalNumChannels, currentInfo.originalSampleRate, currentInfo.wantsTrimming);

        if (isThreadRunning() && threadShouldExit())
            return;

        if (currentInfo.wantsNormalization)
        {
            if (currentInfo.originalNumChannels > 1)
            {
                normalizeImpulseResponse (currentInfo.buffer->getWritePointer (0), (int) currentInfo.finalSize, 1.0);
                normalizeImpulseResponse (currentInfo.buffer->getWritePointer (1), (int) currentInfo.finalSize, 1.0);
            }
            else
            {
                normalizeImpulseResponse (currentInfo.buffer->getWritePointer (0), (int) currentInfo.finalSize, 1.0);
            }
        }

        if (currentInfo.originalNumChannels == 1)
            currentInfo.buffer->copyFrom (1, 0, *currentInfo.buffer, 0, 0, (int) currentInfo.finalSize);
    }

    /** Converts the data from an audio file into a stereo audio buffer of floats, and
        performs resampling if necessary.
    */
    bool copyAudioStreamInAudioBuffer (InputStream* stream)
    {
        AudioFormatManager manager;
        manager.registerBasicFormats();
        std::unique_ptr<AudioFormatReader> formatReader (manager.createReaderFor (stream));

        if (formatReader != nullptr)
        {
            currentInfo.originalNumChannels = formatReader->numChannels > 1 ? 2 : 1;
            currentInfo.originalSampleRate = formatReader->sampleRate;
            currentInfo.originalSize = static_cast<int> (jmin (maximumTimeInSamples, formatReader->lengthInSamples));

            impulseResponseOriginal.clear();
            formatReader->read (&(impulseResponseOriginal), 0, (int) currentInfo.originalSize, 0, true, currentInfo.originalNumChannels > 1);

            return true;
        }

        return false;
    }

    /** Copies a buffer from a temporary location to the impulseResponseOriginal
        buffer for the sourceAudioBuffer.
    */
    void copyBufferFromTemporaryLocation()
    {
        const SpinLock::ScopedLockType sl (processLock);

        for (auto channel = 0; channel < currentInfo.originalNumChannels; ++channel)
            impulseResponseOriginal.copyFrom (channel, 0, temporaryBuffer, channel, 0, (int) currentInfo.originalSize);
    }

    /** Trim and resample the impulse response if needed. */
    void trimAndResampleImpulseResponse (int numChannels, double srcSampleRate, bool mustTrim)
    {
        auto thresholdTrim = Decibels::decibelsToGain (-80.0f);
        auto indexStart = 0;
        auto indexEnd = currentInfo.originalSize - 1;

        if (mustTrim)
        {
            indexStart = currentInfo.originalSize - 1;
            indexEnd = 0;

            for (auto channel = 0; channel < numChannels; ++channel)
            {
                auto localIndexStart = 0;
                auto localIndexEnd = currentInfo.originalSize - 1;

                auto* channelData = impulseResponseOriginal.getReadPointer (channel);

                while (localIndexStart < currentInfo.originalSize - 1
                        && channelData[localIndexStart] <= thresholdTrim
                        && channelData[localIndexStart] >= -thresholdTrim)
                    ++localIndexStart;

                while (localIndexEnd >= 0
                        && channelData[localIndexEnd] <= thresholdTrim
                        && channelData[localIndexEnd] >= -thresholdTrim)
                    --localIndexEnd;

                indexStart = jmin (indexStart, localIndexStart);
                indexEnd = jmax (indexEnd, localIndexEnd);
            }

            if (indexStart > 0)
            {
                for (auto channel = 0; channel < numChannels; ++channel)
                {
                    auto* channelData = impulseResponseOriginal.getWritePointer (channel);

                    for (auto i = 0; i < indexEnd - indexStart + 1; ++i)
                        channelData[i] = channelData[i + indexStart];

                    for (auto i = indexEnd - indexStart + 1; i < currentInfo.originalSize - 1; ++i)
                        channelData[i] = 0.0f;
                }
            }
        }

        if (currentInfo.sampleRate == srcSampleRate)
        {
            // No resampling
            currentInfo.finalSize = jmin (static_cast<int> (currentInfo.wantedSize), indexEnd - indexStart + 1);

            impulseResponse.clear();

            for (auto channel = 0; channel < numChannels; ++channel)
                impulseResponse.copyFrom (channel, 0, impulseResponseOriginal, channel, 0, (int) currentInfo.finalSize);
        }
        else
        {
            // Resampling
            auto factorReading = srcSampleRate / currentInfo.sampleRate;
            currentInfo.finalSize = jmin (static_cast<int> (currentInfo.wantedSize), roundToInt ((indexEnd - indexStart + 1) / factorReading));

            impulseResponse.clear();

            MemoryAudioSource memorySource (impulseResponseOriginal, false);
            ResamplingAudioSource resamplingSource (&memorySource, false, (int) numChannels);

            resamplingSource.setResamplingRatio (factorReading);
            resamplingSource.prepareToPlay ((int) currentInfo.finalSize, currentInfo.sampleRate);

            AudioSourceChannelInfo info;
            info.startSample = 0;
            info.numSamples = (int) currentInfo.finalSize;
            info.buffer = &impulseResponse;

            resamplingSource.getNextAudioBlock (info);
        }

        // Filling the second channel with the first if necessary
        if (numChannels == 1)
            impulseResponse.copyFrom (1, 0, impulseResponse, 0, 0, (int) currentInfo.finalSize);
    }

    /** Normalization of the impulse response based on its energy. */
    void normalizeImpulseResponse (float* samples, int numSamples, double factorResampling) const
    {
        auto magnitude = 0.0f;

        for (auto i = 0; i < numSamples; ++i)
            magnitude += samples[i] * samples[i];

        auto magnitudeInv = 1.0f / (4.0f * std::sqrt (magnitude)) * 0.5f * static_cast <float> (factorResampling);

        for (auto i = 0; i < numSamples; ++i)
            samples[i] *= magnitudeInv;
    }

    // ================================================================================================================
    /** Initializes the convolution engines depending on the provided sizes
        and performs the FFT on the impulse responses.
    */
    void initializeConvolutionEngines()
    {
        if (currentInfo.maximumBufferSize == 0)
            return;

        if (changeLevel == 3)
        {
            for (auto i = 0; i < 2; ++i)
                engines[i]->initializeConvolutionEngine (currentInfo, i);

            mustInterpolate = false;
        }
        else
        {
            for (auto i = 0; i < 2; ++i)
            {
                engines[i + 2]->initializeConvolutionEngine (currentInfo, i);
                engines[i + 2]->reset();

                if (isThreadRunning() && threadShouldExit())
                    return;
            }

            for (auto i = 0; i < 2; ++i)
            {
                changeVolumes[i].setValue (1.0f);
                changeVolumes[i].reset (currentInfo.sampleRate, 0.05);
                changeVolumes[i].setValue (0.0f);

                changeVolumes[i + 2].setValue (0.0f);
                changeVolumes[i + 2].reset (currentInfo.sampleRate, 0.05);
                changeVolumes[i + 2].setValue (1.0f);

            }

            mustInterpolate = true;
        }
    }


    //==============================================================================
    static constexpr int fifoSize = 256;            // the size of the fifo which handles all the change requests
    AbstractFifo abstractFifo;                      // the abstract fifo

    Array<ChangeRequest> fifoRequestsType;          // an array of ChangeRequest
    Array<juce::var> fifoRequestsParameter;         // an array of change parameters

    Array<ChangeRequest> requestsType;              // an array of ChangeRequest
    Array<juce::var> requestsParameter;             // an array of change parameters

    int changeLevel = 0;                            // the current level of requested change in the convolution engine

    //==============================================================================
    ConvolutionEngine::ProcessingInformation currentInfo;  // the information about the impulse response to load

    AudioBuffer<float> temporaryBuffer;             // a temporary buffer that is used when the function copyAndLoadImpulseResponse is called in the main API
    SpinLock processLock;                           // a necessary lock to use with this temporary buffer

    AudioBuffer<float> impulseResponseOriginal;     // a buffer with the original impulse response
    AudioBuffer<float> impulseResponse;             // a buffer with the impulse response trimmed, resampled, resized and normalized

    //==============================================================================
    OwnedArray<ConvolutionEngine> engines;          // the 4 convolution engines being used

    AudioBuffer<float> interpolationBuffer;         // a buffer to do the interpolation between the convolution engines 0-1 and 2-3
    LinearSmoothedValue<float> changeVolumes[4];    // the volumes for each convolution engine during interpolation

    bool mustInterpolate = false;                   // tells if the convolution engines outputs must be currently interpolated

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


//==============================================================================
Convolution::Convolution()
{
    pimpl.reset (new Pimpl());
    pimpl->addToFifo (Convolution::Pimpl::ChangeRequest::changeEngine, juce::var (0));
}

Convolution::~Convolution()
{
}

void Convolution::loadImpulseResponse (const void* sourceData, size_t sourceDataSize,
                                       bool wantsStereo, bool wantsTrimming, size_t size,
                                       bool wantsNormalization)
{
    if (sourceData == nullptr)
        return;

    auto maximumSamples = (size_t) pimpl->maximumTimeInSamples;
    auto wantedSize = (size == 0 ? maximumSamples : jmin (size, maximumSamples));

    Pimpl::ChangeRequest types[] = { Pimpl::ChangeRequest::changeSource,
                                     Pimpl::ChangeRequest::changeImpulseResponseSize,
                                     Pimpl::ChangeRequest::changeStereo,
                                     Pimpl::ChangeRequest::changeTrimming,
                                     Pimpl::ChangeRequest::changeNormalization };

    Array<juce::var> sourceParameter;

    sourceParameter.add (juce::var ((int) ConvolutionEngine::ProcessingInformation::SourceType::sourceBinaryData));
    sourceParameter.add (juce::var (sourceData, sourceDataSize));

    juce::var parameters[] = { juce::var (sourceParameter),
                               juce::var (static_cast<int64> (wantedSize)),
                               juce::var (wantsStereo),
                               juce::var (wantsTrimming),
                               juce::var (wantsNormalization) };

    pimpl->addToFifo (types, parameters, 5);
}

void Convolution::loadImpulseResponse (const File& fileImpulseResponse, bool wantsStereo,
                                       bool wantsTrimming, size_t size, bool wantsNormalization)
{
    if (! fileImpulseResponse.existsAsFile())
        return;

    auto maximumSamples = (size_t) pimpl->maximumTimeInSamples;
    auto wantedSize = (size == 0 ? maximumSamples : jmin (size, maximumSamples));

    Pimpl::ChangeRequest types[] = { Pimpl::ChangeRequest::changeSource,
                                     Pimpl::ChangeRequest::changeImpulseResponseSize,
                                     Pimpl::ChangeRequest::changeStereo,
                                     Pimpl::ChangeRequest::changeTrimming,
                                     Pimpl::ChangeRequest::changeNormalization };

    Array<juce::var> sourceParameter;

    sourceParameter.add (juce::var ((int) ConvolutionEngine::ProcessingInformation::SourceType::sourceAudioFile));
    sourceParameter.add (juce::var (fileImpulseResponse.getFullPathName()));

    juce::var parameters[] = { juce::var (sourceParameter),
                               juce::var (static_cast<int64> (wantedSize)),
                               juce::var (wantsStereo),
                               juce::var (wantsTrimming),
                               juce::var (wantsNormalization) };

    pimpl->addToFifo (types, parameters, 5);
}

void Convolution::copyAndLoadImpulseResponseFromBuffer (AudioBuffer<float>& buffer,
                                                        double bufferSampleRate, bool wantsStereo, bool wantsTrimming, bool wantsNormalization, size_t size)
{
    copyAndLoadImpulseResponseFromBlock (AudioBlock<float> (buffer), bufferSampleRate,
        wantsStereo, wantsTrimming, wantsNormalization, size);
}

void Convolution::copyAndLoadImpulseResponseFromBlock (AudioBlock<float> block, double bufferSampleRate,
                                                       bool wantsStereo, bool wantsTrimming, bool wantsNormalization, size_t size)
{
    jassert (bufferSampleRate > 0);

    if (block.getNumSamples() == 0)
        return;

    auto maximumSamples = (size_t) pimpl->maximumTimeInSamples;
    auto wantedSize = (size == 0 ? maximumSamples : jmin (size, maximumSamples));

    pimpl->copyBufferToTemporaryLocation (block);

    Pimpl::ChangeRequest types[] = { Pimpl::ChangeRequest::changeSource,
                                     Pimpl::ChangeRequest::changeImpulseResponseSize,
                                     Pimpl::ChangeRequest::changeStereo,
                                     Pimpl::ChangeRequest::changeTrimming,
                                     Pimpl::ChangeRequest::changeNormalization };

    Array<juce::var> sourceParameter;
    sourceParameter.add (juce::var ((int) ConvolutionEngine::ProcessingInformation::SourceType::sourceAudioBuffer));
    sourceParameter.add (juce::var (bufferSampleRate));

    juce::var parameters[] = { juce::var (sourceParameter),
                               juce::var (static_cast<int64> (wantedSize)),
                               juce::var (wantsStereo),
                               juce::var (wantsTrimming),
                               juce::var (wantsNormalization) };

    pimpl->addToFifo (types, parameters, 5);
}

void Convolution::prepare (const ProcessSpec& spec)
{
    jassert (isPositiveAndBelow (spec.numChannels, static_cast<uint32> (3))); // only mono and stereo is supported

    Pimpl::ChangeRequest types[] = { Pimpl::ChangeRequest::changeSampleRate,
                                     Pimpl::ChangeRequest::changeMaximumBufferSize };

    juce::var parameters[] = { juce::var (spec.sampleRate),
                               juce::var (static_cast<int> (spec.maximumBlockSize)) };

    pimpl->addToFifo (types, parameters, 2);
    pimpl->initProcessing (static_cast<int> (spec.maximumBlockSize));

    for (size_t channel = 0; channel < spec.numChannels; ++channel)
    {
        volumeDry[channel].reset (spec.sampleRate, 0.05);
        volumeWet[channel].reset (spec.sampleRate, 0.05);
    }

    sampleRate = spec.sampleRate;
    dryBuffer = AudioBlock<float> (dryBufferStorage,
                                   jmin (spec.numChannels, 2u),
                                   spec.maximumBlockSize);

    isActive = true;
}

void Convolution::reset() noexcept
{
    dryBuffer.clear();
    pimpl->reset();
}

void Convolution::processSamples (const AudioBlock<float>& input, AudioBlock<float>& output, bool isBypassed) noexcept
{
    if (! isActive)
        return;

    jassert (input.getNumChannels() == output.getNumChannels());
    jassert (isPositiveAndBelow (input.getNumChannels(), static_cast<size_t> (3))); // only mono and stereo is supported

    auto numChannels = jmin (input.getNumChannels(), (size_t) 2);
    auto numSamples  = jmin (input.getNumSamples(), output.getNumSamples());

    auto dry = dryBuffer.getSubsetChannelBlock (0, numChannels);

    if (volumeDry[0].isSmoothing())
    {
        dry.copy (input);

        for (size_t channel = 0; channel < numChannels; ++channel)
            volumeDry[channel].applyGain (dry.getChannelPointer (channel), (int) numSamples);

        pimpl->processSamples (input, output);

        for (size_t channel = 0; channel < numChannels; ++channel)
            volumeWet[channel].applyGain (output.getChannelPointer (channel), (int) numSamples);

        output += dry;
    }
    else
    {
        if (! currentIsBypassed)
            pimpl->processSamples (input, output);

        if (isBypassed != currentIsBypassed)
        {
            currentIsBypassed = isBypassed;

            for (size_t channel = 0; channel < numChannels; ++channel)
            {
                volumeDry[channel].setValue (isBypassed ? 0.0f : 1.0f);
                volumeDry[channel].reset (sampleRate, 0.05);
                volumeDry[channel].setValue (isBypassed ? 1.0f : 0.0f);

                volumeWet[channel].setValue (isBypassed ? 1.0f : 0.0f);
                volumeWet[channel].reset (sampleRate, 0.05);
                volumeWet[channel].setValue (isBypassed ? 0.0f : 1.0f);
            }
        }
    }
}

} // namespace dsp
} // namespace juce
