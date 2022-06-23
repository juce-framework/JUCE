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

/**
    Processor wrapper around juce::Reverb for easy integration into ProcessorChain.

    @tags{DSP}
*/
class Reverb
{
public:
    //==============================================================================
    /** Creates an uninitialised Reverb processor. Call prepare() before first use. */
    Reverb() = default;

    //==============================================================================
    using Parameters = juce::Reverb::Parameters;

    /** Returns the reverb's current parameters. */
    const Parameters& getParameters() const noexcept    { return reverb.getParameters(); }

    /** Applies a new set of parameters to the reverb.
        Note that this doesn't attempt to lock the reverb, so if you call this in parallel with
        the process method, you may get artifacts.
    */
    void setParameters (const Parameters& newParams)    { reverb.setParameters (newParams); }

    /** Returns true if the reverb is enabled. */
    bool isEnabled() const noexcept                     { return enabled; }

    /** Enables/disables the reverb. */
    void setEnabled (bool newValue) noexcept            { enabled = newValue; }

    //==============================================================================
    /** Initialises the reverb. */
    void prepare (const ProcessSpec& spec)
    {
        reverb.setSampleRate (spec.sampleRate);
    }

    /** Resets the reverb's internal state. */
    void reset() noexcept
    {
        reverb.reset();
    }

    //==============================================================================
    /** Applies the reverb to a mono or stereo buffer. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numInChannels = inputBlock.getNumChannels();
        const auto numOutChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert (inputBlock.getNumSamples() == numSamples);

        outputBlock.copyFrom (inputBlock);

        if (! enabled || context.isBypassed)
            return;

        if (numInChannels == 1 && numOutChannels == 1)
        {
            reverb.processMono (outputBlock.getChannelPointer (0), (int) numSamples);
        }
        else if (numInChannels == 2 && numOutChannels == 2)
        {
            reverb.processStereo (outputBlock.getChannelPointer (0),
                                  outputBlock.getChannelPointer (1),
                                  (int) numSamples);
        }
        else
        {
            jassertfalse;   // invalid channel configuration
        }
    }

private:
    //==============================================================================
    juce::Reverb reverb;
    bool enabled = true;
};

} // namespace dsp
} // namespace juce
