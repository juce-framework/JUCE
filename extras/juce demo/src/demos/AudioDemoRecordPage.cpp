/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  13 Nov 2009 3:52:50 pm

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

#include "AudioDemoRecordPage.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

//==============================================================================
/* This is a rough-and-ready circular buffer, used to allow the audio thread to
   push data quickly into a queue, allowing a background thread to come along and
   write it to disk later.
*/
class CircularAudioBuffer
{
public:
    CircularAudioBuffer (const int numChannels, const int numSamples)
        : buffer (numChannels, numSamples)
    {
        clear();
    }

    ~CircularAudioBuffer()
    {
    }

    void clear()
    {
        buffer.clear();

        const ScopedLock sl (bufferLock);
        bufferValidStart = bufferValidEnd = 0;
    }

    void addSamplesToBuffer (const AudioSampleBuffer& sourceBuffer, int numSamples)
    {
        const int bufferSize = buffer.getNumSamples();

        bufferLock.enter();
        int newDataStart = bufferValidEnd;
        int newDataEnd = newDataStart + numSamples;
        const int actualNewDataEnd = newDataEnd;
        bufferValidStart = jmax (bufferValidStart, newDataEnd - bufferSize);
        bufferLock.exit();

        newDataStart %= bufferSize;
        newDataEnd %= bufferSize;

        if (newDataEnd < newDataStart)
        {
            for (int i = jmin (buffer.getNumChannels(), sourceBuffer.getNumChannels()); --i >= 0;)
            {
                buffer.copyFrom (i, newDataStart, sourceBuffer, i, 0, bufferSize - newDataStart);
                buffer.copyFrom (i, 0, sourceBuffer, i, bufferSize - newDataStart, newDataEnd);
            }
        }
        else
        {
            for (int i = jmin (buffer.getNumChannels(), sourceBuffer.getNumChannels()); --i >= 0;)
                buffer.copyFrom (i, newDataStart, sourceBuffer, i, 0, newDataEnd - newDataStart);
        }

        const ScopedLock sl (bufferLock);
        bufferValidEnd = actualNewDataEnd;
    }

    int readSamplesFromBuffer (AudioSampleBuffer& destBuffer, int numSamples)
    {
        const int bufferSize = buffer.getNumSamples();

        bufferLock.enter();
        int availableDataStart = bufferValidStart;
        const int numSamplesDone = jmin (numSamples, bufferValidEnd - availableDataStart);
        int availableDataEnd = availableDataStart + numSamplesDone;
        bufferValidStart = availableDataEnd;
        bufferLock.exit();

        availableDataStart %= bufferSize;
        availableDataEnd %= bufferSize;

        if (availableDataEnd < availableDataStart)
        {
            for (int i = jmin (buffer.getNumChannels(), destBuffer.getNumChannels()); --i >= 0;)
            {
                destBuffer.copyFrom (i, 0, buffer, i, availableDataStart, bufferSize - availableDataStart);
                destBuffer.copyFrom (i, bufferSize - availableDataStart, buffer, i, 0, availableDataEnd);
            }
        }
        else
        {
            for (int i = jmin (buffer.getNumChannels(), destBuffer.getNumChannels()); --i >= 0;)
                destBuffer.copyFrom (i, 0, buffer, i, availableDataStart, numSamplesDone);
        }

        return numSamplesDone;
    }

private:
    CriticalSection bufferLock;
    AudioSampleBuffer buffer;
    int bufferValidStart, bufferValidEnd;
};

//==============================================================================
class AudioRecorder  : public AudioIODeviceCallback,
                       public Thread
{
public:
    AudioRecorder()
        : Thread ("audio recorder"),
          circularBuffer (2, 48000),
          recording (false),
          sampleRate (0)
    {
    }

    ~AudioRecorder()
    {
        stop();
    }

    //==============================================================================
    void startRecording (const File& file)
    {
        stop();

        if (sampleRate > 0)
        {
            fileToRecord = file;
            startThread();

            circularBuffer.clear();
            recording = true;
        }
    }

    void stop()
    {
        recording = false;

        stopThread (5000);
    }

    bool isRecording() const
    {
        return isThreadRunning() && recording;
    }

    //==============================================================================
    void audioDeviceAboutToStart (AudioIODevice* device)
    {
        sampleRate = device->getCurrentSampleRate();
    }

    void audioDeviceStopped()
    {
        sampleRate = 0;
    }

    void audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples)
    {
        if (recording)
        {
            const AudioSampleBuffer incomingData ((float**) inputChannelData, numInputChannels, numSamples);
            circularBuffer.addSamplesToBuffer (incomingData, numSamples);
        }

        // We need to clear the output buffers, in case they're full of junk..
        for (int i = 0; i < numOutputChannels; ++i)
            if (outputChannelData[i] != 0)
                zeromem (outputChannelData[i], sizeof (float) * numSamples);
    }

    //==============================================================================
    void run()
    {
        fileToRecord.deleteFile();

        OutputStream* outStream = fileToRecord.createOutputStream();
        if (outStream == 0)
            return;

        WavAudioFormat wavFormat;
        AudioFormatWriter* writer = wavFormat.createWriterFor (outStream, sampleRate, 1, 16, StringPairArray(), 0);

        if (writer == 0)
        {
            delete outStream;
            return;
        }

        AudioSampleBuffer tempBuffer (2, 8192);

        while (! threadShouldExit())
        {
            int numSamplesReady = circularBuffer.readSamplesFromBuffer (tempBuffer, tempBuffer.getNumSamples());

            if (numSamplesReady > 0)
                tempBuffer.writeToAudioWriter (writer, 0, numSamplesReady);

            Thread::sleep (1);
        }

        delete writer;
    }

    File fileToRecord;
    double sampleRate;
    bool recording;

    CircularAudioBuffer circularBuffer;
};

