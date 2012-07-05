/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  1 May 2011 12:08:14pm

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
                           public ChangeListener,
                           public FileDragAndDropTarget,
                           private Timer
{
public:
    DemoThumbnailComp (AudioFormatManager& formatManager,
                       AudioTransportSource& transportSource_,
                       Slider& zoomSlider_)
        : transportSource (transportSource_),
          zoomSlider (zoomSlider_),
          thumbnailCache (5),
          thumbnail (512, formatManager, thumbnailCache)
    {
        startTime = endTime = 0;
        thumbnail.addChangeListener (this);

        currentPositionMarker.setFill (Colours::purple.withAlpha (0.7f));
        addAndMakeVisible (&currentPositionMarker);
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
        startTimer (1000 / 40);
    }

    void setZoomFactor (double amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            const double newScale = jmax (0.001, thumbnail.getTotalLength() * (1.0 - jlimit (0.0, 0.99, amount)));
            const double timeAtCentre = xToTime (getWidth() / 2.0f);
            startTime = timeAtCentre - newScale * 0.5;
            endTime = timeAtCentre + newScale * 0.5;
            repaint();
        }
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            double newStart = startTime - wheel.deltaX * (endTime - startTime) / 10.0;
            newStart = jlimit (0.0, jmax (0.0, thumbnail.getTotalLength() - (endTime - startTime)), newStart);
            endTime = newStart + (endTime - startTime);
            startTime = newStart;

            if (wheel.deltaY != 0)
                zoomSlider.setValue (zoomSlider.getValue() - wheel.deltaY);

            repaint();
        }
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);
        g.setColour (Colours::lightblue);

        if (thumbnail.getTotalLength() > 0)
        {
            thumbnail.drawChannels (g, getLocalBounds().reduced (2, 2),
                                    startTime, endTime, 1.0f);
        }
        else
        {
            g.setFont (14.0f);
            g.drawFittedText ("(No audio file selected)", 0, 0, getWidth(), getHeight(),
                              Justification::centred, 2);
        }
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    bool isInterestedInFileDrag (const StringArray& /*files*/)
    {
        return true;
    }

    void filesDropped (const StringArray& files, int /*x*/, int /*y*/)
    {
        AudioDemoPlaybackPage* demoPage = findParentComponentOfClass<AudioDemoPlaybackPage>();

        if (demoPage != nullptr)
            demoPage->showFile (File (files[0]));
    }

    void mouseDown (const MouseEvent& e)
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e)
    {
        transportSource.setPosition (jmax (0.0, xToTime ((float) e.x)));
    }

    void mouseUp (const MouseEvent&)
    {
        transportSource.start();
    }

    void timerCallback()
    {
        currentPositionMarker.setVisible (transportSource.isPlaying() || isMouseButtonDown());

        double currentPlayPosition = transportSource.getCurrentPosition();

        currentPositionMarker.setRectangle (Rectangle<float> (timeToX (currentPlayPosition) - 0.75f, 0,
                                                              1.5f, (float) getHeight()));
    }

