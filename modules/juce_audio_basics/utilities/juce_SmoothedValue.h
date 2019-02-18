/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A base class for the smoothed value classes.

    This class is used to provide common functionality to the SmoothedValue and
    dsp::LogRampedValue classes.

    @tags{Audio}
*/
template <typename SmoothedValueType>
class SmoothedValueBase
{
private:
    //==============================================================================
    template <typename T> struct FloatTypeHelper;

    template <template <typename> class SmoothedValueClass, typename FloatType>
    struct FloatTypeHelper <SmoothedValueClass <FloatType>>
    {
        using Type = FloatType;
    };

    template <template <typename, typename> class SmoothedValueClass, typename FloatType, typename SmoothingType>
    struct FloatTypeHelper <SmoothedValueClass <FloatType, SmoothingType>>
    {
        using Type = FloatType;
    };

public:
    using FloatType = typename FloatTypeHelper<SmoothedValueType>::Type;

    //==============================================================================
    /** Constructor. */
    SmoothedValueBase() = default;

    virtual ~SmoothedValueBase() {}

    //==============================================================================
    /** Returns true if the current value is currently being interpolated. */
    bool isSmoothing() const noexcept                    { return countdown > 0; }

    /** Returns the current value of the ramp. */
    FloatType getCurrentValue() const noexcept           { return currentValue; }

    //==============================================================================
    /** Returns the target value towards which the smoothed value is currently moving. */
    FloatType getTargetValue() const noexcept            { return target; }

    /** Sets the current value and the target value.
        @param newValue    the new value to take
    */
    void setCurrentAndTargetValue (FloatType newValue)
    {
        target = currentValue = newValue;
        countdown = 0;
    }

    //==============================================================================
    /** Applies a smoothed gain to a stream of samples
        S[i] *= gain
        @param samples Pointer to a raw array of samples
        @param numSamples Length of array of samples
    */
    void applyGain (FloatType* samples, int numSamples) noexcept
    {
        jassert (numSamples >= 0);

        if (isSmoothing())
        {
            for (int i = 0; i < numSamples; ++i)
                samples[i] *= getNextSmoothedValue();
        }
        else
        {
            FloatVectorOperations::multiply (samples, target, numSamples);
        }
    }

    /** Computes output as a smoothed gain applied to a stream of samples.
        Sout[i] = Sin[i] * gain
        @param samplesOut A pointer to a raw array of output samples
        @param samplesIn  A pointer to a raw array of input samples
        @param numSamples The length of the array of samples
    */
    void applyGain (FloatType* samplesOut, const FloatType* samplesIn, int numSamples) noexcept
    {
        jassert (numSamples >= 0);

        if (isSmoothing())
        {
            for (int i = 0; i < numSamples; ++i)
                samplesOut[i] = samplesIn[i] * getNextSmoothedValue();
        }
        else
        {
            FloatVectorOperations::multiply (samplesOut, samplesIn, target, numSamples);
        }
    }

    /** Applies a smoothed gain to a buffer */
    void applyGain (AudioBuffer<FloatType>& buffer, int numSamples) noexcept
    {
        jassert (numSamples >= 0);

        if (isSmoothing())
        {
            if (buffer.getNumChannels() == 1)
            {
                auto* samples = buffer.getWritePointer (0);

                for (int i = 0; i < numSamples; ++i)
                    samples[i] *= getNextSmoothedValue();
            }
            else
            {
                for (auto i = 0; i < numSamples; ++i)
                {
                    auto gain = getNextSmoothedValue();

                    for (int channel = 0; channel < buffer.getNumChannels(); channel++)
                        buffer.setSample (channel, i, buffer.getSample (channel, i) * gain);
                }
            }
        }
        else
        {
            buffer.applyGain (0, numSamples, target);
        }
    }

private:
    //==============================================================================
    FloatType getNextSmoothedValue() noexcept
    {
        return static_cast <SmoothedValueType*> (this)->getNextValue();
    }

protected:
    //==============================================================================
    FloatType currentValue = 0;
    FloatType target = currentValue;
    int countdown = 0;
};

//==============================================================================
/**
    A namespace containing a set of types used for specifying the smoothing
    behaviour of the SmoothedValue class.

    For example:
    @code
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> frequency (1.0f);
    @endcode
*/
namespace ValueSmoothingTypes
{
    /** Used to indicate a linear smoothing between values. */
    struct Linear {};

    /** Used to indicate a smoothing between multiplicative values. */
    struct Multiplicative {};
}

//==============================================================================
/**
    A utility class for values that need smoothing, like volume, that should not
    change abruptly to avoid audio glitches.

    To smooth values spread across an exponential range, where the increments
    between the current and target value are multiplicative (like frequencies),
    you should pass the multiplicative smoothing type as a template paramater:

    @code
    SmoothedValue<float, ValueSmoothingTypes::Multiplicative> yourSmoothedValue;
    @endcode

    Note that when you are using multiplicative smoothing you cannot ever reach a
    target value of zero!

    @tags{Audio}
*/
template <typename FloatType, typename SmoothingType = ValueSmoothingTypes::Linear>
class SmoothedValue   : public SmoothedValueBase <SmoothedValue <FloatType, SmoothingType>>
{
public:
    //==============================================================================
    /** Constructor. */
    SmoothedValue() noexcept
        : SmoothedValue ((FloatType) (std::is_same<SmoothingType, ValueSmoothingTypes::Linear>::value ? 0 : 1))
    {
    }

