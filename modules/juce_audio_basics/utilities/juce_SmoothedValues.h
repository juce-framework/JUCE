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

    @tags{Audio}
*/
template <template<typename> class SmoothedValueClass, typename FloatType>
class SmoothedValueBase
{
public:
    //==============================================================================
    /** Constructor. */
    SmoothedValueBase() = default;

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

protected:
    //==============================================================================
    void reset (double sampleRate, double rampLengthInSeconds) noexcept
    {
        jassert (sampleRate > 0 && rampLengthInSeconds >= 0);
        auto& derived = *(static_cast<SmoothedValueClass<FloatType>*> (this));
        derived.reset ((int) std::floor (rampLengthInSeconds * sampleRate));
    }

    //==============================================================================
    FloatType currentValue = 0, target = 0;
    int countdown = 0;

private:
    //==============================================================================
    FloatType getNextSmoothedValue() noexcept
    {
        return static_cast<SmoothedValueClass<FloatType>*> (this)->getNextValue();
    }
};

//==============================================================================
/**
    Utility class for linearly smoothed values like volume etc. that should
    not change abruptly but as a linear ramp to avoid audio glitches.

    @tags{Audio}
*/
template <typename FloatType>
class LinearSmoothedValue   : public SmoothedValueBase<LinearSmoothedValue, FloatType>
{
public:
    //==============================================================================
    /** Constructor. */
    LinearSmoothedValue() = default;

    /** Constructor. */
    LinearSmoothedValue (FloatType initialValue) noexcept
    {
        // Visual Studio can't handle base class initialisation with CRTP
        this->currentValue = initialValue;
        this->target = initialValue;
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

        this->target = newValue;
        this->countdown = stepsToTarget;
        step = (this->target - this->currentValue) / (FloatType) this->countdown;
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

        this->currentValue = this->isSmoothing() ? this->currentValue + step
                                                 : this->target;

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

        this->currentValue += step * (FloatType) numSamples;
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
    FloatType step = FloatType();
    int stepsToTarget = 0;
};

//==============================================================================
/**
    Utility class for logarithmically smoothed values.

    Logarithmically smoothed values can be more relevant than linear ones for
    specific cases such as algorithm change smoothing, using two of them in
    opposite directions.

    @see LinearSmoothedValue

    @tags{Audio}
*/
template <typename FloatType>
class LogSmoothedValue   : public SmoothedValueBase<LogSmoothedValue, FloatType>
{
public:
    //==============================================================================
    /** Constructor. */
    LogSmoothedValue() = default;

    /** Constructor. */
    LogSmoothedValue (FloatType initialValue) noexcept
    {
        // Visual Studio can't handle base class initialisation with CRTP
        this->currentValue = initialValue;
        this->target = initialValue;
    }

    //==============================================================================
    /** Sets the behaviour of the log ramp.
        @param midPointAmplitudedB           Sets the amplitude of the mid point in
                                             decibels, with the target value at 0 dB
                                             and the initial value at -inf dB
        @param rateOfChangeShouldIncrease    If true then the ramp starts shallow
                                             and gets progressively steeper, if false
                                             then the ramp is initially steep and
                                             flattens out as you approach the target
                                             value
    */
    void setLogParameters (FloatType midPointAmplitudedB, bool rateOfChangeShouldIncrease) noexcept
    {
        jassert (midPointAmplitudedB < (FloatType) 0.0);
        B = Decibels::decibelsToGain (midPointAmplitudedB);

        increasingRateOfChange = rateOfChangeShouldIncrease;
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
        @param numSteps             The number of samples over which the ramp should be active
        @param increasingRateOfChange     If the log behaviour makes the ramp increase
                                    slowly at the beginning, rather than at the end
    */
    void reset (int numSteps) noexcept
    {
        stepsToTarget = numSteps;

        this->setCurrentAndTargetValue (this->target);

        updateRampParameters();
    }

    //==============================================================================
    /** Set a new target value.

        @param newValue     The new target value
        @param force        If true, the value will be set immediately, bypassing the ramp
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

        this->target = newValue;
        this->countdown = stepsToTarget;
        source = this->currentValue;

        updateRampParameters();
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

        temp *= r; temp += d;
        this->currentValue = jmap (temp, source, this->target);

        return this->currentValue;
    }

    //==============================================================================
    /** Skip the next numSamples samples.

        This is identical to calling getNextValue numSamples times.
        @see getNextValue
    */
    FloatType skip (int numSamples) noexcept
    {
        if (numSamples >= this->countdown)
        {
            this->setCurrentAndTargetValue (this->target);
            return this->target;
        }

        this->countdown -= numSamples;

        auto rN = std::pow (r, numSamples);
        temp *= rN;
        temp += d * (rN - (FloatType) 1) / (r - (FloatType) 1);

        this->currentValue = jmap (temp, source, this->target);
        return this->currentValue;
    }

private:
    //==============================================================================
    void updateRampParameters()
    {
        auto D = increasingRateOfChange ? B : (FloatType) 1 - B;
        auto base = ((FloatType) 1 / D) - (FloatType) 1;
        r = std::pow (base, (FloatType) 2 / (FloatType) stepsToTarget);
        auto rN = std::pow (r, (FloatType) stepsToTarget);
        d = (r - (FloatType) 1) / (rN - (FloatType) 1);
        temp = 0;
    }

    //==============================================================================
    bool increasingRateOfChange = true;
    FloatType B = Decibels::decibelsToGain ((FloatType) -40);

    int stepsToTarget = 0;
    FloatType temp = 0, source = 0, r = 0, d = 1;
};

} // namespace juce
