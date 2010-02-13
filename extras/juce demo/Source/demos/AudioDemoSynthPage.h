/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  18 Sep 2009 9:46:49 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_AUDIODEMOSYNTHPAGE_AUDIODEMOSYNTHPAGE_76557DC7__
#define __JUCER_HEADER_AUDIODEMOSYNTHPAGE_AUDIODEMOSYNTHPAGE_76557DC7__

//[Headers]     -- You can add your own extra header files here --
#include "../jucedemo_headers.h"
#include "AudioDemoTabComponent.h"
class SynthAudioSource;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class AudioDemoSynthPage  : public Component,
                            public ButtonListener
{
public:
    //==============================================================================
    AudioDemoSynthPage (AudioDeviceManager& deviceManager_);
    ~AudioDemoSynthPage();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    AudioDeviceManager& deviceManager;
    MidiKeyboardState keyboardState;
    AudioSourcePlayer audioSourcePlayer;
    SynthAudioSource* synthAudioSource;
    //[/UserVariables]

    //==============================================================================
    MidiKeyboardComponent* keyboardComponent;
    ToggleButton* sineButton;
    ToggleButton* sampledButton;
    LiveAudioInputDisplayComp* liveAudioDisplayComp;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    AudioDemoSynthPage (const AudioDemoSynthPage&);
    const AudioDemoSynthPage& operator= (const AudioDemoSynthPage&);
};


#endif   // __JUCER_HEADER_AUDIODEMOSYNTHPAGE_AUDIODEMOSYNTHPAGE_76557DC7__