    /** Constructor. */
    SmoothedValue (FloatType initialValue) noexcept
    {
        // Multiplicative smoothed values cannot ever reach 0!
        jassert (! (std::is_same<SmoothingType, ValueSmoothingTypes::Multiplicative>::value && initialValue == 0));

        // Visual Studio can't handle base class initialisation with CRTP
        this->currentValue = initialValue;
        this->target = this->currentValue;
    }

    //==============================================================================
    /** Reset to a new sample rate and ramp length.
        @param sampleRate           The sample rate
        @param rampLengthInSeconds  The duration of the ramp in seconds
    */
    void reset (double sampleRate, double rampLengthInSeconds) noexcept
    {
        jassert (sampleRate > 0 && rampLengthInSeconds >= 0);
        reset ((int) std::floor (rampLengthInSeconds * sampleRate));
    }

    /** Set a new ramp length directly in samples.
        @param numSteps     The number of samples over which the ramp should be active
    */
    void reset (int numSteps) noexcept
    {
        stepsToTarget = numSteps;
        this->setCurrentAndTargetValue (this->target);
    }

    //==============================================================================
    /** Set the next value to ramp towards.
        @param newValue     The new target value
    */
    void setTargetValue (FloatType newValue) noexcept
    {
        if (newValue == this->target)
            return;

        if (stepsToTarget <= 0)
        {
            this->setCurrentAndTargetValue (newValue);
            return;
        }

        // Multiplicative smoothed values cannot ever reach 0!
        jassert (! (std::is_same<SmoothingType, ValueSmoothingTypes::Multiplicative>::value && newValue == 0));

        this->target = newValue;
        this->countdown = stepsToTarget;

        setStepSize();
    }

    //==============================================================================
    /** Compute the next value.
        @returns Smoothed value
    */
    FloatType getNextValue() noexcept
    {
        if (! this->isSmoothing())
            return this->target;

        --(this->countdown);

        if (this->isSmoothing())
            setNextValue();
        else
            this->currentValue = this->target;

        return this->currentValue;
    }

    //==============================================================================
    /** Skip the next numSamples samples.
        This is identical to calling getNextValue numSamples times. It returns
        the new current value.
        @see getNextValue
    */
    FloatType skip (int numSamples) noexcept
    {
        if (numSamples >= this->countdown)
        {
            this->setCurrentAndTargetValue (this->target);
            return this->target;
        }

        skipCurrentValue (numSamples);

        this->countdown -= numSamples;
        return this->currentValue;
    }

    //==============================================================================
    /** THIS FUNCTION IS DEPRECATED.

        Use `setTargetValue (float)` and `setCurrentAndTargetValue()` instead:

        lsv.setValue (x, false); -> lsv.setTargetValue (x);
        lsv.setValue (x, true);  -> lsv.setCurrentAndTargetValue (x);

        @param newValue     The new target value
        @param force        If true, the value will be set immediately, bypassing the ramp
    */
    JUCE_DEPRECATED_WITH_BODY (void setValue (FloatType newValue, bool force = false) noexcept,
    {
        if (force)
        {
            this->setCurrentAndTargetValue (newValue);
            return;
        }

        setTargetValue (newValue);
    })

private:
    //==============================================================================
    template <typename T>
    using LinearVoid = typename std::enable_if <std::is_same <T, ValueSmoothingTypes::Linear>::value, void>::type;

    template <typename T>
    using MultiplicativeVoid = typename std::enable_if <std::is_same <T, ValueSmoothingTypes::Multiplicative>::value, void>::type;

    //==============================================================================
    template <typename T = SmoothingType>
    LinearVoid<T> setStepSize() noexcept
    {
        step = (this->target - this->currentValue) / (FloatType) this->countdown;
    }

    template <typename T = SmoothingType>
    MultiplicativeVoid<T> setStepSize()
    {
        step = std::exp ((std::log (std::abs (this->target)) - std::log (std::abs (this->currentValue))) / this->countdown);
    }

    //==============================================================================
    template <typename T = SmoothingType>
    LinearVoid<T> setNextValue() noexcept
    {
        this->currentValue += step;
    }

    template <typename T = SmoothingType>
    MultiplicativeVoid<T> setNextValue() noexcept
    {
        this->currentValue *= step;
    }

    //==============================================================================
    template <typename T = SmoothingType>
    LinearVoid<T> skipCurrentValue (int numSamples) noexcept
    {
        this->currentValue += step * (FloatType) numSamples;
    }

    template <typename T = SmoothingType>
    MultiplicativeVoid<T> skipCurrentValue (int numSamples)
    {
        this->currentValue *= (FloatType) std::pow (step, numSamples);
    }

    //==============================================================================
    FloatType step = FloatType();
    int stepsToTarget = 0;
};

template <typename FloatType>
using LinearSmoothedValue = SmoothedValue <FloatType, ValueSmoothingTypes::Linear>;

} // namespace juce
