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
    Adds a DC offset (voltage bias) to the audio samples.

    This is a useful preprocessor for asymmetric waveshaping when a waveshaper is
    bookended by a bias on input and a DC-offset removing high pass filter on output.

    This is an extremely simple bias implementation that simply adds a value to a signal.
    More complicated bias behaviours exist in real circuits - for your homework ;).

    @tags{DSP}
*/
template <typename FloatType>
class Bias
{
public:
    Bias() noexcept = default;

    //==============================================================================
    /** Sets the DC bias
        @param newBias DC offset in range [-1, 1]
    */
    void setBias (FloatType newBias) noexcept
    {
        jassert (newBias >= static_cast<FloatType> (-1) && newBias <= static_cast<FloatType> (1));
        bias.setTargetValue (newBias);
    }

    //==============================================================================
    /** Returns the DC bias
        @return DC bias, which should be in the range [-1, 1]
    */
    FloatType getBias() const noexcept              { return bias.getTargetValue(); }

    /** Sets the length of the ramp used for smoothing gain changes. */
    void setRampDurationSeconds (double newDurationSeconds) noexcept
    {
        if (! approximatelyEqual (rampDurationSeconds, newDurationSeconds))
        {
            rampDurationSeconds = newDurationSeconds;
            updateRamp();
        }
    }

    double getRampDurationSeconds() const noexcept  { return rampDurationSeconds; }

    //==============================================================================
    /** Called before processing starts */
    void prepare (const ProcessSpec& spec) noexcept
    {
        sampleRate = spec.sampleRate;
        updateRamp();
    }

    void reset() noexcept
    {
        bias.reset (sampleRate, rampDurationSeconds);
    }

    //==============================================================================
    /** Returns the result of processing a single sample. */
    template <typename SampleType>
    SampleType processSample (SampleType inputSample) noexcept
    {
        return inputSample + bias.getNextValue();
    }

    //==============================================================================
    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto&& inBlock  = context.getInputBlock();
        auto&& outBlock = context.getOutputBlock();

        jassert (inBlock.getNumChannels() == outBlock.getNumChannels());
        jassert (inBlock.getNumSamples()  == outBlock.getNumSamples());

        auto len         = inBlock.getNumSamples();
        auto numChannels = inBlock.getNumChannels();

        if (context.isBypassed)
        {
            bias.skip (static_cast<int> (len));

            if (context.usesSeparateInputAndOutputBlocks())
                outBlock.copyFrom (inBlock);

            return;
        }

        if (numChannels == 1)
        {
            auto* src = inBlock.getChannelPointer (0);
            auto* dst = outBlock.getChannelPointer (0);

            for (size_t i = 0; i < len; ++i)
                dst[i] = src[i] + bias.getNextValue();
        }
        else
        {
            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6255 6386)
            auto* biases = static_cast<FloatType*> (alloca (sizeof (FloatType) * len));

            for (size_t i = 0; i < len; ++i)
                biases[i] = bias.getNextValue();

            for (size_t chan = 0; chan < numChannels; ++chan)
                FloatVectorOperations::add (outBlock.getChannelPointer (chan),
                                            inBlock.getChannelPointer (chan),
                                            biases, static_cast<int> (len));
            JUCE_END_IGNORE_WARNINGS_MSVC
        }
    }


private:
    //==============================================================================
    SmoothedValue<FloatType> bias;
    double sampleRate = 0, rampDurationSeconds = 0;

    void updateRamp() noexcept
    {
        if (sampleRate > 0)
            bias.reset (sampleRate, rampDurationSeconds);
    }
};

} // namespace juce::dsp
