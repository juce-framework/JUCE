/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "AudioDemoSynthPage.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

//==============================================================================
/** Our demo synth sound is just a basic sine wave..
*/
class SineWaveSound : public SynthesiserSound
{
public:
    SineWaveSound()
    {
    }

    bool appliesToNote (const int /*midiNoteNumber*/)           { return true; }
    bool appliesToChannel (const int /*midiChannel*/)           { return true; }
};


//==============================================================================
/** Our demo synth voice just plays a sine wave..
*/
class SineWaveVoice  : public SynthesiserVoice
{
public:
    SineWaveVoice()
       : currentAngle (0), angleDelta (0), level (0), tailOff (0)
    {
    }

    bool canPlaySound (SynthesiserSound* sound)
    {
        return dynamic_cast<SineWaveSound*> (sound) != 0;
    }

    void startNote (int midiNoteNumber, float velocity,
                    SynthesiserSound*, int /*currentPitchWheelPosition*/)
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * double_Pi;
    }

    void stopNote (bool allowTailOff)
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

    void pitchWheelMoved (int /*newValue*/)
    {
        // can't be bothered implementing this for the demo!
    }

    void controllerMoved (int /*controllerNumber*/, int /*newValue*/)
    {
        // not interested in controllers in this case.
    }

    void renderNextBlock (AudioSampleBuffer& outputBuffer, int startSample, int numSamples)
    {
        if (angleDelta != 0.0)
        {
            if (tailOff > 0)
            {
                while (--numSamples >= 0)
                {
                    const float currentSample = (float) (sin (currentAngle) * level * tailOff);

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        *outputBuffer.getSampleData (i, startSample) += currentSample;

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
                    const float currentSample = (float) (sin (currentAngle) * level);

                    for (int i = outputBuffer.getNumChannels(); --i >= 0;)
                        *outputBuffer.getSampleData (i, startSample) += currentSample;

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle, angleDelta, level, tailOff;
};


// This is an audio source that streams the output of our demo synth.
class SynthAudioSource  : public AudioSource
{
public:
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

    //==============================================================================
    SynthAudioSource (MidiKeyboardState& keyboardState_)
        : keyboardState (keyboardState_)
    {
        // add some voices to our synth, to play the sounds..
        for (int i = 4; --i >= 0;)
        {
            synth.addVoice (new SineWaveVoice());   // These voices will play our custom sine-wave sounds..
            synth.addVoice (new SamplerVoice());    // and these ones play the sampled sounds
        }

        // and add some sounds for them to play...
        setUsingSineWaveSound();
    }

    void setUsingSineWaveSound()
    {
        synth.clearSounds();
        synth.addSound (new SineWaveSound());
    }

    void setUsingSampledSound()
    {
        synth.clearSounds();

        WavAudioFormat wavFormat;

        ScopedPointer<AudioFormatReader> audioReader (wavFormat.createReaderFor (new MemoryInputStream (BinaryData::cello_wav,
                                                                                                        BinaryData::cello_wavSize,
                                                                                                        false),
                                                                                 true));

        BigInteger allNotes;
        allNotes.setRange (0, 128, true);

        synth.addSound (new SamplerSound ("demo sound",
                                          *audioReader,
                                          allNotes,
                                          74,   // root midi note
                                          0.1,  // attack time
                                          0.1,  // release time
                                          10.0  // maximum sample length
                                          ));
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate)
    {
        midiCollector.reset (sampleRate);

        synth.setCurrentPlaybackSampleRate (sampleRate);
    }

    void releaseResources()
    {
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
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
};

//[/MiscUserDefs]

//==============================================================================
AudioDemoSynthPage::AudioDemoSynthPage (AudioDeviceManager& deviceManager_)
    : deviceManager (deviceManager_)
{
    addAndMakeVisible (keyboardComponent = new MidiKeyboardComponent (keyboardState, MidiKeyboardComponent::horizontalKeyboard));

    addAndMakeVisible (sineButton = new ToggleButton (String::empty));
    sineButton->setButtonText ("Use sine wave");
    sineButton->setRadioGroupId (321);
    sineButton->addListener (this);
    sineButton->setToggleState (true, dontSendNotification);

    addAndMakeVisible (sampledButton = new ToggleButton (String::empty));
    sampledButton->setButtonText ("Use sampled sound");
    sampledButton->setRadioGroupId (321);
    sampledButton->addListener (this);

    addAndMakeVisible (liveAudioDisplayComp = new LiveAudioInputDisplayComp());


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    deviceManager.addAudioCallback (liveAudioDisplayComp);

    synthAudioSource = new SynthAudioSource (keyboardState);
    audioSourcePlayer.setSource (synthAudioSource);

    deviceManager.addAudioCallback (&audioSourcePlayer);
    deviceManager.addMidiInputCallback (String::empty, &(synthAudioSource->midiCollector));
    //[/Constructor]
}

AudioDemoSynthPage::~AudioDemoSynthPage()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    audioSourcePlayer.setSource (0);
    deviceManager.removeMidiInputCallback (String::empty, &(synthAudioSource->midiCollector));
    deviceManager.removeAudioCallback (&audioSourcePlayer);
    deviceManager.removeAudioCallback (liveAudioDisplayComp);
    //[/Destructor_pre]

    keyboardComponent = nullptr;
    sineButton = nullptr;
    sampledButton = nullptr;
    liveAudioDisplayComp = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AudioDemoSynthPage::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::lightgrey);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioDemoSynthPage::resized()
{
    keyboardComponent->setBounds (8, 96, getWidth() - 16, 64);
    sineButton->setBounds (16, 176, 150, 24);
    sampledButton->setBounds (16, 200, 150, 24);
    liveAudioDisplayComp->setBounds (8, 8, getWidth() - 16, 64);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void AudioDemoSynthPage::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == sineButton)
    {
        //[UserButtonCode_sineButton] -- add your button handler code here..
        synthAudioSource->setUsingSineWaveSound();
        //[/UserButtonCode_sineButton]
    }
    else if (buttonThatWasClicked == sampledButton)
    {
        //[UserButtonCode_sampledButton] -- add your button handler code here..
        synthAudioSource->setUsingSampledSound();
        //[/UserButtonCode_sampledButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioDemoSynthPage" componentName=""
                 parentClasses="public Component" constructorParams="AudioDeviceManager&amp; deviceManager_"
                 variableInitialisers="deviceManager (deviceManager_)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="0"
                 initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffd3d3d3"/>
  <GENERICCOMPONENT name="" id="86605ec4f02c4320" memberName="keyboardComponent"
                    virtualName="" explicitFocusOrder="0" pos="8 96 16M 64" class="MidiKeyboardComponent"
                    params="keyboardState, MidiKeyboardComponent::horizontalKeyboard"/>
  <TOGGLEBUTTON name="" id="d75101df45006ba9" memberName="sineButton" virtualName=""
                explicitFocusOrder="0" pos="16 176 150 24" buttonText="Use sine wave"
                connectedEdges="0" needsCallback="1" radioGroupId="321" state="1"/>
  <TOGGLEBUTTON name="" id="2d687b4ac3dad628" memberName="sampledButton" virtualName=""
                explicitFocusOrder="0" pos="16 200 150 24" buttonText="Use sampled sound"
                connectedEdges="0" needsCallback="1" radioGroupId="321" state="0"/>
  <GENERICCOMPONENT name="" id="7d70eb2617f56220" memberName="liveAudioDisplayComp"
                    virtualName="" explicitFocusOrder="0" pos="8 8 16M 64" class="LiveAudioInputDisplayComp"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
