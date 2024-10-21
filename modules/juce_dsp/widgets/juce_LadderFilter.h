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

enum class LadderFilterMode
{
    LPF12,  // low-pass  12 dB/octave
    HPF12,  // high-pass 12 dB/octave
    BPF12,  // band-pass 12 dB/octave
    LPF24,  // low-pass  24 dB/octave
    HPF24,  // high-pass 24 dB/octave
    BPF24   // band-pass 24 dB/octave
};

/**
    Multi-mode filter based on the Moog ladder filter.

    @tags{DSP}
*/
template <typename SampleType>
class LadderFilter
{
public:
    //==============================================================================
    using Mode = LadderFilterMode;

    //==============================================================================
    /** Creates an uninitialised filter. Call prepare() before first use. */
    LadderFilter();

    /** Enables or disables the filter. If disabled it will simply pass through the input signal. */
    void setEnabled (bool isEnabled) noexcept    { enabled = isEnabled; }

    /** Sets filter mode. */
    void setMode (Mode newMode) noexcept;

    /** Initialises the filter. */
    void prepare (const ProcessSpec& spec);

    /** Returns the current number of channels. */
    size_t getNumChannels() const noexcept       { return state.size(); }

    /** Resets the internal state variables of the filter. */
    void reset() noexcept;

    /** Sets the cutoff frequency of the filter.

        @param newCutoff cutoff frequency in Hz
    */
    void setCutoffFrequencyHz (SampleType newCutoff) noexcept;

    /** Sets the resonance of the filter.

        @param newResonance a value between 0 and 1; higher values increase the resonance and can result in self oscillation!
    */
    void setResonance (SampleType newResonance) noexcept;

    /** Sets the amount of saturation in the filter.

        @param newDrive saturation amount; it can be any number greater than or equal to one. Higher values result in more distortion.
    */
    void setDrive (SampleType newDrive) noexcept;

    //==============================================================================
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() <= getNumChannels());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (! enabled || context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t n = 0; n < numSamples; ++n)
        {
            updateSmoothers();

            for (size_t ch = 0; ch < numChannels; ++ch)
                outputBlock.getChannelPointer (ch)[n] = processSample (inputBlock.getChannelPointer (ch)[n], ch);
        }
    }

protected:
    //==============================================================================
    SampleType processSample (SampleType inputValue, size_t channelToUse) noexcept;
    void updateSmoothers() noexcept;

private:
    //==============================================================================
    void setSampleRate (SampleType newValue) noexcept;
    void setNumChannels (size_t newValue)   { state.resize (newValue); }
    void updateCutoffFreq() noexcept        { cutoffTransformSmoother.setTargetValue (std::exp (cutoffFreqHz * cutoffFreqScaler)); }
    void updateResonance() noexcept         { scaledResonanceSmoother.setTargetValue (jmap (resonance, SampleType (0.1), SampleType (1.0))); }

    //==============================================================================
    SampleType drive, drive2, gain, gain2, comp;

    static constexpr size_t numStates = 5;
    std::vector<std::array<SampleType, numStates>> state;
    std::array<SampleType, numStates> A;

    SmoothedValue<SampleType> cutoffTransformSmoother, scaledResonanceSmoother;
    SampleType cutoffTransformValue, scaledResonanceValue;

    LookupTableTransform<SampleType> saturationLUT { [] (SampleType x) { return std::tanh (x); },
                                                     SampleType (-5), SampleType (5), 128 };

    SampleType cutoffFreqHz { SampleType (200) };
    SampleType resonance;

    SampleType cutoffFreqScaler;

    Mode mode;
    bool enabled = true;
};

} // namespace juce::dsp
