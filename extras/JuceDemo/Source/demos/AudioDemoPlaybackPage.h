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

#ifndef __JUCER_HEADER_AUDIODEMOPLAYBACKPAGE_AUDIODEMOPLAYBACKPAGE_1B87E5D0__
#define __JUCER_HEADER_AUDIODEMOPLAYBACKPAGE_AUDIODEMOPLAYBACKPAGE_1B87E5D0__

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
                               public SliderListener,
                               public ButtonListener
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
    void browserRootChanged (const File&) {}
    void showFile (const File& file);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void sliderValueChanged (Slider* sliderThatWasMoved);
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    AudioDeviceManager& deviceManager;
    AudioFormatManager formatManager;
    TimeSliceThread thread;
    DirectoryContentsList directoryList;

    AudioSourcePlayer audioSourcePlayer;
    AudioTransportSource transportSource;
    ScopedPointer<AudioFormatReaderSource> currentAudioFileSource;

    void loadFileIntoTransport (const File& audioFile);
    //[/UserVariables]

    //==============================================================================
    Label* zoomLabel;
    Label* explanation;
    Slider* zoomSlider;
    DemoThumbnailComp* thumbnail;
    TextButton* startStopButton;
    FileTreeComponent* fileTreeComp;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioDemoPlaybackPage)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_AUDIODEMOPLAYBACKPAGE_AUDIODEMOPLAYBACKPAGE_1B87E5D0__
