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


#pragma once


class MPEDemoSynthVoice : public MPESynthesiserVoice
{
public:
    //==============================================================================
    MPEDemoSynthVoice()  {}

    //==============================================================================
    void noteStarted() override
    {
        jassert (currentlyPlayingNote.isValid());
        jassert (currentlyPlayingNote.keyState == MPENote::keyDown
                 || currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);

        level.setValue (currentlyPlayingNote.pressure.asUnsignedFloat());
        frequency.setValue (currentlyPlayingNote.getFrequencyInHertz());
        timbre.setValue (currentlyPlayingNote.timbre.asUnsignedFloat());

        phase = 0.0;
        auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
        phaseDelta = MathConstants<double>::twoPi * cyclesPerSample;

        tailOff = 0.0;
    }

    void noteStopped (bool allowTailOff) override
    {
        jassert (currentlyPlayingNote.keyState == MPENote::off);

        if (allowTailOff)
        {
            // start a tail-off by setting this flag. The render callback will pick up on
            // this and do a fade out, calling clearCurrentNote() when it's finished.

            if (tailOff == 0.0) // we only need to begin a tail-off if it's not already doing so - the
                tailOff = 1.0;  // stopNote method could be called more than once.
        }
        else
        {
            // we're being told to stop playing immediately, so reset everything..
            clearCurrentNote();
            phaseDelta = 0.0;
        }
    }

    void notePressureChanged() override
    {
        level.setValue (currentlyPlayingNote.pressure.asUnsignedFloat());
    }

    void notePitchbendChanged() override
    {
        frequency.setValue (currentlyPlayingNote.getFrequencyInHertz());
    }

    void noteTimbreChanged() override
    {
        timbre.setValue (currentlyPlayingNote.timbre.asUnsignedFloat());
    }

    void noteKeyStateChanged() override
    {
    }

    void setCurrentSampleRate (double newRate) override
    {
        if (currentSampleRate != newRate)
        {
            noteStopped (false);
            currentSampleRate = newRate;

            level.reset (currentSampleRate, smoothingLengthInSeconds);
            timbre.reset (currentSampleRate, smoothingLengthInSeconds);
            frequency.reset (currentSampleRate, smoothingLengthInSeconds);
        }
    }

    //==============================================================================
    virtual void renderNextBlock (AudioBuffer<float>& outputBuffer,
                                  int startSample,
                                  int numSamples) override
    {
        if (phaseDelta != 0.0)
        {
            if (tailOff > 0)
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = getNextSample() * (float) tailOff;

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();

                        phaseDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    auto currentSample = getNextSample();

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    ++startSample;
                }
            }
        }
    }

private:
    //==============================================================================
    float getNextSample() noexcept
    {
        auto levelDb = (level.getNextValue() - 1.0) * maxLevelDb;
        auto amplitude = std::pow (10.0f, 0.05f * levelDb) * maxLevel;

        // timbre is used to blend between a sine and a square.
        auto f1 = std::sin (phase);
        auto f2 = std::copysign (1.0, f1);
        auto a2 = timbre.getNextValue();
        auto a1 = 1.0 - a2;

        auto nextSample = float (amplitude * ((a1 * f1) + (a2 * f2)));

        auto cyclesPerSample = frequency.getNextValue() / currentSampleRate;
        phaseDelta = MathConstants<double>::twoPi * cyclesPerSample;
        phase = std::fmod (phase + phaseDelta, MathConstants<double>::twoPi);

        return nextSample;
    }

    //==============================================================================
    LinearSmoothedValue<double> level, timbre, frequency;

    double phase = 0.0;
    double phaseDelta = 0.0;
    double tailOff = 0.0;

    const double maxLevel = 0.05f;
    const double maxLevelDb = 31.0f;
    const double smoothingLengthInSeconds = 0.01;
};
