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

namespace juce::dsp
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

} // namespace juce::dsp
