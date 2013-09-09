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

#include "AudioDemoRecordPage.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...


//==============================================================================
/** A simple class that acts as an AudioIODeviceCallback and writes the
    incoming audio data to a WAV file.
*/
class AudioRecorder  : public AudioIODeviceCallback
{
public:
    AudioRecorder()
        : backgroundThread ("Audio Recorder Thread"),
          sampleRate (0), activeWriter (0)
    {
        backgroundThread.startThread();
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
            // Create an OutputStream to write to our destination file...
            file.deleteFile();
            ScopedPointer<FileOutputStream> fileStream (file.createOutputStream());

            if (fileStream != 0)
            {
                // Now create a WAV writer object that writes to our output stream...
                WavAudioFormat wavFormat;
                AudioFormatWriter* writer = wavFormat.createWriterFor (fileStream, sampleRate, 1, 16, StringPairArray(), 0);

                if (writer != 0)
                {
                    fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

                    // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                    // write the data to disk on our background thread.
                    threadedWriter = new AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768);

                    // And now, swap over our active writer pointer so that the audio callback will start using it..
                    const ScopedLock sl (writerLock);
                    activeWriter = threadedWriter;
                }
            }
        }
    }

    void stop()
    {
        // First, clear this pointer to stop the audio callback from using our writer object..
        {
            const ScopedLock sl (writerLock);
            activeWriter = 0;
        }

        // Now we can delete the writer object. It's done in this order because the deletion could
        // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
        // the audio callback while this happens.
        threadedWriter = 0;
    }

    bool isRecording() const
    {
        return activeWriter != 0;
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

    void audioDeviceIOCallback (const float** inputChannelData, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples)
    {
        const ScopedLock sl (writerLock);

        if (activeWriter != 0)
            activeWriter->write (inputChannelData, numSamples);

        // We need to clear the output buffers, in case they're full of junk..
        for (int i = 0; i < numOutputChannels; ++i)
            if (outputChannelData[i] != 0)
                zeromem (outputChannelData[i], sizeof (float) * (size_t) numSamples);
    }

private:
    TimeSliceThread backgroundThread; // the thread that will write our audio data to disk
    ScopedPointer<AudioFormatWriter::ThreadedWriter> threadedWriter; // the FIFO used to buffer the incoming data
    double sampleRate;

    CriticalSection writerLock;
    AudioFormatWriter::ThreadedWriter* volatile activeWriter;
};

//[/MiscUserDefs]

//==============================================================================
AudioDemoRecordPage::AudioDemoRecordPage (AudioDeviceManager& deviceManager_)
    : deviceManager (deviceManager_)
{
    addAndMakeVisible (liveAudioDisplayComp = new LiveAudioInputDisplayComp());

    addAndMakeVisible (explanationLabel = new Label (String::empty,
                                                     "This page demonstrates how to record a wave file from the live audio input..\n\nPressing record will start recording a file in your \"Documents\" folder."));
    explanationLabel->setFont (Font (15.00f, Font::plain));
    explanationLabel->setJustificationType (Justification::topLeft);
    explanationLabel->setEditable (false, false, false);
    explanationLabel->setColour (TextEditor::textColourId, Colours::black);
    explanationLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (recordButton = new TextButton (String::empty));
    recordButton->setButtonText ("Record");
    recordButton->addListener (this);
    recordButton->setColour (TextButton::buttonColourId, Colour (0xffff5c5c));
    recordButton->setColour (TextButton::textColourOnId, Colours::black);


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
    deviceManager.removeAudioCallback (recorder);
    deviceManager.removeAudioCallback (liveAudioDisplayComp);
    recorder = 0;
    //[/Destructor_pre]

    liveAudioDisplayComp = nullptr;
    explanationLabel = nullptr;
    recordButton = nullptr;


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
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioDemoRecordPage" componentName=""
                 parentClasses="public Component" constructorParams="AudioDeviceManager&amp; deviceManager_"
                 variableInitialisers="deviceManager (deviceManager_)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="0"
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


//[EndFile] You can add extra defines here...
//[/EndFile]
