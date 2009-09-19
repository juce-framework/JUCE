/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  18 Sep 2009 5:32:11 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "AudioDemoLatencyPage.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

//==============================================================================
class LatencyTester  : public AudioIODeviceCallback,
                       public Timer
{
public:
    LatencyTester (TextEditor* resultsBox_)
        : testSound (1, 1),
          recordedSound (1, 1),
          playingSampleNum (0),
          recordedSampleNum (-1),
          sineWaveFrequency (300.0),
          isRunning (false),
          resultsBox (resultsBox_)
    {
    }

    ~LatencyTester()
    {
    }

    //==============================================================================
    void beginTest()
    {
        startTimer (50);

        const ScopedLock sl (lock);
        recordedSound.clear();
        playingSampleNum = recordedSampleNum = 0;
        isRunning = true;
    }

    void timerCallback()
    {
        if (isRunning && recordedSampleNum >= recordedSound.getNumSamples())
        {
            isRunning = false;
            stopTimer();

            // Test has finished, so calculate the result..
            String message;

            const int latencySamples = calculateLatencySamples();

            if (latencySamples >= 0)
            {
                message << "\n\nLatency test results:\n"
                        << latencySamples << " samples (" << (latencySamples * 1000.0 / sampleRate) << " milliseconds)\n"
                        << "The audio device reports an input latency of "
                        << deviceInputLatency << " samples, output latency of "
                        << deviceOutputLatency << " samples."
                        << "\nSo the corrected latency = "
                        << (latencySamples - deviceInputLatency - deviceOutputLatency)
                        << " samples (" << ((latencySamples - deviceInputLatency - deviceOutputLatency) * 1000.0 / sampleRate)
                        << " milliseconds)";
            }
            else
            {
                message = "\n\nCouldn't detect the test signal!!\nMake sure there's no background noise that might be confusing it..";
            }

            resultsBox->setCaretPosition (INT_MAX);
            resultsBox->insertTextAtCursor (message);
            resultsBox->setCaretPosition (INT_MAX);
        }
    }

    //==============================================================================
    void audioDeviceAboutToStart (AudioIODevice* device)
    {
        isRunning = false;
        sampleRate = device->getCurrentSampleRate();
        deviceInputLatency = device->getInputLatencyInSamples();
        deviceOutputLatency = device->getOutputLatencyInSamples();
        playingSampleNum = recordedSampleNum = 0;

        createTestSound();

        recordedSound.setSize (1, (int) (1.5 * sampleRate));
        recordedSound.clear();
    }

    void audioDeviceStopped()
    {
    }

    void audioDeviceIOCallback (const float** inputChannelData,
                                int numInputChannels,
                                float** outputChannelData,
                                int numOutputChannels,
                                int numSamples)
    {
        const ScopedLock sl (lock);

        if (isRunning)
        {
            float* const recordingBuffer = recordedSound.getSampleData (0, 0);
            const float* const playBuffer = testSound.getSampleData (0, 0);

            for (int i = 0; i < numSamples; ++i)
            {
                if (recordedSampleNum < recordedSound.getNumSamples())
                {
                    float inputSamp = 0;
                    for (int j = numInputChannels; --j >= 0;)
                        if (inputChannelData[j] != 0)
                            inputSamp += inputChannelData[j][i];

                    recordingBuffer [recordedSampleNum] = inputSamp;
                }

                ++recordedSampleNum;

                float outputSamp = (playingSampleNum < testSound.getNumSamples()) ? playBuffer [playingSampleNum] : 0;

                for (int j = numOutputChannels; --j >= 0;)
                    if (outputChannelData[j] != 0)
                        outputChannelData[j][i] = outputSamp;

                ++playingSampleNum;
            }
        }
        else
        {
            // We need to clear the output buffers, in case they're full of junk..
            for (int i = 0; i < numOutputChannels; ++i)
                if (outputChannelData[i] != 0)
                    zeromem (outputChannelData[i], sizeof (float) * numSamples);
        }
    }

private:
    AudioSampleBuffer testSound, recordedSound;
    int playingSampleNum, recordedSampleNum;
    CriticalSection lock;
    const double sineWaveFrequency;
    double sampleRate;
    bool isRunning;
    TextEditor* resultsBox;
    int deviceInputLatency, deviceOutputLatency;

    void createTestSound()
    {
        const int length = ((int) sampleRate) / 4;
        testSound.setSize (1, length);
        testSound.clear();
        float* s = testSound.getSampleData (0, 0);

        const double scale = 2.0 * double_Pi / (sampleRate / sineWaveFrequency);
        int n = 0;

        for (int i = 512; i < length; ++i)
            s[i] = 0.95f * sinf ((float) (scale * n++));

        testSound.applyGainRamp (0, length - length / 3, length / 3, 1.0f, 0.0f);
    }

    int findStartSampleOfSineWave (const AudioSampleBuffer& buffer, const double sampleRate) const
    {
        const float* s = buffer.getSampleData (0, 0);

        const double damping = 0.995;
        double avG = 0;

        for (int i = 0; i < buffer.getNumSamples(); i += 2)
        {
            const int num = jmin (512, buffer.getNumSamples() - i);

            const double g = calcGoertzel (s + i, num, sampleRate, sineWaveFrequency);
            avG = avG * damping  + (1.0 - damping) * g * g;

            if (avG > 3)
                return i;
        }

        return -1;
    }

    static double calcGoertzel (const float* const samples, const int numSamples,
                                const double sampleRate, const double frequency) throw()
    {
        double n = 0, n1 = 0;
        const double pi2freqOverRate = (2.0 * double_Pi) * frequency / sampleRate;
        const double multiplier = 2.0 * cos (pi2freqOverRate);

        for (int i = 0; i < numSamples; ++i)
        {
            const double n2 = n1;
            n1 = n;
            n = multiplier * n1 - n2 + samples[i];
        }

        return n - n1 * exp (-pi2freqOverRate);
    }

    int calculateLatencySamples() const
    {
        // Detect the sound in both our test sound and the recording of it, and measure the difference
        // in their start times..
        const int referenceStart = findStartSampleOfSineWave (testSound, sampleRate);
        jassert (referenceStart >= 0);

        const int recordedStart = findStartSampleOfSineWave (recordedSound, sampleRate);

        return (recordedStart < 0) ? -1 : (recordedStart - referenceStart);
    }

    LatencyTester (const LatencyTester&);
    const LatencyTester& operator= (const LatencyTester&);
};


