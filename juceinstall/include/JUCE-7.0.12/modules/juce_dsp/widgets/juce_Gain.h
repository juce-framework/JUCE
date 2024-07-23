/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
