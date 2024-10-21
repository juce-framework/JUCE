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

enum class PannerRule
{
    linear,          // regular 6 dB or linear panning rule, allows the panned sound to be
                     // perceived as having a constant level when summed to mono
    balanced,        // both left and right are 1 when pan value is 0, with left decreasing
                     // to 0 above this value and right decreasing to 0 below it
    sin3dB,          // alternate version of the regular 3 dB panning rule with a sine curve
    sin4p5dB,        // alternate version of the regular 4.5 dB panning rule with a sine curve
    sin6dB,          // alternate version of the regular 6 dB panning rule with a sine curve
    squareRoot3dB,   // regular 3 dB or constant power panning rule, allows the panned sound
                     // to be perceived as having a constant level regardless of the pan position
    squareRoot4p5dB  // regular 4.5 dB panning rule, a compromise option between 3 dB and 6 dB panning rules
};

/**
    A processor to perform panning operations on stereo buffers.

    @tags{DSP}
*/
template <typename SampleType>
class Panner
{
public:
    //==============================================================================
    using Rule = PannerRule;

    //==============================================================================
    /** Constructor. */
    Panner();

    //==============================================================================
    /** Sets the panning rule. */
    void setRule (Rule newRule);

    /** Sets the current panning value, between -1 (full left) and 1 (full right). */
    void setPan (SampleType newPan);

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

        const auto numInputChannels  = inputBlock.getNumChannels();
        const auto numOutputChannels = outputBlock.getNumChannels();
        [[maybe_unused]] const auto numSamples        = outputBlock.getNumSamples();

        jassert (inputBlock.getNumSamples() == numSamples);

        if (numOutputChannels != 2 || numInputChannels == 0 || numInputChannels > 2)
            return;

        if (numInputChannels == 2)
        {
            outputBlock.copyFrom (inputBlock);
        }
        else
        {
            outputBlock.getSingleChannelBlock (0).copyFrom (inputBlock);
            outputBlock.getSingleChannelBlock (1).copyFrom (inputBlock);
        }

        if (context.isBypassed)
            return;

        outputBlock.getSingleChannelBlock (0).multiplyBy (leftVolume);
        outputBlock.getSingleChannelBlock (1).multiplyBy (rightVolume);
    }

private:
    //==============================================================================
    void update();

    //==============================================================================
    Rule currentRule = Rule::balanced;
    SampleType pan = 0.0;
    SmoothedValue<SampleType> leftVolume, rightVolume;
    double sampleRate = 44100.0;
};

} // namespace juce::dsp
