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

enum class LinkwitzRileyFilterType
{
    lowpass,
    highpass,
    allpass
};

/**
    A filter class designed to perform multi-band separation using the TPT
    (Topology-Preserving Transform) structure.

    Linkwitz-Riley filters are widely used in audio crossovers that have two outputs,
    a low-pass and a high-pass, such that their sum is equivalent to an all-pass filter
    with a flat magnitude frequency response. The Linkwitz-Riley filters available in
    this class are designed to have a -24 dB/octave slope (LR 4th order).

    @tags{DSP}
*/
template <typename SampleType>
class LinkwitzRileyFilter
{
public:
    //==============================================================================
    using Type = LinkwitzRileyFilterType;

    //==============================================================================
    /** Constructor. */
    LinkwitzRileyFilter();

    //==============================================================================
    /** Sets the filter type. */
    void setType (Type newType);

    /** Sets the cutoff frequency of the filter in Hz. */
    void setCutoffFrequency (SampleType newCutoffFrequencyHz);

    //==============================================================================
    /** Returns the type of the filter. */
    Type getType() const noexcept                      { return filterType; }

    /** Returns the cutoff frequency of the filter. */
    SampleType getCutoffFrequency() const noexcept     { return cutoffFrequency; }

    //==============================================================================
    /** Initialises the filter. */
    void prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the filter. */
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

        jassert (inputBlock.getNumChannels() <= s1.size());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample ((int) channel, inputSamples[i]);
        }

       #if JUCE_DSP_ENABLE_SNAP_TO_ZERO
        snapToZero();
       #endif
    }

    /** Performs the filter operation on a single sample at a time. */
    SampleType processSample (int channel, SampleType inputValue);

    /** Performs the filter operation on a single sample at a time, and returns both
        the low-pass and the high-pass outputs of the TPT structure.
    */
    void processSample (int channel, SampleType inputValue, SampleType &outputLow, SampleType &outputHigh);

    /** Ensure that the state variables are rounded to zero if the state
        variables are denormals. This is only needed if you are doing
        sample by sample processing.
    */
    void snapToZero() noexcept;

private:
    //==============================================================================
    void update();

    //==============================================================================
    SampleType g, R2, h;
    std::vector<SampleType> s1, s2, s3, s4;

    double sampleRate = 44100.0;
    SampleType cutoffFrequency = 2000.0;
    Type filterType = Type::lowpass;
};

} // namespace juce::dsp
