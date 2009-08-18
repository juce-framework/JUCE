/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../jucedemo_headers.h"


//==============================================================================
/** Our demo synth only has one type of sound, and it's very basic..
*/
class SineWaveSound : public SynthesiserSound
{
public:
    SineWaveSound (const BitArray& midiNotes_)
        : midiNotes (midiNotes_)
    {
    }

    bool appliesToNote (const int midiNoteNumber)
    {
        return midiNotes [midiNoteNumber];
    }

    bool appliesToChannel (const int midiChannel)           { return true; }

private:
    // this will contain the notes that this sound is attached to.
    BitArray midiNotes;
};


//==============================================================================
/** Our demo synth voice just plays a sine wave..
*/
class SineWaveVoice  : public SynthesiserVoice
{
public:
    SineWaveVoice()
        : angleDelta (0.0),
          tailOff (0.0)
    {
    }

    bool canPlaySound (SynthesiserSound* sound)
    {
        return dynamic_cast <SineWaveSound*> (sound) != 0;
    }

    void startNote (const int midiNoteNumber, const float velocity,
                    SynthesiserSound* sound, const int currentPitchWheelPosition)
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        double cyclesPerSecond = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * double_Pi;
    }

    void stopNote (const bool allowTailOff)
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

    void pitchWheelMoved (const int newValue)
    {
        // can't be bothered implementing this for the demo!
    }

    void controllerMoved (const int controllerNumber, const int newValue)
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

//==============================================================================
/** This is an audio source that streams the output of our demo synth.
*/
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
    MidiKeyboardState keyboardState;

    // the synth itself!
    Synthesiser synth;

    //==============================================================================
    SynthAudioSource()
    {
        // we'll be mixing two different types of sound, so here we'll create two
        // sets of note maps, putting each sound on a different octave of the keyboard:
        BitArray sinewaveNotes, samplerNotes;

        int i;
        for (i = 0; i < 128; ++i)
        {
            if (((i / 12) & 1) != 0)
                sinewaveNotes.setBit (i);
            else
                samplerNotes.setBit (i);
        }

        // add a wave sound, which will get applied to some of the notes..
        synth.addSound (new SineWaveSound (sinewaveNotes));

        // give our synth a few voices that can play the wave sound..
        for (i = 4; --i >= 0;)
            synth.addVoice (new SineWaveVoice());

        WavAudioFormat wavFormat;
        AudioFormatReader* audioReader
            = wavFormat.createReaderFor (new MemoryInputStream (BinaryData::cello_wav,
                                                                BinaryData::cello_wavSize,
                                                                false),
                                         true);

        synth.addSound (new SamplerSound (T("demo sound"),
                                          *audioReader,
                                          samplerNotes,
                                          74,   // root midi note
                                          0.1,  // attack time
                                          0.1,  // release time
                                          10.0  // maximum sample length
                                          ));

        delete audioReader;

        // and give the synth some sampler voices to play the sampled sound..
        for (i = 4; --i >= 0;)
            synth.addVoice (new SamplerVoice());
    }

    void prepareToPlay (int samplesPerBlockExpected,
                        double sampleRate)
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

//==============================================================================
class AudioInputWaveformDisplay  : public Component,
                                   public Timer,
                                   public AudioIODeviceCallback
{
public:
    AudioInputWaveformDisplay()
    {
        bufferPos = 0;
        bufferSize = 2048;
        circularBuffer = (float*) juce_calloc (sizeof (float) * bufferSize);
        currentInputLevel = 0.0f;
        numSamplesIn = 0;

        setOpaque (true);
        startTimer (1000 / 50);  // repaint every 1/50 of a second
    }

    ~AudioInputWaveformDisplay()
    {
        juce_free (circularBuffer);
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::black);
        g.setColour (Colours::lightgreen);

        const float halfHeight = getHeight() * 0.5f;

        int bp = bufferPos;

        for (int x = getWidth(); --x >= 0;)
        {
            const int samplesAgo = getWidth() - x;
            const float level = circularBuffer [(bp + bufferSize - samplesAgo) % bufferSize];

            if (level > 0.01f)
                g.drawLine ((float) x, halfHeight - halfHeight * level,
                            (float) x, halfHeight + halfHeight * level);
        }
    }

    void timerCallback()
    {
        repaint();
    }

    void addSample (const float sample)
    {
        currentInputLevel += fabsf (sample);

        const int samplesToAverage = 128;

        if (++numSamplesIn > samplesToAverage)
        {
            circularBuffer [bufferPos++ % bufferSize] = currentInputLevel / samplesToAverage;

            numSamplesIn = 0;
            currentInputLevel = 0.0f;
        }
    }

    void audioDeviceIOCallback (const float** inputChannelData,
                                int totalNumInputChannels,
                                float** outputChannelData,
                                int totalNumOutputChannels,
                                int numSamples)
    {
        for (int i = 0; i < totalNumInputChannels; ++i)
        {
            if (inputChannelData [i] != 0)
            {
                for (int j = 0; j < numSamples; ++j)
                    addSample (inputChannelData [i][j]);

                break;
            }
        }
    }

    void audioDeviceAboutToStart (AudioIODevice*)
    {
        zeromem (circularBuffer, sizeof (float) * bufferSize);
    }

    void audioDeviceStopped()
    {
        zeromem (circularBuffer, sizeof (float) * bufferSize);
    }

