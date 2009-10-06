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
        createTestSound();
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
                        << latencySamples << " samples (" << String (latencySamples * 1000.0 / sampleRate, 1) << " milliseconds)\n"
                        << "The audio device reports an input latency of "
                        << deviceInputLatency << " samples, output latency of "
                        << deviceOutputLatency << " samples."
                        << "\nSo the corrected latency = "
                        << (latencySamples - deviceInputLatency - deviceOutputLatency)
                        << " samples (" << String ((latencySamples - deviceInputLatency - deviceOutputLatency) * 1000.0 / sampleRate, 2)
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
    double sampleRate;
    bool isRunning;
    TextEditor* resultsBox;
    int deviceInputLatency, deviceOutputLatency;

    Array <int> spikes;

    void createTestSound()
    {
        const int length = ((int) sampleRate) / 4;
        testSound.setSize (1, length);
        testSound.clear();
        float* s = testSound.getSampleData (0, 0);

        Random rand (0);
        rand.setSeedRandomly();

        for (int i = 0; i < length; ++i)
            s[i] = (rand.nextFloat() - rand.nextFloat() + rand.nextFloat() - rand.nextFloat()) * 0.06f;

        spikes.clear();

        int spikePos = 0;
        int spikeDelta = 50;

        while (spikePos < length)
        {
            spikes.add (spikePos);

            s [spikePos] = 0.99f;
            s [spikePos + 1] = -0.99f;

            spikePos += spikeDelta;
            spikeDelta += spikeDelta / 6 + rand.nextInt (5);
        }
    }

    // Searches a buffer for a set of spikes that matches those in the test sound
    int findOffsetOfSpikes (const AudioSampleBuffer& buffer) const
    {
        const float minSpikeLevel = 5.0f;
        const double smooth = 0.975;
        const float* s = buffer.getSampleData (0, 0);
        const int spikeDriftAllowed = 5;

        Array <int> spikesFound (100);
        double runningAverage = 0;
        int lastSpike = 0;

        for (int i = 0; i < buffer.getNumSamples() - 10; ++i)
        {
            const float samp = fabsf (s[i]);

            if (samp > runningAverage * minSpikeLevel && i > lastSpike + 20)
            {
                lastSpike = i;
                spikesFound.add (i);
            }

            runningAverage = runningAverage * smooth + (1.0 - smooth) * samp;
        }

        int bestMatch = -1;
        int bestNumMatches = spikes.size() / 3; // the minimum number of matches required

        if (spikesFound.size() < bestNumMatches)
            return -1;

        for (int offsetToTest = 0; offsetToTest < buffer.getNumSamples() - 2048; ++offsetToTest)
        {
            int numMatchesHere = 0;
            int foundIndex = 0;

            for (int refIndex = 0; refIndex < spikes.size(); ++refIndex)
            {
                const int referenceSpike = spikes.getUnchecked (refIndex) + offsetToTest;
                int spike = 0;

                while ((spike = spikesFound.getUnchecked (foundIndex)) < referenceSpike - spikeDriftAllowed
                         && foundIndex < spikesFound.size() - 1)
                    ++foundIndex;

                if (spike >= referenceSpike - spikeDriftAllowed && spike <= referenceSpike + spikeDriftAllowed)
                    ++numMatchesHere;
            }

            if (numMatchesHere > bestNumMatches)
            {
                bestNumMatches = numMatchesHere;
                bestMatch = offsetToTest;

                if (numMatchesHere == spikes.size())
                    break;
            }
        }

        return bestMatch;
    }

    int calculateLatencySamples() const
    {
        // Detect the sound in both our test sound and the recording of it, and measure the difference
        // in their start times..
        const int referenceStart = findOffsetOfSpikes (testSound);
        jassert (referenceStart >= 0);

        const int recordedStart = findOffsetOfSpikes (recordedSound);

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
