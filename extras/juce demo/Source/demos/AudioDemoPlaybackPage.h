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

#ifndef __JUCER_HEADER_AUDIODEMOPLAYBACKPAGE_AUDIODEMOPLAYBACKPAGE_8AB55057__
#define __JUCER_HEADER_AUDIODEMOPLAYBACKPAGE_AUDIODEMOPLAYBACKPAGE_8AB55057__

//[Headers]     -- You can add your own extra header files here --
#include "../jucedemo_headers.h"
class DemoThumbnailComp;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class AudioDemoPlaybackPage  : public Component,
                               public FileBrowserListener,
                               public ButtonListener,
                               public SliderListener
{
public:
    //==============================================================================
    AudioDemoPlaybackPage (AudioDeviceManager& deviceManager_);
    ~AudioDemoPlaybackPage();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

    // These methods are our FileBrowserListener implementation:
    void selectionChanged();
    void fileClicked (const File& file, const MouseEvent& e);
    void fileDoubleClicked (const File& file);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    void sliderValueChanged (Slider* sliderThatWasMoved);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    AudioDeviceManager& deviceManager;
    TimeSliceThread thread;
    DirectoryContentsList directoryList;

    AudioSourcePlayer audioSourcePlayer;
    AudioTransportSource transportSource;
    AudioFormatReaderSource* currentAudioFileSource;

    void loadFileIntoTransport (const File& audioFile);
    //[/UserVariables]

    //==============================================================================
    Label* zoomLabel;
    DemoThumbnailComp* thumbnail;
    TextButton* startStopButton;
    FileTreeComponent* fileTreeComp;
    Label* explanation;
    Slider* zoomSlider;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    AudioDemoPlaybackPage (const AudioDemoPlaybackPage&);
    const AudioDemoPlaybackPage& operator= (const AudioDemoPlaybackPage&);
};


#endif   // __JUCER_HEADER_AUDIODEMOPLAYBACKPAGE_AUDIODEMOPLAYBACKPAGE_8AB55057__