private:
    float* circularBuffer;
    float currentInputLevel;
    int volatile bufferPos, bufferSize, numSamplesIn;
};


//==============================================================================
class AudioDemo  : public Component,
                   public FilenameComponentListener,
                   public ButtonListener,
                   public ChangeListener,
                   public AudioIODeviceCallback
{
    //==============================================================================
    FilenameComponent* fileChooser;
    TextButton* playButton;
    TextButton* stopButton;
    TextButton* audioSettingsButton;

    MidiKeyboardComponent* keyboardComponent;
    AudioInputWaveformDisplay* waveformComponent;

    //==============================================================================
    // this wraps the actual audio device
    AudioDeviceManager audioDeviceManager;

    // this allows an audio source to be streamed to the IO device
    AudioSourcePlayer audioSourcePlayer;

    // this controls the playback of a positionable audio stream, handling the
    // starting/stopping and sample-rate conversion
    AudioTransportSource transportSource;

    // this source contains our synth, and generates its output
    SynthAudioSource synthSource;

    // this source is used to mix together the output from our synth source
    // and wave player source
    MixerAudioSource mixerSource;

    // this is the actual stream that's going to read from the audio file.
    AudioFormatReaderSource* currentAudioFileSource;

    File currentFile;

public:
    //==============================================================================
    AudioDemo()
    {
        setName (T("Audio"));

        currentAudioFileSource = 0;

        //==============================================================================
        AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        addAndMakeVisible (fileChooser = new FilenameComponent (T("audiofile"),
                                                                File::nonexistent,
                                                                true, false, false,
                                                                formatManager.getWildcardForAllFormats(),
                                                                String::empty,
                                                                T("(choose a WAV or AIFF file to play)")));
        fileChooser->addListener (this);
        fileChooser->setBrowseButtonText (T("browse"));

        addAndMakeVisible (playButton = new TextButton (T("play"),
                                                        T("click here to play the current audio file")));
        playButton->addButtonListener (this);
        playButton->setColour (TextButton::buttonColourId, Colours::lightgreen);
        playButton->setColour (TextButton::buttonOnColourId, Colours::lightgreen);
        playButton->setConnectedEdges (Button::ConnectedOnRight);

        addAndMakeVisible (stopButton = new TextButton (T("stop"),
                                                        T("click here to play the current audio file")));
        stopButton->addButtonListener (this);
        stopButton->setColour (TextButton::buttonColourId, Colours::red);
        stopButton->setColour (TextButton::buttonOnColourId, Colours::red);
        stopButton->setConnectedEdges (Button::ConnectedOnLeft);

        addAndMakeVisible (audioSettingsButton = new TextButton (T("show audio settings..."),
                                                                 T("click here to change the audio device settings")));
        audioSettingsButton->addButtonListener (this);

        addAndMakeVisible (keyboardComponent = new MidiKeyboardComponent (synthSource.keyboardState,
                                                                          MidiKeyboardComponent::horizontalKeyboard));


        addAndMakeVisible (waveformComponent = new AudioInputWaveformDisplay());

        //==============================================================================
        // register for start/stop messages from the transport source..
        transportSource.addChangeListener (this);

        // and initialise the device manager with no settings so that it picks a
        // default device to use.
        const String error (audioDeviceManager.initialise (1, /* number of input channels */
                                                           2, /* number of output channels */
                                                           0, /* no XML settings.. */
                                                           true  /* select default device on failure */));

        if (error.isNotEmpty())
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         T("Audio Demo"),
                                         T("Couldn't open an output device!\n\n") + error);
        }
        else
        {
            // add the two audio sources to our mixer..
            mixerSource.addInputSource (&transportSource, false);
            mixerSource.addInputSource (&synthSource, false);

            // ..and connect the mixer to our source player.
            audioSourcePlayer.setSource (&mixerSource);

            // start the IO device pulling its data from our callback..
            audioDeviceManager.addAudioCallback (this);

            // and we need to send midi input to our synth for processing
            audioDeviceManager.addMidiInputCallback (String::empty, &synthSource.midiCollector);
        }
    }

    ~AudioDemo()
    {
        audioDeviceManager.removeMidiInputCallback (String::empty, &synthSource.midiCollector);
        audioDeviceManager.removeAudioCallback (this);

        transportSource.removeChangeListener (this);

        transportSource.setSource (0);
        deleteAndZero (currentAudioFileSource);

        audioSourcePlayer.setSource (0);

        deleteAllChildren();
    }

    //==============================================================================
    void audioDeviceIOCallback (const float** inputChannelData,
                                int totalNumInputChannels,
                                float** outputChannelData,
                                int totalNumOutputChannels,
                                int numSamples)
    {
        // pass the audio callback on to our player source, and also the waveform display comp
        audioSourcePlayer.audioDeviceIOCallback (inputChannelData, totalNumInputChannels, outputChannelData, totalNumOutputChannels, numSamples);
        waveformComponent->audioDeviceIOCallback (inputChannelData, totalNumInputChannels, outputChannelData, totalNumOutputChannels, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device)
    {
        audioSourcePlayer.audioDeviceAboutToStart (device);
        waveformComponent->audioDeviceAboutToStart (device);
    }

    void audioDeviceStopped()
    {
        audioSourcePlayer.audioDeviceStopped();
        waveformComponent->audioDeviceStopped();
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        // print some text to explain what state we're in.

        g.setColour (Colours::black);
        g.setFont (14.0f);

        String s;

        if (transportSource.isPlaying())
            s = T("playing");
        else
            s = T("stopped");

        if (currentAudioFileSource == 0)
            s += T(" - no source file selected");
        else
            s += T(" - file: \"") + currentFile.getFullPathName() + T("\"");

        g.drawText (s, 250, 50, getWidth() - 250, 24, Justification::centredLeft, true);
    }

    void resized()
    {
        fileChooser->setBounds (10, 10, getWidth() - 20, 24);
        playButton->setBounds (10, 50, 100, 24);
        stopButton->setBounds (110, 50, 100, 24);
        audioSettingsButton->setBounds (10, 120, 200, 24);
        audioSettingsButton->changeWidthToFitText();
        keyboardComponent->setBounds (10, 200, getWidth() - 20, 60);
        waveformComponent->setBounds (10, 300, 400, 80);

        updateButtons();
    }

    void updateButtons()
    {
        playButton->setEnabled (currentAudioFileSource != 0 && ! transportSource.isPlaying());
        stopButton->setEnabled (transportSource.isPlaying());
        repaint();
    }

    void buttonClicked (Button* button)
    {
        if (button == playButton)
        {
            transportSource.setPosition (0.0);
            transportSource.start();
        }
        else if (button == stopButton)
        {
            transportSource.stop();
        }
        else if (button == audioSettingsButton)
        {
            // Create an AudioDeviceSelectorComponent which contains the audio choice widgets...

            AudioDeviceSelectorComponent audioSettingsComp (audioDeviceManager,
                                                            0, 1,
                                                            2, 2,
                                                            true,
                                                            false,
                                                            true,
                                                            false);

            // ...and show it in a DialogWindow...
            audioSettingsComp.setSize (500, 450);

            DialogWindow::showModalDialog (T("Audio Settings"),
                                           &audioSettingsComp,
                                           this,
                                           Colours::azure,
                                           true);
        }
    }

    void filenameComponentChanged (FilenameComponent*)
    {
        // this is called when the user changes the filename in the file chooser box

        File audioFile (fileChooser->getCurrentFile());

        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (0);
        deleteAndZero (currentAudioFileSource);

        // create a new file source from the file..

        // get a format manager and set it up with the basic types (wav and aiff).
        AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        AudioFormatReader* reader = formatManager.createReaderFor (audioFile);

        if (reader != 0)
        {
            currentFile = audioFile;

            currentAudioFileSource = new AudioFormatReaderSource (reader, true);

            // ..and plug it into our transport source
            transportSource.setSource (currentAudioFileSource,
                                       32768, // tells it to buffer this many samples ahead
                                       reader->sampleRate);
        }

        updateButtons();
    }

    void changeListenerCallback (void*)
    {
        // callback from the transport source to tell us that play has
        // started or stopped, so update our buttons..
        updateButtons();
    }
};


//==============================================================================
Component* createAudioDemo()
{
    return new AudioDemo();
}
