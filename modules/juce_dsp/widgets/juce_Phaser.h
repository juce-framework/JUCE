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
    A 6 stage phaser that modulates first order all-pass filters to create sweeping
    notches in the magnitude frequency response.

    This audio effect can be controlled with standard phaser parameters: the speed
    and depth of the LFO controlling the frequency response, a mix control, a
    feedback control, and the centre frequency of the modulation.

    @tags{DSP}
*/
template <typename SampleType>
class Phaser
{
public:
    //==============================================================================
    /** Constructor. */
    Phaser();

    //==============================================================================
    /** Sets the rate (in Hz) of the LFO modulating the phaser all-pass filters. This
        rate must be lower than 100 Hz.
    */
    void setRate (SampleType newRateHz);

    /** Sets the volume (between 0 and 1) of the LFO modulating the phaser all-pass
        filters.
    */
    void setDepth (SampleType newDepth);

    /** Sets the centre frequency (in Hz) of the phaser all-pass filters modulation.
    */
    void setCentreFrequency (SampleType newCentreHz);

    /** Sets the feedback volume (between -1 and 1) of the phaser. Negative can be
        used to get specific phaser sounds.
    */
    void setFeedback (SampleType newFeedback);

    /** Sets the amount of dry and wet signal in the output of the phaser (between 0
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

        int numSamplesDown = 0;
        auto counter = updateCounter;

        for (size_t i = 0; i < numSamples; ++i)
        {
            if (counter == 0)
                numSamplesDown++;

            counter++;

            if (counter == maxUpdateCounter)
                counter = 0;
        }

        if (numSamplesDown > 0)
        {
            auto freqBlock = AudioBlock<SampleType> (bufferFrequency).getSubBlock (0, (size_t) numSamplesDown);
            auto contextFreq = ProcessContextReplacing<SampleType> (freqBlock);
            freqBlock.clear();

            osc.process (contextFreq);
            freqBlock.multiplyBy (oscVolume);
        }

        auto* freqSamples = bufferFrequency.getWritePointer (0);

        for (int i = 0; i < numSamplesDown; ++i)
        {
            auto lfo = jlimit (static_cast<SampleType> (0.0),
                               static_cast<SampleType> (1.0),
                               freqSamples[i] + normCentreFrequency);

            freqSamples[i] = mapToLog10 (lfo, static_cast<SampleType> (20.0),
                                         static_cast<SampleType> (jmin (20000.0, 0.49 * sampleRate)));
        }

        auto currentFrequency = filters[0]->getCutoffFrequency();
        dryWet.pushDrySamples (inputBlock);

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            counter = updateCounter;
            int k = 0;

            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
            {
                auto input = inputSamples[i];
                auto output = input - lastOutput[channel];

                if (i == 0 && counter != 0)
                    for (int n = 0; n < numStages; ++n)
                        filters[n]->setCutoffFrequency (currentFrequency);

                if (counter == 0)
                {
                    for (int n = 0; n < numStages; ++n)
                        filters[n]->setCutoffFrequency (freqSamples[k]);

                    k++;
                }

                for (int n = 0; n < numStages; ++n)
                    output = filters[n]->processSample ((int) channel, output);

                outputSamples[i] = output;
                lastOutput[channel] = output * feedbackVolume[channel].getNextValue();

                counter++;

                if (counter == maxUpdateCounter)
                    counter = 0;
            }
        }

        dryWet.mixWetSamples (outputBlock);
        updateCounter = (updateCounter + (int) numSamples) % maxUpdateCounter;
    }

private:
    //==============================================================================
    void update();

    //==============================================================================
    Oscillator<SampleType> osc;
    OwnedArray<FirstOrderTPTFilter<SampleType>> filters;
    SmoothedValue<SampleType, ValueSmoothingTypes::Linear> oscVolume;
    std::vector<SmoothedValue<SampleType, ValueSmoothingTypes::Linear>> feedbackVolume { 2 };
    DryWetMixer<SampleType> dryWet;
    std::vector<SampleType> lastOutput { 2 };
    AudioBuffer<SampleType> bufferFrequency;
    SampleType normCentreFrequency = 0.5;
    double sampleRate = 44100.0;

    int updateCounter = 0;
    static constexpr int maxUpdateCounter = 4;

    SampleType rate = 1.0, depth = 0.5, feedback = 0.0, mix = 0.5;
    SampleType centreFrequency = 1300.0;
    static constexpr int numStages = 6;
};

} // namespace juce::dsp
