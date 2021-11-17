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

struct ADSRTests  : public UnitTest
{
    ADSRTests()  : UnitTest ("ADSR", UnitTestCategories::audio)  {}

    void runTest() override
    {
        constexpr double sampleRate = 44100.0;
        const ADSR::Parameters parameters { 0.1f, 0.1f, 0.5f, 0.1f };

        ADSR adsr;
        adsr.setSampleRate (sampleRate);
        adsr.setParameters (parameters);

        beginTest ("Idle");
        {
            adsr.reset();

            expect (! adsr.isActive());
            expectEquals (adsr.getNextSample(), 0.0f);
        }

        beginTest ("Attack");
        {
            adsr.reset();

            adsr.noteOn();
            expect (adsr.isActive());

            auto buffer = getTestBuffer (sampleRate, parameters.attack);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isIncreasing (buffer));
        }

        beginTest ("Decay");
        {
            adsr.reset();

            adsr.noteOn();
            advanceADSR (adsr, roundToInt (parameters.attack * sampleRate));

            auto buffer = getTestBuffer (sampleRate, parameters.decay);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isDecreasing (buffer));
        }

        beginTest ("Sustain");
        {
            adsr.reset();

            adsr.noteOn();
            advanceADSR (adsr, roundToInt ((parameters.attack + parameters.decay + 0.01) * sampleRate));

            auto random = getRandom();

            for (int numTests = 0; numTests < 100; ++numTests)
            {
                const auto sustainLevel = random.nextFloat();
                const auto sustainLength = jmax (0.1f, random.nextFloat());

                adsr.setParameters ({ parameters.attack, parameters.decay, sustainLevel, parameters.release });

                auto buffer = getTestBuffer (sampleRate, sustainLength);
                adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

                expect (isSustained (buffer, sustainLevel));
            }
        }

        beginTest ("Release");
        {
            adsr.reset();

            adsr.noteOn();
            advanceADSR (adsr, roundToInt ((parameters.attack + parameters.decay) * sampleRate));
            adsr.noteOff();

            auto buffer = getTestBuffer (sampleRate, parameters.release);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isDecreasing (buffer));
        }

        beginTest ("Zero-length attack jumps to decay");
        {
            adsr.reset();
            adsr.setParameters ({ 0.0f, parameters.decay, parameters.sustain, parameters.release });

            adsr.noteOn();

            auto buffer = getTestBuffer (sampleRate, parameters.decay);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isDecreasing (buffer));
        }

        beginTest ("Zero-length decay jumps to sustain");
        {
            adsr.reset();
            adsr.setParameters ({ parameters.attack, 0.0f, parameters.sustain, parameters.release });

            adsr.noteOn();
            advanceADSR (adsr, roundToInt (parameters.attack * sampleRate));
            adsr.getNextSample();

            expectEquals (adsr.getNextSample(), parameters.sustain);

            auto buffer = getTestBuffer (sampleRate, 1);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isSustained (buffer, parameters.sustain));
        }

        beginTest ("Zero-length attack and decay jumps to sustain");
        {
            adsr.reset();
            adsr.setParameters ({ 0.0f, 0.0f, parameters.sustain, parameters.release });

            adsr.noteOn();

            expectEquals (adsr.getNextSample(), parameters.sustain);

            auto buffer = getTestBuffer (sampleRate, 1);
            adsr.applyEnvelopeToBuffer (buffer, 0, buffer.getNumSamples());

            expect (isSustained (buffer, parameters.sustain));
        }

        beginTest ("Zero-length release resets to idle");
        {
            adsr.reset();
            adsr.setParameters ({ parameters.attack, parameters.decay, parameters.sustain, 0.0f });

            adsr.noteOn();
            advanceADSR (adsr, roundToInt ((parameters.attack + parameters.decay) * sampleRate));
            adsr.noteOff();

            expect (! adsr.isActive());
        }
    }

    static void advanceADSR (ADSR& adsr, int numSamplesToAdvance)
    {
        while (--numSamplesToAdvance >= 0)
            adsr.getNextSample();
    }

    static AudioBuffer<float> getTestBuffer (double sampleRate, float lengthInSeconds)
    {
        AudioBuffer<float> buffer { 2, roundToInt (lengthInSeconds * sampleRate) };

        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
                buffer.setSample (channel, sample, 1.0f);

        return buffer;
    }

    static bool isIncreasing (const AudioBuffer<float>& b)
    {
        jassert (b.getNumChannels() > 0 && b.getNumSamples() > 0);

        for (int channel = 0; channel < b.getNumChannels(); ++channel)
        {
            float previousSample = -1.0f;

            for (int sample = 0; sample < b.getNumSamples(); ++sample)
            {
                const auto currentSample = b.getSample (channel, sample);

                if (currentSample <= previousSample)
                    return false;

                previousSample = currentSample;
            }
        }

        return true;
    }

    static bool isDecreasing (const AudioBuffer<float>& b)
    {
        jassert (b.getNumChannels() > 0 && b.getNumSamples() > 0);

        for (int channel = 0; channel < b.getNumChannels(); ++channel)
        {
            float previousSample = std::numeric_limits<float>::max();

            for (int sample = 0; sample < b.getNumSamples(); ++sample)
            {
                const auto currentSample = b.getSample (channel, sample);

                if (currentSample >= previousSample)
                    return false;

                previousSample = currentSample;
            }
        }

        return true;
    }

    static bool isSustained (const AudioBuffer<float>& b, float sustainLevel)
    {
        jassert (b.getNumChannels() > 0 && b.getNumSamples() > 0);

        for (int channel = 0; channel < b.getNumChannels(); ++channel)
            if (b.findMinMax (channel, 0, b.getNumSamples()) != Range<float> { sustainLevel, sustainLevel })
                return false;

        return true;
    }
};

static ADSRTests adsrTests;

} // namespace juce
