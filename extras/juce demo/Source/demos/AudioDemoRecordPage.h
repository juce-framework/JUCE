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

#ifndef __JUCER_HEADER_AUDIODEMORECORDPAGE_AUDIODEMORECORDPAGE_4FF281BF__
#define __JUCER_HEADER_AUDIODEMORECORDPAGE_AUDIODEMORECORDPAGE_4FF281BF__

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
    AudioRecorder* recorder;
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


#endif   // __JUCER_HEADER_AUDIODEMORECORDPAGE_AUDIODEMORECORDPAGE_4FF281BF__
