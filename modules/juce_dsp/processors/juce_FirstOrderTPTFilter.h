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

enum class FirstOrderTPTFilterType
{
    lowpass,
    highpass,
    allpass
};

//==============================================================================
/**
    A first order filter class using the TPT (Topology-Preserving Transform) structure.

    This filter can be modulated at high rates without producing audio artefacts. See
    Vadim Zavalishin's documentation about TPT structures for more information.

    Note: Using this class prevents some loud audio artefacts commonly encountered when
    changing the cutoff frequency using of other filter simulation structures and IIR
    filter classes. However, this class may still require additional smoothing for
    cutoff frequency changes.

    see StateVariableFilter, IIRFilter, SmoothedValue

    @tags{DSP}
*/
template <typename SampleType>
class FirstOrderTPTFilter
{
public:
    //==============================================================================
    using Type = FirstOrderTPTFilterType;

    //==============================================================================
    /** Constructor. */
    FirstOrderTPTFilter();

    //==============================================================================
    /** Sets the filter type. */
    void setType (Type newType);

    /** Sets the cutoff frequency of the filter.

        @param newFrequencyHz cutoff frequency in Hz.
    */
    void setCutoffFrequency (SampleType newFrequencyHz);

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

    /** Resets the internal state variables of the filter to a given value. */
    void reset (SampleType newValue);

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
            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample ((int) channel, inputSamples[i]);
        }

       #if JUCE_SNAP_TO_ZERO
        snapToZero();
       #endif
    }

    //==============================================================================
    /** Processes one sample at a time on a given channel. */
    SampleType processSample (int channel, SampleType inputValue);

    /** Ensure that the state variables are rounded to zero if the state
        variables are denormals. This is only needed if you are doing
        sample by sample processing.
    */
    void snapToZero() noexcept;

private:
    //==============================================================================
    void update();

    //==============================================================================
    SampleType G = 0;
    std::vector<SampleType> s1 { 2 };
    double sampleRate = 44100.0;

    //==============================================================================
    Type filterType = Type::lowpass;
    SampleType cutoffFrequency = 1000.0;
};

} // namespace dsp
} // namespace juce
