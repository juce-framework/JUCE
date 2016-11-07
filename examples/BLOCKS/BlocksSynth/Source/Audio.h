
#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

#include "Oscillators.h"

/**
    Class to handle the Audio functionality
*/
class Audio : public AudioIODeviceCallback
{
public:
    Audio()
    {
        // Set up the audio device manager
        audioDeviceManager.initialiseWithDefaultDevices (0, 2);
        audioDeviceManager.addAudioCallback (this);

        // Set up the synthesiser and add each of the waveshapes
        synthesiser.clearVoices();
        synthesiser.clearSounds();

        synthesiser.addVoice (new SineVoice());
        synthesiser.addVoice (new SquareVoice());
        synthesiser.addVoice (new SawVoice());
        synthesiser.addVoice (new TriangleVoice());

        synthesiser.addSound (new SineSound());
        synthesiser.addSound (new SquareSound());
        synthesiser.addSound (new SawSound());
        synthesiser.addSound (new TriangleSound());
    }

    ~Audio()
    {
        audioDeviceManager.removeAudioCallback (this);
    }

    /** Audio callback */
    void audioDeviceIOCallback (const float **/*inputChannelData*/, int /*numInputChannels*/,
                                float **outputChannelData, int numOutputChannels, int numSamples) override
    {
        AudioSampleBuffer sampleBuffer = AudioSampleBuffer (outputChannelData, numOutputChannels, numSamples);
        sampleBuffer.clear();

        synthesiser.renderNextBlock (sampleBuffer, MidiBuffer(), 0, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        synthesiser.setCurrentPlaybackSampleRate (device->getCurrentSampleRate());
    }

    void audioDeviceStopped() override {}

    /** Called to turn a synthesiser note on */
    void noteOn (int channel, int noteNum, float velocity)
    {
        synthesiser.noteOn (channel, noteNum, velocity);
    }

    /** Called to turn a synthesiser note off */
    void noteOff (int channel, int noteNum, float velocity)
    {
        synthesiser.noteOff (channel, noteNum, velocity, false);
    }

    /** Called to turn all synthesiser notes off */
    void allNotesOff()
    {
        for (int i = 1; i < 5; ++i)
            synthesiser.allNotesOff (i, false);
    }

    /** Send pressure change message to synthesiser */
    void pressureChange (int channel, float newPressure)
    {
        synthesiser.handleChannelPressure (channel, static_cast<int> (newPressure * 127));
    }

    /** Send pitch change message to synthesiser */
    void pitchChange (int channel, float pitchChange)
    {
        synthesiser.handlePitchWheel (channel, static_cast<int> (pitchChange * 127));
    }

private:
    AudioDeviceManager audioDeviceManager;
    Synthesiser synthesiser;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Audio)
};

#endif  // AUDIO_H_INCLUDED
