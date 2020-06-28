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
namespace dsp
{

/**
    A simple limiter with standard threshold and release time controls, featuring
    two compressors and a hard clipper at 0 dB.

    @tags{DSP}
*/
template <typename SampleType>
class Limiter
{
public:
    //==============================================================================
    /** Constructor. */
    Limiter() = default;

    //==============================================================================
    /** Sets the threshold in dB of the limiter.*/
    void setThreshold (SampleType newThreshold);

    /** Sets the release time in milliseconds of the limiter.*/
    void setRelease (SampleType newRelease);

    //==============================================================================
    /** Initialises the processor. */
    void prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

    //==============================================================================
    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        firstStageCompressor.process (context);

        auto secondContext = ProcessContextReplacing<SampleType> (outputBlock);
        secondStageCompressor.process (secondContext);

        outputBlock.multiplyBy (outputVolume);

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            FloatVectorOperations::clip (outputBlock.getChannelPointer (channel), outputBlock.getChannelPointer (channel),
                                         (SampleType) -1.0, (SampleType) 1.0, (int) numSamples);
        }
    }

private:
    //==============================================================================
    void update();

    //==============================================================================
    Compressor<SampleType> firstStageCompressor, secondStageCompressor;
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> outputVolume;

    double sampleRate = 44100.0;
    SampleType thresholddB = -10.0, releaseTime = 100.0;
};

} // namespace dsp
} // namespace juce