private:
    AudioTransportSource& transportSource;
    Slider& zoomSlider;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double startTime, endTime;

    DrawableRectangle currentPositionMarker;

    float timeToX (const double time) const
    {
        return getWidth() * (float) ((time - startTime) / (endTime - startTime));
    }

    double xToTime (const float x) const
    {
        return (x / getWidth()) * (endTime - startTime) + startTime;
    }
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
                                              L"zoom:"));
    zoomLabel->setFont (Font (15.0000f, Font::plain));
    zoomLabel->setJustificationType (Justification::centredRight);
    zoomLabel->setEditable (false, false, false);
    zoomLabel->setColour (TextEditor::textColourId, Colours::black);
    zoomLabel->setColour (TextEditor::backgroundColourId, Colour (0x0));

    addAndMakeVisible (explanation = new Label (String::empty,
                                                L"Select an audio file in the treeview above, and this page will display its waveform, and let you play it.."));
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

    addAndMakeVisible (thumbnail = new DemoThumbnailComp (formatManager, transportSource, *zoomSlider));

    addAndMakeVisible (startStopButton = new TextButton (String::empty));
    startStopButton->setButtonText (L"Play/Stop");
    startStopButton->addListener (this);
    startStopButton->setColour (TextButton::buttonColourId, Colour (0xff79ed7f));

    addAndMakeVisible (fileTreeComp = new FileTreeComponent (directoryList));

    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    formatManager.registerBasicFormats();

    directoryList.setDirectory (File::getSpecialLocation (File::userHomeDirectory), true, true);
    thread.startThread (3);

    fileTreeComp->setColour (FileTreeComponent::backgroundColourId, Colours::white);
    fileTreeComp->addListener (this);

    deviceManager.addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (&transportSource);
    //[/Constructor]
}

AudioDemoPlaybackPage::~AudioDemoPlaybackPage()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    transportSource.setSource (nullptr);
    audioSourcePlayer.setSource (nullptr);

    deviceManager.removeAudioCallback (&audioSourcePlayer);
    fileTreeComp->removeListener (this);
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

void AudioDemoPlaybackPage::showFile (const File& file)
{
    loadFileIntoTransport (file);

    zoomSlider->setValue (0, false, false);
    thumbnail->setFile (file);
}

void AudioDemoPlaybackPage::loadFileIntoTransport (const File& audioFile)
{
    // unload the previous file source and delete it..
    transportSource.stop();
    transportSource.setSource (nullptr);
    currentAudioFileSource = nullptr;

    AudioFormatReader* reader = formatManager.createReaderFor (audioFile);

    if (reader != nullptr)
    {
        currentAudioFileSource = new AudioFormatReaderSource (reader, true);

        // ..and plug it into our transport source
        transportSource.setSource (currentAudioFileSource,
                                   32768, // tells it to buffer this many samples ahead
                                   &thread, // this is the background thread to use for reading-ahead
                                   reader->sampleRate);
    }
}

void AudioDemoPlaybackPage::selectionChanged()
{
    showFile (fileTreeComp->getSelectedFile());
}

void AudioDemoPlaybackPage::fileClicked (const File&, const MouseEvent&)
{
}

void AudioDemoPlaybackPage::fileDoubleClicked (const File&)
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
  <LABEL name="" id="7db7d0a64ef21311" memberName="explanation" virtualName=""
         explicitFocusOrder="0" pos="256 82R 275M 64" edTextCol="ff000000"
         edBkgCol="0" labelText="Select an audio file in the treeview above, and this page will display its waveform, and let you play it.."
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="14" bold="0" italic="0" justification="18"/>
  <SLIDER name="" id="38bbc108f4c96092" memberName="zoomSlider" virtualName=""
          explicitFocusOrder="0" pos="72 90R 200 24" min="0" max="1" int="0"
          style="LinearHorizontal" textBoxPos="NoTextBox" textBoxEditable="1"
          textBoxWidth="80" textBoxHeight="20" skewFactor="2"/>
  <GENERICCOMPONENT name="" id="beef657b0e007936" memberName="thumbnail" virtualName=""
                    explicitFocusOrder="0" pos="16 221R 32M 123" class="DemoThumbnailComp"
                    params="formatManager, transportSource, *zoomSlider"/>
  <TEXTBUTTON name="" id="abe446e2f3f09420" memberName="startStopButton" virtualName=""
              explicitFocusOrder="0" pos="16 46R 150 32" bgColOff="ff79ed7f"
              buttonText="Play/Stop" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <GENERICCOMPONENT name="" id="1de1dc6a18a9032b" memberName="fileTreeComp" virtualName=""
                    explicitFocusOrder="0" pos="16 8 32M 245M" class="FileTreeComponent"
                    params="directoryList"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
