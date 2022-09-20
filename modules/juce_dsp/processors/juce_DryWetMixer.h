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

namespace juce
{
namespace dsp
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

} // namespace dsp
} // namespace juce
