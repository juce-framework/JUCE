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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "AudioDemoSetupPage.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
AudioDemoSetupPage::AudioDemoSetupPage (AudioDeviceManager& deviceManager_)
    : deviceManager (deviceManager_),
      deviceSelector (0)
{
    addAndMakeVisible (deviceSelector = new AudioDeviceSelectorComponent (deviceManager, 0, 2, 0, 2, true, true, true, false));


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);

    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

AudioDemoSetupPage::~AudioDemoSetupPage()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (deviceSelector);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AudioDemoSetupPage::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::lightgrey);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioDemoSetupPage::resized()
{
    deviceSelector->setBounds (8, 8, getWidth() - 16, getHeight() - 16);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioDemoSetupPage" componentName=""
                 parentClasses="public Component" constructorParams="AudioDeviceManager&amp; deviceManager_"
                 variableInitialisers="deviceManager (deviceManager_)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330000013" fixedSize="0"
                 initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffd3d3d3"/>
  <GENERICCOMPONENT name="" id="a04c56de9f3fc537" memberName="deviceSelector" virtualName=""
                    explicitFocusOrder="0" pos="8 8 16M 16M" class="AudioDeviceSelectorComponent"
                    params="deviceManager, 0, 2, 0, 2, true, true, true, false"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
