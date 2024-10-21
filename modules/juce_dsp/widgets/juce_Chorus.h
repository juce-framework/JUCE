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
    A simple chorus DSP widget that modulates the delay of a delay line in order to
    create sweeping notches in the magnitude frequency response.

    This audio effect can be controlled via the speed and depth of the LFO controlling
    the frequency response, a mix control, a feedback control, and the centre delay
    of the modulation.

    Note: To get classic chorus sounds try to use a centre delay time around 7-8 ms
    with a low feedback volume and a low depth. This effect can also be used as a
    flanger with a lower centre delay time and a lot of feedback, and as a vibrato
    effect if the mix value is 1.

    @tags{DSP}
*/
template <typename SampleType>
class Chorus
{
public:
    //==============================================================================
    /** Constructor. */
    Chorus();

    //==============================================================================
    /** Sets the rate (in Hz) of the LFO modulating the chorus delay line. This rate
        must be lower than 100 Hz.
    */
    void setRate (SampleType newRateHz);

    /** Sets the volume of the LFO modulating the chorus delay line (between 0 and 1).
    */
    void setDepth (SampleType newDepth);

    /** Sets the centre delay in milliseconds of the chorus delay line modulation.
        This delay must be between 1 and 100 ms.
    */
    void setCentreDelay (SampleType newDelayMs);

    /** Sets the feedback volume (between -1 and 1) of the chorus delay line.
        Negative values can be used to get specific chorus sounds.
    */
    void setFeedback (SampleType newFeedback);

    /** Sets the amount of dry and wet signal in the output of the chorus (between 0
        for full dry and 1 for full wet).
    */
    void setMix (SampleType newMix);

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
        jassert (inputBlock.getNumChannels() == lastOutput.size());
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        auto delayValuesBlock = AudioBlock<SampleType> (bufferDelayTimes).getSubBlock (0, numSamples);
        auto contextDelay = ProcessContextReplacing<SampleType> (delayValuesBlock);
        delayValuesBlock.clear();

        osc.process (contextDelay);
        delayValuesBlock.multiplyBy (oscVolume);

        auto* delaySamples = bufferDelayTimes.getWritePointer (0);

        for (size_t i = 0; i < numSamples; ++i)
        {
            auto lfo = jmax (static_cast<SampleType> (1.0), maximumDelayModulation * delaySamples[i] + centreDelay);
            delaySamples[i] = static_cast<SampleType> (lfo * sampleRate / 1000.0);
        }

        dryWet.pushDrySamples (inputBlock);

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                auto input = inputSamples[i];
                auto output = input - lastOutput[channel];

                delay.pushSample ((int) channel, output);
                delay.setDelay (delaySamples[i]);
                output = delay.popSample ((int) channel);

                outputSamples[i] = output;
                lastOutput[channel] = output * feedbackVolume[channel].getNextValue();
            }
        }

        dryWet.mixWetSamples (outputBlock);
    }

private:
    //==============================================================================
    void update();

    //==============================================================================
    Oscillator<SampleType> osc;
    DelayLine<SampleType, DelayLineInterpolationTypes::Linear> delay;
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> oscVolume;
    std::vector<SmoothedValue<SampleType, ValueSmoothingTypes::Linear>> feedbackVolume { 2 };
    DryWetMixer<SampleType> dryWet;
    std::vector<SampleType> lastOutput { 2 };
    AudioBuffer<SampleType> bufferDelayTimes;

    double sampleRate = 44100.0;
    SampleType rate = 1.0, depth = 0.25, feedback = 0.0, mix = 0.5,
               centreDelay = 7.0;

    static constexpr SampleType maxDepth               = 1.0,
                                maxCentreDelayMs       = 100.0,
                                oscVolumeMultiplier    = 0.5,
                                maximumDelayModulation = 20.0;
};

} // namespace juce::dsp
