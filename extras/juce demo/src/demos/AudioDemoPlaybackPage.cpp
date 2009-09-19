/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  18 Sep 2009 7:17:11 pm

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

#include "AudioDemoPlaybackPage.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
class DemoThumbnailComp  : public Component,
                           public ChangeListener
{
public:
    DemoThumbnailComp()
        : thumbnailCache (5),
          thumbnail (512, formatManager, thumbnailCache)
    {
        startTime = endTime = 0;
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener (this);
    }

    ~DemoThumbnailComp()
    {
        thumbnail.removeChangeListener (this);
    }

    void setFile (const File& file)
    {
        thumbnail.setSource (new FileInputSource (file));
        startTime = 0;
        endTime = thumbnail.getTotalLength();
    }

    void setZoomFactor (double amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            double timeDisplayed = jmax (0.001, (thumbnail.getTotalLength() - startTime) * (1.0 - jlimit (0.0, 1.0, amount)));
            endTime = startTime + timeDisplayed;
            repaint();
        }
    }

    void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            double newStart = startTime + (wheelIncrementX + wheelIncrementY) * (endTime - startTime) / 10.0;
            newStart = jlimit (0.0, thumbnail.getTotalLength() - (endTime - startTime), newStart);
            endTime = newStart + (endTime - startTime);
            startTime = newStart;
            repaint();
        }
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);

        g.setColour (Colours::mediumblue);

        if (thumbnail.getTotalLength() > 0)
        {
            int heightPerChannel = (getHeight() - 4) / thumbnail.getNumChannels();

            for (int i = 0; i < thumbnail.getNumChannels(); ++i)
            {
                thumbnail.drawChannel (g, 2, 2 + heightPerChannel * i,
                                       getWidth() - 4, heightPerChannel,
                                       startTime, endTime,
                                       i, 1.0f);
            }
        }
        else
        {
            g.setFont (14.0f);
            g.drawFittedText ("(No audio file selected)", 0, 0, getWidth(), getHeight(), Justification::centred, 2);
        }
    }

    void changeListenerCallback (void*)
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double startTime, endTime;
};

//[/MiscUserDefs]

//==============================================================================
AudioDemoPlaybackPage::AudioDemoPlaybackPage (AudioDeviceManager& deviceManager_)
    : deviceManager (deviceManager_),
      thread ("audio file preview"),
      directoryList (0, thread),
      zoomLabel (0),
      thumbnail (0),
      startStopButton (0),
      fileTreeComp (0),
      explanation (0),
      zoomSlider (0)
{
    addAndMakeVisible (zoomLabel = new Label (String::empty,
                                              T("zoom:")));
    zoomLabel->setFont (Font (15.0000f, Font::plain));
    zoomLabel->setJustificationType (Justification::centredRight);
    zoomLabel->setEditable (false, false, false);
    zoomLabel->setColour (TextEditor::textColourId, Colours::black);
    zoomLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (thumbnail = new DemoThumbnailComp());

    addAndMakeVisible (startStopButton = new TextButton (String::empty));
    startStopButton->setButtonText (T("Play/Stop"));
    startStopButton->addButtonListener (this);
    startStopButton->setColour (TextButton::buttonColourId, Colour (0xff79ed7f));

    addAndMakeVisible (fileTreeComp = new FileTreeComponent (directoryList));

    addAndMakeVisible (explanation = new Label (String::empty,
                                                T("Select an audio file in the treeview above, and this page will display its waveform, and let you play it..")));
    explanation->setFont (Font (14.0000f, Font::plain));
    explanation->setJustificationType (Justification::bottomRight);
    explanation->setEditable (false, false, false);
    explanation->setColour (TextEditor::textColourId, Colours::black);
    explanation->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (zoomSlider = new Slider (String::empty));
    zoomSlider->setRange (0, 1, 0);
    zoomSlider->setSliderStyle (Slider::LinearHorizontal);
    zoomSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
    zoomSlider->addListener (this);
    zoomSlider->setSkewFactor (2);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);

    //[Constructor] You can add your own custom stuff here..
    directoryList.setDirectory (File::getSpecialLocation (File::userHomeDirectory), true, true);
    thread.startThread (3);

    fileTreeComp->setColour (FileTreeComponent::backgroundColourId, Colours::white);
    fileTreeComp->addListener (this);

    deviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (&transportSource);
    currentAudioFileSource = 0;
    //[/Constructor]
}

