
#ifndef OSCILLATORS_H_INCLUDED
#define OSCILLATORS_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

/**
    Base class for oscillators
*/
class Oscillator : public SynthesiserVoice
{
public:
    Oscillator()
    {
        amplitude.reset (getSampleRate(), 0.1);
        phaseIncrement.reset (getSampleRate(), 0.1);
    }

    virtual ~Oscillator()
    {
    }

    void startNote (int midiNoteNumber, float velocity, SynthesiserSound*, int) override
    {
        frequency = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        phaseIncrement.setValue (((2.0 * double_Pi) * frequency) / sampleRate);
        amplitude.setValue (velocity);

        // Store the initial note and work out the maximum frequency deviations for pitch bend
        initialNote = midiNoteNumber;
        maxFreq = MidiMessage::getMidiNoteInHertz (initialNote + 4) - frequency;
        minFreq = frequency - MidiMessage::getMidiNoteInHertz (initialNote - 4);
    }

    void stopNote (float, bool) override
    {
        clearCurrentNote();
        amplitude.setValue (0.0);
    }

    void pitchWheelMoved (int newValue) override
    {
        // Change the phase increment based on pitch bend amount
        double frequencyOffset = ((newValue > 0 ? maxFreq : minFreq) * (newValue / 127.0));
        phaseIncrement.setValue (((2.0 * double_Pi) * (frequency + frequencyOffset)) / sampleRate);
    }

    void controllerMoved (int, int) override
    {
    }

    void channelPressureChanged (int newChannelPressureValue) override
    {
        // Set the amplitude based on pressure value
        amplitude.setValue (newChannelPressureValue / 127.0);
    }

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        while(--numSamples >= 0)
        {
            double output = getSample() * amplitude.getNextValue();

            for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                outputBuffer.addSample (i, startSample, static_cast<float> (output));

            ++startSample;
        }
    }

    /** Returns the next sample */
    double getSample()
    {
        double output = renderWaveShape (phasePos);

        phasePos += phaseIncrement.getNextValue();

        if (phasePos > (2.0 * double_Pi))
            phasePos -= (2.0 * double_Pi);

        return output;
    }

    /** Subclasses should override this to say whether they can play the given sound */
    virtual bool canPlaySound (SynthesiserSound* sound) override = 0;

    /** Subclasses should override this to render a waveshape */
    virtual double renderWaveShape (const double currentPhase) = 0;

private:
    LinearSmoothedValue<double> amplitude;
    LinearSmoothedValue<double> phaseIncrement;

    double frequency;
	double phasePos = 0.0f;
	double sampleRate = 44100.0;

    int initialNote;
	double maxFreq;
	double minFreq;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oscillator)
};

//==============================================================================
/**
    Sine sound struct - applies to MIDI channel 1
*/
struct SineSound : public SynthesiserSound
{
    SineSound () {}

    bool appliesToNote (int) override { return true; }

    bool appliesToChannel (int midiChannel) override { return (midiChannel == 1); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineSound)
};

/**
    Sine voice struct that renders a sin waveshape
*/
struct SineVoice : public Oscillator
{
    SineVoice() {};

    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<SineSound*> (sound) != nullptr; }

    double renderWaveShape (const double currentPhase) override { return sin (currentPhase); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineVoice)
};

//==============================================================================
/**
    Square sound struct - applies to MIDI channel 2
*/
struct SquareSound : public SynthesiserSound
{
    SquareSound() {}

    bool appliesToNote (int) override { return true; }

    bool appliesToChannel (int midiChannel) override { return (midiChannel == 2); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquareSound)
};

/**
    Square voice struct that renders a square waveshape
*/
struct SquareVoice : public Oscillator
{
    SquareVoice() {};

    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<SquareSound*> (sound) != nullptr; }

    double renderWaveShape (const double currentPhase) override { return (currentPhase < double_Pi ? 0.0 : 1.0); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquareVoice)
};

//==============================================================================
/**
    Sawtooth sound - applies to MIDI channel 3
*/
struct SawSound : public SynthesiserSound
{
    SawSound() {}

    bool appliesToNote (int) override { return true; }

    bool appliesToChannel (int midiChannel) override { return (midiChannel == 3); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SawSound)
};

/**
    Sawtooth voice that renders a sawtooth waveshape
*/
struct SawVoice : public Oscillator
{
    SawVoice() {}

    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<SawSound*> (sound) != nullptr; }

    double renderWaveShape (const double currentPhase) override { return (1.0 / double_Pi) * currentPhase - 1.0; }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SawVoice)
};

//==============================================================================
/**
    Triangle sound - applies to MIDI channel 4
*/
struct TriangleSound : public SynthesiserSound
{
    TriangleSound() {}

    bool appliesToNote (int) override { return true; }

    bool appliesToChannel (int midiChannel) override { return (midiChannel == 4); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriangleSound)
};

/**
    Triangle voice that renders a triangle waveshape
*/
struct TriangleVoice : public Oscillator
{
    TriangleVoice() {}

    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<TriangleSound*> (sound) != nullptr; }

    double renderWaveShape (const double currentPhase) override
    {
        return (currentPhase < double_Pi ? -1.0 + (2.0 / double_Pi) * currentPhase
                                         :  3.0 - (2.0 / double_Pi) * currentPhase);
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriangleVoice)
};

#endif  // OSCILLATORS_H_INCLUDED
