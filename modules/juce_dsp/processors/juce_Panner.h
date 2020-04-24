/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

enum class PannerRule
{
    linear,
    balanced,
    sin3dB,
    sin4p5dB,
    sin6dB,
    squareRoot3dB,
    squareRoot4p5dB
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
        const auto numSamples        = outputBlock.getNumSamples();

        jassert (inputBlock.getNumSamples() == numSamples);
        ignoreUnused (numSamples);

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

} // namespace dsp
} // namespace juce
