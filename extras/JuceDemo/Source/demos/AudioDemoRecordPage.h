/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  1 May 2011 12:08:25pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_AUDIODEMORECORDPAGE_AUDIODEMORECORDPAGE_E12E1947__
#define __JUCER_HEADER_AUDIODEMORECORDPAGE_AUDIODEMORECORDPAGE_E12E1947__

//[Headers]     -- You can add your own extra header files here --
#include "../jucedemo_headers.h"
#include "AudioDemoTabComponent.h"
class AudioRecorder;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class AudioDemoRecordPage  : public Component,
                             public ButtonListener
{
public:
    //==============================================================================
    AudioDemoRecordPage (AudioDeviceManager& deviceManager_);
    ~AudioDemoRecordPage();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    void visibilityChanged();



    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    AudioDeviceManager& deviceManager;
    ScopedPointer<AudioRecorder> recorder;
    //[/UserVariables]

    //==============================================================================
    LiveAudioInputDisplayComp* liveAudioDisplayComp;
    Label* explanationLabel;
    TextButton* recordButton;


    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    AudioDemoRecordPage (const AudioDemoRecordPage&);
    const AudioDemoRecordPage& operator= (const AudioDemoRecordPage&);
};


#endif   // __JUCER_HEADER_AUDIODEMORECORDPAGE_AUDIODEMORECORDPAGE_E12E1947__
