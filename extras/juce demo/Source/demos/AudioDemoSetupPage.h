/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  18 Sep 2009 11:26:45 am

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_AUDIODEMOSETUPPAGE_AUDIODEMOSETUPPAGE_244CE6A7__
#define __JUCER_HEADER_AUDIODEMOSETUPPAGE_AUDIODEMOSETUPPAGE_244CE6A7__

//[Headers]     -- You can add your own extra header files here --
#include "../jucedemo_headers.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class AudioDemoSetupPage  : public Component
{
public:
    //==============================================================================
    AudioDemoSetupPage (AudioDeviceManager& deviceManager_);
    ~AudioDemoSetupPage();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    AudioDeviceManager& deviceManager;
    //[/UserVariables]

    //==============================================================================
    AudioDeviceSelectorComponent* deviceSelector;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    AudioDemoSetupPage (const AudioDemoSetupPage&);
    const AudioDemoSetupPage& operator= (const AudioDemoSetupPage&);
};


#endif   // __JUCER_HEADER_AUDIODEMOSETUPPAGE_AUDIODEMOSETUPPAGE_244CE6A7__