//[/MiscUserDefs]

//==============================================================================
AudioDemoRecordPage::AudioDemoRecordPage (AudioDeviceManager& deviceManager_)
    : deviceManager (deviceManager_),
      liveAudioDisplayComp (0),
      explanationLabel (0),
      recordButton (0)
{
    addAndMakeVisible (liveAudioDisplayComp = new LiveAudioInputDisplayComp());

    addAndMakeVisible (explanationLabel = new Label (String::empty,
                                                     T("This page demonstrates how to record a wave file from the live audio input..\n\nPressing record will start recording a file in your \"Documents\" folder.")));
    explanationLabel->setFont (Font (15.0000f, Font::plain));
    explanationLabel->setJustificationType (Justification::topLeft);
    explanationLabel->setEditable (false, false, false);
    explanationLabel->setColour (TextEditor::textColourId, Colours::black);
    explanationLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (recordButton = new TextButton (String::empty));
    recordButton->setButtonText (T("Record"));
    recordButton->addButtonListener (this);
    recordButton->setColour (TextButton::buttonColourId, Colour (0xffff5c5c));
    recordButton->setColour (TextButton::textColourId, Colours::black);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);

    //[Constructor] You can add your own custom stuff here..
    recorder = new AudioRecorder();
    deviceManager.addAudioCallback (recorder);
    deviceManager.addAudioCallback (liveAudioDisplayComp);
    //[/Constructor]
}

AudioDemoRecordPage::~AudioDemoRecordPage()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    recorder->stop();
    deviceManager.removeAudioCallback (recorder);
    delete recorder;
    deviceManager.removeAudioCallback (liveAudioDisplayComp);
    //[/Destructor_pre]

    deleteAndZero (liveAudioDisplayComp);
    deleteAndZero (explanationLabel);
    deleteAndZero (recordButton);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AudioDemoRecordPage::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::lightgrey);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioDemoRecordPage::resized()
{
    liveAudioDisplayComp->setBounds (8, 8, getWidth() - 16, 64);
    explanationLabel->setBounds (160, 88, getWidth() - 169, 216);
    recordButton->setBounds (8, 88, 136, 40);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void AudioDemoRecordPage::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == recordButton)
    {
        //[UserButtonCode_recordButton] -- add your button handler code here..
        if (recorder->isRecording())
        {
            recorder->stop();
        }
        else
        {
            File file (File::getSpecialLocation (File::userDocumentsDirectory)
                       .getNonexistentChildFile ("Juce Demo Audio Recording", ".wav"));

            recorder->startRecording (file);
        }

        if (recorder->isRecording())
            recordButton->setButtonText ("Stop");
        else
            recordButton->setButtonText ("Record");

        //[/UserButtonCode_recordButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void AudioDemoRecordPage::visibilityChanged()
{
    //[UserCode_visibilityChanged] -- Add your code here...
    recorder->stop();
    recordButton->setButtonText ("Record");
    //[/UserCode_visibilityChanged]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioDemoRecordPage" componentName=""
                 parentClasses="public Component" constructorParams="AudioDeviceManager&amp; deviceManager_"
                 variableInitialisers="deviceManager (deviceManager_)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330000013" fixedSize="0"
                 initialWidth="600" initialHeight="400">
  <METHODS>
    <METHOD name="visibilityChanged()"/>
  </METHODS>
  <BACKGROUND backgroundColour="ffd3d3d3"/>
  <GENERICCOMPONENT name="" id="7d70eb2617f56220" memberName="liveAudioDisplayComp"
                    virtualName="" explicitFocusOrder="0" pos="8 8 16M 64" class="LiveAudioInputDisplayComp"
                    params=""/>
  <LABEL name="" id="1162fb2599a768b4" memberName="explanationLabel" virtualName=""
         explicitFocusOrder="0" pos="160 88 169M 216" edTextCol="ff000000"
         edBkgCol="0" labelText="This page demonstrates how to record a wave file from the live audio input..&#10;&#10;Pressing record will start recording a file in your &quot;Documents&quot; folder."
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15" bold="0" italic="0" justification="9"/>
  <TEXTBUTTON name="" id="2c10a0ba9fad39da" memberName="recordButton" virtualName=""
              explicitFocusOrder="0" pos="8 88 136 40" bgColOff="ffff5c5c"
              textCol="ff000000" buttonText="Record" connectedEdges="0" needsCallback="1"
              radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
