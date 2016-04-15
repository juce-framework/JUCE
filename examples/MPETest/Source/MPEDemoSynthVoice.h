/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/


#ifndef MPEDEMOSYNTHVOICE_H_INCLUDED
#define MPEDEMOSYNTHVOICE_H_INCLUDED


class MPEDemoSynthVoice : public MPESynthesiserVoice
{
public:
    //==============================================================================
    MPEDemoSynthVoice()
        : phase (0.0), phaseDelta (0.0), tailOff (0.0)
    {
    }

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
        const double cyclesPerSample = frequency.getNextValue() / currentSampleRate;
        phaseDelta = 2.0 * double_Pi * cyclesPerSample;

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
                                // stopNote method could be called more than once.
                tailOff = 1.0;
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
                    const float currentSample = getNextSample() * (float) tailOff;

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
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
                    const float currentSample = getNextSample();

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
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
        const double levelDb = (level.getNextValue() - 1.0) * maxLevelDb;
        const double amplitude = std::pow (10.0f, 0.05f * levelDb) * maxLevel;

        // timbre is used to blend between a sine and a square.
        const double f1 = std::sin (phase);
        const double f2 = std::copysign (1.0, f1);
        const double a2 = timbre.getNextValue();
        const double a1 = 1.0 - a2;

        const float nextSample = float (amplitude * ((a1 * f1) + (a2 * f2)));

        const double cyclesPerSample = frequency.getNextValue() / currentSampleRate;
        phaseDelta = 2.0 * double_Pi * cyclesPerSample;
        phase = std::fmod (phase + phaseDelta, 2.0 * double_Pi);

        return nextSample;
    }

    //==============================================================================
    LinearSmoothedValue<double> level, timbre, frequency;
    double phase, phaseDelta, tailOff;

    const double maxLevel = 0.05f;
    const double maxLevelDb = 31.0f;
    const double smoothingLengthInSeconds = 0.01;
};


#endif  // MPEDEMOSYNTHVOICE_H_INCLUDED
