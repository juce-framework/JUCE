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

enum class DryWetMixingRule
{
    linear,          // dry volume is equal to 1 - wet volume
    balanced,        // both dry and wet are 1 when mix is 0.5, with dry decreasing to 0
                     // above this value and wet decreasing to 0 below it
    sin3dB,          // alternate dry/wet mixing rule using the 3 dB sine panning rule
    sin4p5dB,        // alternate dry/wet mixing rule using the 4.5 dB sine panning rule
    sin6dB,          // alternate dry/wet mixing rule using the 6 dB sine panning rule
    squareRoot3dB,   // alternate dry/wet mixing rule using the regular 3 dB panning rule
    squareRoot4p5dB  // alternate dry/wet mixing rule using the regular 4.5 dB panning rule
};

/**
    A processor to handle dry/wet mixing of two audio signals, where the wet signal
    may have additional latency.

    Once a DryWetMixer object is configured, push the dry samples using pushDrySamples
    and mix into the fully wet samples using mixWetSamples.

    @tags{DSP}
*/
template <typename SampleType>
class DryWetMixer
{
public:
    //==============================================================================
    using MixingRule = DryWetMixingRule;

    //==============================================================================
    /** Default constructor. */
    DryWetMixer();

    /** Constructor. */
    explicit DryWetMixer (int maximumWetLatencyInSamples);

    //==============================================================================
    /** Sets the mix rule. */
    void setMixingRule (MixingRule newRule);

    /** Sets the current dry/wet mix proportion, with 0.0 being full dry and 1.0
        being fully wet.
    */
    void setWetMixProportion (SampleType newWetMixProportion);

    /** Sets the relative latency of the wet signal path compared to the dry signal
        path, and thus the amount of latency compensation that will be added to the
        dry samples in this processor.
    */
    void setWetLatency (SampleType wetLatencyInSamples);

    //==============================================================================
    /** Initialises the processor. */
    void prepare (const ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

    //==============================================================================
    /** Copies the dry path samples into an internal delay line. */
    void pushDrySamples (const AudioBlock<const SampleType> drySamples);

    /** Mixes the supplied wet samples with the latency-compensated dry samples from
        pushDrySamples.

        @param wetSamples    Input:  The AudioBlock references fully wet samples.
                             Output: The AudioBlock references the wet samples mixed
                                     with the latency compensated dry samples.

        @see pushDrySamples
    */
    void mixWetSamples (AudioBlock<SampleType> wetSamples);

private:
    //==============================================================================
    void update();

    //==============================================================================
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> dryVolume, wetVolume;
    DelayLine<SampleType, DelayLineInterpolationTypes::Thiran> dryDelayLine;
    AudioBuffer<SampleType> bufferDry;

    SingleThreadedAbstractFifo fifo;
    SampleType mix = 1.0;
    MixingRule currentMixingRule = MixingRule::linear;
    double sampleRate = 44100.0;
    int maximumWetLatencyInSamples = 0;
};

} // namespace juce::dsp