AudioDemoPlaybackPage::~AudioDemoPlaybackPage()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    transportSource.setSource (0);
    audioSourcePlayer.setSource (0);

    deviceManager.removeAudioCallback (&audioSourcePlayer);
    fileTreeComp->removeListener (this);

    deleteAndZero (currentAudioFileSource);
    //[/Destructor_pre]

    deleteAndZero (zoomLabel);
    deleteAndZero (thumbnail);
    deleteAndZero (startStopButton);
    deleteAndZero (fileTreeComp);
    deleteAndZero (explanation);
    deleteAndZero (zoomSlider);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AudioDemoPlaybackPage::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::lightgrey);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioDemoPlaybackPage::resized()
{
    zoomLabel->setBounds (16, getHeight() - 90, 55, 24);
    thumbnail->setBounds (16, getHeight() - 221, getWidth() - 32, 123);
    startStopButton->setBounds (16, getHeight() - 46, 150, 32);
    fileTreeComp->setBounds (16, 8, getWidth() - 32, getHeight() - 245);
    explanation->setBounds (256, getHeight() - 82, getWidth() - 275, 64);
    zoomSlider->setBounds (72, getHeight() - 90, 200, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void AudioDemoPlaybackPage::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == startStopButton)
    {
        //[UserButtonCode_startStopButton] -- add your button handler code here..
        if (transportSource.isPlaying())
        {
            transportSource.stop();
        }
        else
        {
            transportSource.setPosition (0);
            transportSource.start();
        }
        //[/UserButtonCode_startStopButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void AudioDemoPlaybackPage::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == zoomSlider)
    {
        //[UserSliderCode_zoomSlider] -- add your slider handling code here..
        thumbnail->setZoomFactor (zoomSlider->getValue());
        //[/UserSliderCode_zoomSlider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void AudioDemoPlaybackPage::loadFileIntoTransport (const File& audioFile)
{
    // unload the previous file source and delete it..
    transportSource.stop();
    transportSource.setSource (0);
    deleteAndZero (currentAudioFileSource);

    // get a format manager and set it up with the basic types (wav and aiff).
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    AudioFormatReader* reader = formatManager.createReaderFor (audioFile);

    if (reader != 0)
    {
        currentAudioFileSource = new AudioFormatReaderSource (reader, true);

        // ..and plug it into our transport source
        transportSource.setSource (currentAudioFileSource,
                                   32768, // tells it to buffer this many samples ahead
                                   reader->sampleRate);
    }
}

void AudioDemoPlaybackPage::selectionChanged()
{
    loadFileIntoTransport (fileTreeComp->getSelectedFile());

    zoomSlider->setValue (0, false, false);
    thumbnail->setFile (fileTreeComp->getSelectedFile());
}

void AudioDemoPlaybackPage::fileClicked (const File& file, const MouseEvent& e)
{
}

void AudioDemoPlaybackPage::fileDoubleClicked (const File& file)
{
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioDemoPlaybackPage" componentName=""
                 parentClasses="public Component, public FileBrowserListener"
                 constructorParams="AudioDeviceManager&amp; deviceManager_" variableInitialisers="deviceManager (deviceManager_),&#10;thread (&quot;audio file preview&quot;),&#10;directoryList (0, thread)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffd3d3d3"/>
  <LABEL name="" id="d4f78f975d81c8d3" memberName="zoomLabel" virtualName=""
         explicitFocusOrder="0" pos="16 90R 55 24" edTextCol="ff000000"
         edBkgCol="0" labelText="zoom:" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="34"/>
  <GENERICCOMPONENT name="" id="beef657b0e007936" memberName="thumbnail" virtualName=""
                    explicitFocusOrder="0" pos="16 221R 32M 123" class="DemoThumbnailComp"
                    params=""/>
  <TEXTBUTTON name="" id="abe446e2f3f09420" memberName="startStopButton" virtualName=""
              explicitFocusOrder="0" pos="16 46R 150 32" bgColOff="ff79ed7f"
              buttonText="Play/Stop" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <GENERICCOMPONENT name="" id="1de1dc6a18a9032b" memberName="fileTreeComp" virtualName=""
                    explicitFocusOrder="0" pos="16 8 32M 245M" class="FileTreeComponent"
                    params="directoryList"/>
  <LABEL name="" id="7db7d0a64ef21311" memberName="explanation" virtualName=""
         explicitFocusOrder="0" pos="256 82R 275M 64" edTextCol="ff000000"
         edBkgCol="0" labelText="Select an audio file in the treeview above, and this page will display its waveform, and let you play it.."
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="14" bold="0" italic="0" justification="18"/>
  <SLIDER name="" id="38bbc108f4c96092" memberName="zoomSlider" virtualName=""
          explicitFocusOrder="0" pos="72 90R 200 24" min="0" max="1" int="0"
          style="LinearHorizontal" textBoxPos="NoTextBox" textBoxEditable="1"
          textBoxWidth="80" textBoxHeight="20" skewFactor="2"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
