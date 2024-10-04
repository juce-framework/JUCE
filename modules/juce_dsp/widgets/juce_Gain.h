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
    Applies a gain to audio samples as single samples or AudioBlocks.

    @tags{DSP}
*/
template <typename FloatType>
class Gain
{
public:
    Gain() noexcept = default;

    //==============================================================================
    /** Applies a new gain as a linear value. */
    void setGainLinear (FloatType newGain) noexcept             { gain.setTargetValue (newGain); }

    /** Applies a new gain as a decibel value. */
    void setGainDecibels (FloatType newGainDecibels) noexcept   { setGainLinear (Decibels::decibelsToGain<FloatType> (newGainDecibels)); }

    /** Returns the current gain as a linear value. */
    FloatType getGainLinear() const noexcept                    { return gain.getTargetValue(); }

    /** Returns the current gain in decibels. */
    FloatType getGainDecibels() const noexcept                  { return Decibels::gainToDecibels<FloatType> (getGainLinear()); }

    /** Sets the length of the ramp used for smoothing gain changes. */
    void setRampDurationSeconds (double newDurationSeconds) noexcept
    {
        if (! approximatelyEqual (rampDurationSeconds, newDurationSeconds))
        {
            rampDurationSeconds = newDurationSeconds;
            reset();
        }
    }

    /** Returns the ramp duration in seconds. */
    double getRampDurationSeconds() const noexcept              { return rampDurationSeconds; }

    /** Returns true if the current value is currently being interpolated. */
    bool isSmoothing() const noexcept                           { return gain.isSmoothing(); }

    //==============================================================================
    /** Called before processing starts. */
    void prepare (const ProcessSpec& spec) noexcept
    {
        sampleRate = spec.sampleRate;
        reset();
    }

    /** Resets the internal state of the gain */
    void reset() noexcept
    {
        if (sampleRate > 0)
            gain.reset (sampleRate, rampDurationSeconds);
    }

    //==============================================================================
    /** Returns the result of processing a single sample. */
    template <typename SampleType>
    SampleType JUCE_VECTOR_CALLTYPE processSample (SampleType s) noexcept
    {
        return s * gain.getNextValue();
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto&& inBlock  = context.getInputBlock();
        auto&& outBlock = context.getOutputBlock();

        jassert (inBlock.getNumChannels() == outBlock.getNumChannels());
        jassert (inBlock.getNumSamples() == outBlock.getNumSamples());

        auto len         = inBlock.getNumSamples();
        auto numChannels = inBlock.getNumChannels();

        if (context.isBypassed)
        {
            gain.skip (static_cast<int> (len));

            if (context.usesSeparateInputAndOutputBlocks())
                outBlock.copyFrom (inBlock);

            return;
        }

        if (numChannels == 1)
        {
            auto* src = inBlock.getChannelPointer (0);
            auto* dst = outBlock.getChannelPointer (0);

            for (size_t i = 0; i < len; ++i)
                dst[i] = src[i] * gain.getNextValue();
        }
        else
        {
            JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6255 6386)
            auto* gains = static_cast<FloatType*> (alloca (sizeof (FloatType) * len));

            for (size_t i = 0; i < len; ++i)
                gains[i] = gain.getNextValue();
            JUCE_END_IGNORE_WARNINGS_MSVC

            for (size_t chan = 0; chan < numChannels; ++chan)
                FloatVectorOperations::multiply (outBlock.getChannelPointer (chan),
                                                 inBlock.getChannelPointer (chan),
                                                 gains, static_cast<int> (len));
        }
    }

private:
    //==============================================================================
    SmoothedValue<FloatType> gain;
    double sampleRate = 0, rampDurationSeconds = 0;
};

} // namespace juce::dsp