//[/MiscUserDefs]

//==============================================================================
AudioDemoLatencyPage::AudioDemoLatencyPage (AudioDeviceManager& deviceManager_)
    : deviceManager (deviceManager_),
      liveAudioDisplayComp (0),
      startTestButton (0),
      testResultsBox (0)
{
    addAndMakeVisible (liveAudioDisplayComp = new LiveAudioInputDisplayComp());

    addAndMakeVisible (startTestButton = new TextButton (String::empty));
    startTestButton->setButtonText (T("Test Latency"));
    startTestButton->addButtonListener (this);

    addAndMakeVisible (testResultsBox = new TextEditor (String::empty));
    testResultsBox->setMultiLine (true);
    testResultsBox->setReturnKeyStartsNewLine (true);
    testResultsBox->setReadOnly (true);
    testResultsBox->setScrollbarsShown (true);
    testResultsBox->setCaretVisible (false);
    testResultsBox->setPopupMenuEnabled (true);
    testResultsBox->setColour (TextEditor::backgroundColourId, Colour (0x32ffffff));
    testResultsBox->setColour (TextEditor::outlineColourId, Colour (0x1c000000));
    testResultsBox->setColour (TextEditor::shadowColourId, Colour (0x16000000));
    testResultsBox->setText (T("Running this test measures the round-trip latency between the audio output and input devices you\'ve got selected.\n\nIt\'ll play a sound, then try to measure the time at which the sound arrives back at the audio input. Obviously for this to work you need to have your microphone somewhere near your speakers..."));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);

    //[Constructor] You can add your own custom stuff here..
    deviceManager.addAudioCallback (liveAudioDisplayComp);

    latencyTester = new LatencyTester (testResultsBox);
    deviceManager.addAudioCallback (latencyTester);
    //[/Constructor]
}

AudioDemoLatencyPage::~AudioDemoLatencyPage()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    deviceManager.removeAudioCallback (liveAudioDisplayComp);

    deviceManager.removeAudioCallback (latencyTester);
    delete latencyTester;
    //[/Destructor_pre]

    deleteAndZero (liveAudioDisplayComp);
    deleteAndZero (startTestButton);
    deleteAndZero (testResultsBox);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AudioDemoLatencyPage::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::lightgrey);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioDemoLatencyPage::resized()
{
    liveAudioDisplayComp->setBounds (8, 8, getWidth() - 16, 64);
    startTestButton->setBounds (8, getHeight() - 41, 168, 32);
    testResultsBox->setBounds (8, 88, getWidth() - 16, getHeight() - 137);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void AudioDemoLatencyPage::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == startTestButton)
    {
        //[UserButtonCode_startTestButton] -- add your button handler code here..
        latencyTester->beginTest();
        //[/UserButtonCode_startTestButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioDemoLatencyPage" componentName=""
                 parentClasses="public Component" constructorParams="AudioDeviceManager&amp; deviceManager_"
                 variableInitialisers="deviceManager (deviceManager_)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330000013" fixedSize="0"
                 initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffd3d3d3"/>
  <GENERICCOMPONENT name="" id="7d70eb2617f56220" memberName="liveAudioDisplayComp"
                    virtualName="" explicitFocusOrder="0" pos="8 8 16M 64" class="LiveAudioInputDisplayComp"
                    params=""/>
  <TEXTBUTTON name="" id="83ed8fcbf36419df" memberName="startTestButton" virtualName=""
              explicitFocusOrder="0" pos="8 41R 168 32" buttonText="Test Latency"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="" id="95d03a8403bc35da" memberName="testResultsBox" virtualName=""
              explicitFocusOrder="0" pos="8 88 16M 137M" bkgcol="32ffffff"
              outlinecol="1c000000" shadowcol="16000000" initialText="Running this test measures the round-trip latency between the audio output and input devices you've got selected.&#10;&#10;It'll play a sound, then try to measure the time at which the sound arrives back at the audio input. Obviously for this to work you need to have your microphone somewhere near your speakers..."
              multiline="1" retKeyStartsLine="1" readonly="1" scrollbars="1"
              caret="0" popupmenu="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
