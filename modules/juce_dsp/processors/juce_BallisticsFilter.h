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

enum class BallisticsFilterLevelCalculationType
{
    peak,
    RMS
};

/**
    A processor to apply standard attack / release ballistics to an input signal.
    This is useful in dynamics processors, envelope followers, modulated audio
    effects and for smoothing animation in data visualisation.

    @tags{DSP}
*/
template <typename SampleType>
class BallisticsFilter
{
public:
    //==============================================================================
    using LevelCalculationType = BallisticsFilterLevelCalculationType;

    //==============================================================================
    /** Constructor. */
    BallisticsFilter();

    //==============================================================================
    /** Sets the attack time in ms.

        Attack times less that 0.001 will be snapped to zero and very long attack
        times will eventually saturate depending on the numerical precision used.
    */
    void setAttackTime (SampleType attackTimeMs);

    /** Sets the release time in ms.

        Release times less that 0.001 will be snapped to zero and very long release
        times will eventually saturate depending on the numerical precision used.
    */
    void setReleaseTime (SampleType releaseTimeMs);

    /** Sets how the filter levels are calculated.

        Level calculation in digital envelope followers is usually performed using
        peak detection with a rectifier function (like std::abs) and filtering,
        which returns an envelope dependant on the peak or maximum values of the
        signal amplitude.

        To perform an estimation of the average value of the signal you can use
        an RMS (root mean squared) implementation of the ballistics filter instead.
        This is useful in some compressor and noise-gate designs, or in specific
        types of volume meters.
    */
    void setLevelCalculationType (LevelCalculationType newCalculationType);

    //==============================================================================
    /** Initialises the filter. */
    void prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the filter. */
    void reset();

    /** Resets the internal state variables of the filter to the given initial value. */
    void reset (SampleType initialValue);

    //==============================================================================
    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() <= yold.size());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample ((int) channel, inputSamples[i]);
        }

       #if JUCE_SNAP_TO_ZERO
        snapToZero();
       #endif
    }

    /** Processes one sample at a time on a given channel. */
    SampleType processSample (int channel, SampleType inputValue);

    /** Ensure that the state variables are rounded to zero if the state
        variables are denormals. This is only needed if you are doing
        sample by sample processing.
    */
    void snapToZero() noexcept;

private:
    //==============================================================================
    SampleType calculateLimitedCte (SampleType) const noexcept;

    //==============================================================================
    std::vector<SampleType> yold;
    double sampleRate = 44100.0, expFactor = -0.142;
    SampleType attackTime = 1.0, releaseTime = 100.0, cteAT = 0.0, cteRL = 0.0;
    LevelCalculationType levelType = LevelCalculationType::peak;
};

} // namespace dsp
} // namespace juce
