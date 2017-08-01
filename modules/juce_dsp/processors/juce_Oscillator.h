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


/**
    Generates a signal based on a user-supplied function.
*/
template <typename SampleType>
class Oscillator
{
public:
    /** The NumericType is the underlying primitive type used by the SampleType (which
        could be either a primitive or vector)
    */
    using NumericType = typename SampleTypeHelpers::ElementType<SampleType>::Type;

    /** Creates an oscillator with a periodic input function (-pi..pi).

        If lookup table is not zero, then the function will be approximated
        with a lookup table.
    */
    Oscillator (const std::function<NumericType (NumericType)>& function, size_t lookupTableNumPoints = 0)
        : generator (function), frequency (440.0f)
    {
        if (lookupTableNumPoints != 0)
        {
            auto table = new LookupTableTransform<NumericType> (generator, static_cast <NumericType> (-1.0 * double_Pi),
                                                                static_cast<NumericType> (double_Pi), lookupTableNumPoints);

            lookupTable = table;
            generator = [table] (NumericType x) { return (*table) (x); };
        }
    }

    //==============================================================================
    /** Sets the frequency of the oscillator. */
    void setFrequency (NumericType newGain) noexcept             { frequency.setValue (newGain); }

    /** Returns the current frequency of the oscillator. */
    NumericType getFrequency() const noexcept                    { return frequency.getTargetValue(); }

    //==============================================================================
    /** Called before processing starts. */
    void prepare (const ProcessSpec& spec) noexcept
    {
        sampleRate = static_cast<NumericType> (spec.sampleRate);
        rampBuffer.resize ((int) spec.maximumBlockSize);

        reset();
    }

    /** Resets the internal state of the oscillator */
    void reset() noexcept
    {
        pos = 0.0;

        if (sampleRate > 0)
            frequency.reset (sampleRate, 0.05);
    }

    //==============================================================================
    /** Returns the result of processing a single sample. */
    SampleType JUCE_VECTOR_CALLTYPE processSample (SampleType) noexcept
    {
        auto increment = static_cast<NumericType> (2.0 * double_Pi) * frequency.getNextValue() / sampleRate;
        auto value = generator (pos - static_cast<NumericType> (double_Pi));
        pos = std::fmod (pos + increment, static_cast<NumericType> (2.0 * double_Pi));

        return value;
    }

    /** Processes the input and output buffers supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        auto&& outBlock = context.getOutputBlock();

        // this is an output-only processory
        jassert (context.getInputBlock().getNumChannels() == 0 || (! context.usesSeparateInputAndOutputBlocks()));
        jassert (outBlock.getNumSamples() <= static_cast<size_t> (rampBuffer.size()));

        auto len           = outBlock.getNumSamples();
        auto numChannels   = outBlock.getNumChannels();
        auto baseIncrement = static_cast<NumericType> (2.0 * double_Pi) / sampleRate;

        if (frequency.isSmoothing())
        {
            auto* buffer = rampBuffer.getRawDataPointer();

            for (size_t i = 0; i < len; ++i)
            {
                buffer[i] = pos - static_cast<NumericType> (double_Pi);

                pos = std::fmod (pos + (baseIncrement * frequency.getNextValue()), static_cast<NumericType> (2.0 * double_Pi));
            }

            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                auto* dst = outBlock.getChannelPointer (ch);

                for (size_t i = 0; i < len; ++i)
                    dst[i] = generator (buffer[i]);
            }
        }
        else
        {
            auto freq = baseIncrement * frequency.getNextValue();

            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                auto p = pos;
                auto* dst = outBlock.getChannelPointer (ch);

                for (size_t i = 0; i < len; ++i)
                {
                    dst[i] = generator (p - static_cast<NumericType> (double_Pi));
                    p = std::fmod (p + freq, static_cast<NumericType> (2.0 * double_Pi));
                }
            }

            pos = std::fmod (pos + freq * static_cast<NumericType> (len), static_cast<NumericType> (2.0 * double_Pi));
        }
    }

private:
    //==============================================================================
    std::function<NumericType (NumericType)> generator;
    ScopedPointer<LookupTableTransform<NumericType>> lookupTable;
    Array<NumericType> rampBuffer;
    LinearSmoothedValue<NumericType> frequency {static_cast<NumericType> (440.0)};
    NumericType sampleRate = 48000.0, pos = 0.0;
};
