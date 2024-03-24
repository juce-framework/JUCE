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

       #if JUCE_DSP_ENABLE_SNAP_TO_ZERO
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

} // namespace juce::dsp
