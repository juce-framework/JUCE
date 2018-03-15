/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
    Multi-mode filter based on the Moog ladder filter.

    @tags{DSP}
*/
template <typename Type>
class LadderFilter
{
public:
    enum class Mode
    {
        LPF12,  // low-pass  12 dB/octave
        HPF12,  // high-pass 12 dB/octave
        LPF24,  // low-pass  24 dB/octave
        HPF24   // high-pass 24 dB/octave
    };

    //==============================================================================
    /** Creates an uninitialised filter. Call prepare() before first use. */
    LadderFilter();

    /** Enables or disables the filter. If disabled it will simply pass through the input signal. */
    void setEnabled (bool newValue) noexcept    { enabled = newValue; }

    /** Sets filter mode. */
    void setMode (Mode newValue) noexcept;

    /** Initialises the filter. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Returns the current number of channels. */
    size_t getNumChannels() const noexcept      { return state.size(); }

    /** Resets the internal state variables of the filter. */
    void reset() noexcept;

    /** Sets the cutoff frequency of the filter.
        @param newValue cutoff frequency in Hz */
    void setCutoffFrequencyHz (Type newValue) noexcept;

    /** Sets the resonance of the filter.
        @param newValue a value between 0 and 1; higher values increase the resonance and can result in self oscillation! */
    void setResonance (Type newValue) noexcept;

    /** Sets the amound of saturation in the filter.
        @param newValue saturation amount; it can be any number greater than or equal to one. Higher values result in more distortion.*/
    void setDrive (Type newValue) noexcept;

    //==============================================================================
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() <= getNumChannels());
        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples()  == numSamples);

        if (! enabled || context.isBypassed)
        {
            outputBlock.copy (inputBlock);
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
    Type processSample (Type inputValue, size_t channelToUse) noexcept;
    void updateSmoothers() noexcept;

private:
    //==============================================================================
    Type drive, drive2, gain, gain2, comp;

    static constexpr size_t numStates = 5;
    std::vector<std::array<Type, numStates>> state;
    std::array<Type, numStates> A;

    LinearSmoothedValue<Type> cutoffTransformSmoother;
    LinearSmoothedValue<Type> scaledResonanceSmoother;
    Type cutoffTransformValue;
    Type scaledResonanceValue;

    LookupTableTransform<Type> saturationLUT { [] (Type x) { return std::tanh (x); }, Type (-5), Type (5), 128 };

    Type cutoffFreqHz { Type (200) };
    Type resonance;

    Type cutoffFreqScaler;

    Mode mode;
    bool enabled = true;

    //==============================================================================
    void setSampleRate (Type newValue) noexcept;
    void setNumChannels (size_t newValue)   { state.resize (newValue); }
    void updateCutoffFreq() noexcept        { cutoffTransformSmoother.setValue (std::exp (cutoffFreqHz * cutoffFreqScaler)); }
    void updateResonance() noexcept         { scaledResonanceSmoother.setValue (jmap (resonance, Type (0.1), Type (1.0))); }
};

} // namespace dsp
} // namespace juce
