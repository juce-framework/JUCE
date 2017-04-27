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

#include "../JuceDemoHeader.h"
#include "AudioLiveScrollingDisplay.h"

//==============================================================================
/** Our demo synth sound is just a basic sine wave.. */
struct SineWaveSound : public SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote (int /*midiNoteNumber*/) override        { return true; }
    bool appliesToChannel (int /*midiChannel*/) override        { return true; }
};


//==============================================================================
/** Our demo synth voice just plays a sine wave.. */
struct SineWaveVoice  : public SynthesiserVoice
{
    SineWaveVoice()   : currentAngle (0), angleDelta (0), level (0), tailOff (0)
    {
    }

    bool canPlaySound (SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * double_Pi;
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
    {
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
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved (int /*newValue*/) override
    {
        // can't be bothered implementing this for the demo!
    }

    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override
    {
        // not interested in controllers in this case.
    }

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0)
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (std::sin (currentAngle) * level * tailOff);

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99;

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote();

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (std::sin (currentAngle) * level);

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample (i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle, angleDelta, level, tailOff;
};

//==============================================================================
// This is an audio source that streams the output of our demo synth.
struct SynthAudioSource  : public AudioSource
{
    SynthAudioSource (MidiKeyboardState& keyState)  : keyboardState (keyState)
    {
        // Add some voices to our synth, to play the sounds..
        for (int i = 4; --i >= 0;)
        {
            synth.addVoice (new SineWaveVoice());   // These voices will play our custom sine-wave sounds..
            synth.addVoice (new SamplerVoice());    // and these ones play the sampled sounds
        }

        // ..and add a sound for them to play...
        setUsingSineWaveSound();
    }

    void setUsingSineWaveSound()
    {
        synth.clearSounds();
        synth.addSound (new SineWaveSound());
    }

    void setUsingSampledSound()
    {
        WavAudioFormat wavFormat;

        ScopedPointer<AudioFormatReader> audioReader (wavFormat.createReaderFor (new MemoryInputStream (BinaryData::cello_wav,
                                                                                                        BinaryData::cello_wavSize,
                                                                                                        false),
                                                                                 true));

        BigInteger allNotes;
        allNotes.setRange (0, 128, true);

        synth.clearSounds();
        synth.addSound (new SamplerSound ("demo sound",
                                          *audioReader,
                                          allNotes,
                                          74,   // root midi note
                                          0.1,  // attack time
                                          0.1,  // release time
                                          10.0  // maximum sample length
                                          ));
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        midiCollector.reset (sampleRate);

        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void releaseResources() override
    {
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // the synth always adds its output to the audio buffer, so we have to clear it
        // first..
        bufferToFill.clearActiveBufferRegion();

        // fill a midi buffer with incoming messages from the midi input.
        MidiBuffer incomingMidi;
        midiCollector.removeNextBlockOfMessages (incomingMidi, bufferToFill.numSamples);

        // pass these messages to the keyboard state so that it can update the component
        // to show on-screen which keys are being pressed on the physical midi keyboard.
        // This call will also add midi messages to the buffer which were generated by
        // the mouse-clicking on the on-screen keyboard.
        keyboardState.processNextMidiBuffer (incomingMidi, 0, bufferToFill.numSamples, true);

        // and now get the synth to process the midi events and generate its output.
        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, 0, bufferToFill.numSamples);
    }

    //==============================================================================
    // this collects real-time midi messages from the midi input device, and
    // turns them into blocks that we can process in our audio callback
    MidiMessageCollector midiCollector;

    // this represents the state of which keys on our on-screen keyboard are held
    // down. When the mouse is clicked on the keyboard component, this object also
    // generates midi messages for this, which we can pass on to our synth.
    MidiKeyboardState& keyboardState;

    // the synth itself!
    Synthesiser synth;
};

//==============================================================================
class AudioSynthesiserDemo  : public Component,
                              private Button::Listener
{
public:
    AudioSynthesiserDemo()
        : deviceManager (MainAppWindow::getSharedAudioDeviceManager()),
          synthAudioSource (keyboardState),
          keyboardComponent (keyboardState, MidiKeyboardComponent::horizontalKeyboard)
    {
        addAndMakeVisible (keyboardComponent);

        addAndMakeVisible (sineButton);
        sineButton.setButtonText ("Use sine wave");
        sineButton.setRadioGroupId (321);
        sineButton.addListener (this);
        sineButton.setToggleState (true, dontSendNotification);

        addAndMakeVisible (sampledButton);
        sampledButton.setButtonText ("Use sampled sound");
        sampledButton.setRadioGroupId (321);
        sampledButton.addListener (this);

        addAndMakeVisible (liveAudioDisplayComp);

        deviceManager.addAudioCallback (&liveAudioDisplayComp);

        audioSourcePlayer.setSource (&synthAudioSource);

        deviceManager.addAudioCallback (&audioSourcePlayer);
        deviceManager.addMidiInputCallback (String(), &(synthAudioSource.midiCollector));

        setOpaque (true);
        setSize (640, 480);
    }

    ~AudioSynthesiserDemo()
    {
        audioSourcePlayer.setSource (nullptr);
        deviceManager.removeMidiInputCallback (String(), &(synthAudioSource.midiCollector));
        deviceManager.removeAudioCallback (&audioSourcePlayer);
        deviceManager.removeAudioCallback (&liveAudioDisplayComp);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        keyboardComponent.setBounds (8, 96, getWidth() - 16, 64);
        sineButton.setBounds (16, 176, 150, 24);
        sampledButton.setBounds (16, 200, 150, 24);
        liveAudioDisplayComp.setBounds (8, 8, getWidth() - 16, 64);
    }

private:
    AudioDeviceManager& deviceManager;
    MidiKeyboardState keyboardState;
    AudioSourcePlayer audioSourcePlayer;
    SynthAudioSource synthAudioSource;
    MidiKeyboardComponent keyboardComponent;
    ToggleButton sineButton;
    ToggleButton sampledButton;
    LiveScrollingAudioDisplay liveAudioDisplayComp;

    //==============================================================================
    void buttonClicked (Button* buttonThatWasClicked) override
    {
        if (buttonThatWasClicked == &sineButton)
            synthAudioSource.setUsingSineWaveSound();
        else if (buttonThatWasClicked == &sampledButton)
            synthAudioSource.setUsingSampledSound();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSynthesiserDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<AudioSynthesiserDemo> demo ("31 Audio: Synthesisers");
