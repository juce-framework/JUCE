/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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
    A very simple ADSR envelope class.

    To use it, call setSampleRate() with the current sample rate and give it some parameters
    with setParameters() then call getNextSample() to get the envelope value to be applied
    to each audio sample or applyEnvelopeToBuffer() to apply the envelope to a whole buffer.

    @tags{Audio}
*/
class JUCE_API  ADSR
{
public:
    //==============================================================================
    ADSR()
    {
        recalculateRates();
    }

    //==============================================================================
    /**
        Holds the parameters being used by an ADSR object.

        @tags{Audio}
    */
    struct JUCE_API  Parameters
    {
        Parameters() = default;

        Parameters (float attackTimeSeconds,
                    float decayTimeSeconds,
                    float sustainLevel,
                    float releaseTimeSeconds)
            : attack (attackTimeSeconds),
              decay (decayTimeSeconds),
              sustain (sustainLevel),
              release (releaseTimeSeconds)
        {
        }

        float attack = 0.1f, decay = 0.1f, sustain = 1.0f, release = 0.1f;
    };

    /** Sets the parameters that will be used by an ADSR object.

        You must have called setSampleRate() with the correct sample rate before
        this otherwise the values may be incorrect!

        @see getParameters
    */
    void setParameters (const Parameters& newParameters)
    {
        // need to call setSampleRate() first!
        jassert (sampleRate > 0.0);

        parameters = newParameters;
        recalculateRates();
    }

    /** Returns the parameters currently being used by an ADSR object.

        @see setParameters
    */
    const Parameters& getParameters() const noexcept  { return parameters; }

    /** Returns true if the envelope is in its attack, decay, sustain or release stage. */
    bool isActive() const noexcept                    { return state != State::idle; }

    //==============================================================================
    /** Sets the sample rate that will be used for the envelope.

        This must be called before the getNextSample() or setParameters() methods.
    */
    void setSampleRate (double newSampleRate) noexcept
    {
        jassert (newSampleRate > 0.0);
        sampleRate = newSampleRate;
    }

    //==============================================================================
    /** Resets the envelope to an idle state. */
    void reset() noexcept
    {
        envelopeVal = 0.0f;
        state = State::idle;
    }

    /** Starts the attack phase of the envelope. */
    void noteOn() noexcept
    {
        if (attackRate > 0.0f)
        {
            state = State::attack;
        }
        else if (decayRate > 0.0f)
        {
            envelopeVal = 1.0f;
            state = State::decay;
        }
        else
        {
            state = State::sustain;
        }
    }

    /** Starts the release phase of the envelope. */
    void noteOff() noexcept
    {
        if (state != State::idle)
        {
            if (parameters.release > 0.0f)
            {
                releaseRate = (float) (envelopeVal / (parameters.release * sampleRate));
                state = State::release;
            }
            else
            {
                reset();
            }
        }
    }

    //==============================================================================
    /** Returns the next sample value for an ADSR object.

        @see applyEnvelopeToBuffer
    */
    float getNextSample() noexcept
    {
        if (state == State::idle)
            return 0.0f;

        if (state == State::attack)
        {
            envelopeVal += attackRate;

            if (envelopeVal >= 1.0f)
            {
                envelopeVal = 1.0f;
                goToNextState();
            }
        }
        else if (state == State::decay)
        {
            envelopeVal -= decayRate;

            if (envelopeVal <= parameters.sustain)
            {
                envelopeVal = parameters.sustain;
                goToNextState();
            }
        }
        else if (state == State::sustain)
        {
            envelopeVal = parameters.sustain;
        }
        else if (state == State::release)
        {
            envelopeVal -= releaseRate;

            if (envelopeVal <= 0.0f)
                goToNextState();
        }

        return envelopeVal;
    }

    /** This method will conveniently apply the next numSamples number of envelope values
        to an AudioBuffer.

        @see getNextSample
    */
    template <typename FloatType>
    void applyEnvelopeToBuffer (AudioBuffer<FloatType>& buffer, int startSample, int numSamples)
    {
        jassert (startSample + numSamples <= buffer.getNumSamples());

        if (state == State::idle)
        {
            buffer.clear (startSample, numSamples);
            return;
        }

        if (state == State::sustain)
        {
            buffer.applyGain (startSample, numSamples, parameters.sustain);
            return;
        }

        auto numChannels = buffer.getNumChannels();

        while (--numSamples >= 0)
        {
            auto env = getNextSample();

            for (int i = 0; i < numChannels; ++i)
                buffer.getWritePointer (i)[startSample] *= env;

            ++startSample;
        }
    }

private:
    //==============================================================================
    void recalculateRates() noexcept
    {
        auto getRate = [] (float distance, float timeInSeconds, double sr)
        {
            return timeInSeconds > 0.0f ? (float) (distance / (timeInSeconds * sr)) : -1.0f;
        };

        attackRate  = getRate (1.0f, parameters.attack, sampleRate);
        decayRate   = getRate (1.0f - parameters.sustain, parameters.decay, sampleRate);
        releaseRate = getRate (parameters.sustain, parameters.release, sampleRate);

        if ((state == State::attack && attackRate <= 0.0f)
            || (state == State::decay && (decayRate <= 0.0f || envelopeVal <= parameters.sustain))
            || (state == State::release && releaseRate <= 0.0f))
        {
            goToNextState();
        }
    }

    void goToNextState() noexcept
    {
        if (state == State::attack)
            state = (decayRate > 0.0f ? State::decay : State::sustain);
        else if (state == State::decay)
            state = State::sustain;
        else if (state == State::release)
            reset();
    }

    //==============================================================================
    enum class State { idle, attack, decay, sustain, release };

    State state = State::idle;
    Parameters parameters;

    double sampleRate = 44100.0;
    float envelopeVal = 0.0f, attackRate = 0.0f, decayRate = 0.0f, releaseRate = 0.0f;
};

} // namespace juce
