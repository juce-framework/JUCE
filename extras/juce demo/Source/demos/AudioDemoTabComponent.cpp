/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  19 Sep 2009 11:10:57 am

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

#include "AudioDemoTabComponent.h"
#include "AudioDemoSetupPage.h"
#include "AudioDemoPlaybackPage.h"
#include "AudioDemoSynthPage.h"
#include "AudioDemoLatencyPage.h"
#include "AudioDemoRecordPage.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
LiveAudioInputDisplayComp::LiveAudioInputDisplayComp()
{
    nextSample = subSample = 0;
    accumulator = 0;
    zeromem (samples, sizeof (samples));
    setOpaque (true);

    startTimer (1000 / 50); // use a timer to keep repainting this component
}

LiveAudioInputDisplayComp::~LiveAudioInputDisplayComp()
{
}

void LiveAudioInputDisplayComp::paint (Graphics& g)
{
    g.fillAll (Colours::black);

    g.setColour (Colours::green);
    const float midY = getHeight() * 0.5f;
    int sampleNum = (nextSample + numElementsInArray (samples) - 1);

    for (int x = jmin (getWidth(), numElementsInArray (samples)); --x >= 0;)
    {
        const float sampleSize = midY * samples [sampleNum-- % numElementsInArray (samples)];
        g.drawVerticalLine (x, midY - sampleSize, midY + sampleSize);
    }
}

void LiveAudioInputDisplayComp::timerCallback()
{
    repaint();
}

void LiveAudioInputDisplayComp::audioDeviceAboutToStart (AudioIODevice* device)
{
    zeromem (samples, sizeof (samples));
}

void LiveAudioInputDisplayComp::audioDeviceStopped()
{
    zeromem (samples, sizeof (samples));
}

void LiveAudioInputDisplayComp::audioDeviceIOCallback (const float** inputChannelData, int numInputChannels,
                                                       float** outputChannelData, int numOutputChannels, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        for (int chan = 0; chan < numInputChannels; ++chan)
        {
            if (inputChannelData[chan] != 0)
                accumulator += fabsf (inputChannelData[chan][i]);
        }

        const int numSubSamples = 100; // how many input samples go onto one pixel.
        const float boost = 10.0f;     // how much to boost the levels to make it more visible.

        if (subSample == 0)
        {
            samples[nextSample] = accumulator * boost / numSubSamples;
            nextSample = (nextSample + 1) % numElementsInArray (samples);
            subSample = numSubSamples;
            accumulator = 0;
        }
        else
        {
            --subSample;
        }
    }

    // We need to clear the output buffers, in case they're full of junk..
    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != 0)
            zeromem (outputChannelData[i], sizeof (float) * numSamples);
}

//[/MiscUserDefs]

//==============================================================================
AudioDemoTabComponent::AudioDemoTabComponent ()
    : tabbedComponent (0)
{
    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    tabbedComponent->setTabBarDepth (30);
    tabbedComponent->addTab (T("Audio Device Setup"), Colours::lightgrey, new AudioDemoSetupPage (deviceManager), true);
    tabbedComponent->addTab (T("File Playback"), Colours::lightgrey, new AudioDemoPlaybackPage (deviceManager), true);
    tabbedComponent->addTab (T("Synth Playback"), Colours::lightgrey, new AudioDemoSynthPage (deviceManager), true);
    tabbedComponent->addTab (T("Latency Test"), Colours::lightgrey, new AudioDemoLatencyPage (deviceManager), true);
    tabbedComponent->addTab (T("Recording"), Colours::lightgrey, new AudioDemoRecordPage (deviceManager), true);
    tabbedComponent->setCurrentTabIndex (0);


    //[UserPreSize]
    deviceManager.initialise (2, 2, 0, true, String::empty, 0);
    //[/UserPreSize]

    setSize (600, 400);

    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

AudioDemoTabComponent::~AudioDemoTabComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (tabbedComponent);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AudioDemoTabComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioDemoTabComponent::resized()
{
    tabbedComponent->setBounds (0, 0, getWidth() - 0, getHeight() - 0);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//==============================================================================
Component* createAudioDemo()
{
    return new AudioDemoTabComponent();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioDemoTabComponent" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TABBEDCOMPONENT name="new tabbed component" id="83c980d7793cdced" memberName="tabbedComponent"
                   virtualName="" explicitFocusOrder="0" pos="0 0 0M 0M" orientation="top"
                   tabBarDepth="30" initialTab="0">
    <TAB name="Audio Device Setup" colour="ffd3d3d3" useJucerComp="1"
         contentClassName="" constructorParams="deviceManager" jucerComponentFile="AudioDemoSetupPage.cpp"/>
    <TAB name="File Playback" colour="ffd3d3d3" useJucerComp="1" contentClassName=""
         constructorParams="deviceManager" jucerComponentFile="AudioDemoPlaybackPage.cpp"/>
    <TAB name="Synth Playback" colour="ffd3d3d3" useJucerComp="1" contentClassName=""
         constructorParams="deviceManager" jucerComponentFile="AudioDemoSynthPage.cpp"/>
    <TAB name="Latency Test" colour="ffd3d3d3" useJucerComp="1" contentClassName=""
         constructorParams="deviceManager" jucerComponentFile="AudioDemoLatencyPage.cpp"/>
    <TAB name="Recording" colour="ffd3d3d3" useJucerComp="1" contentClassName=""
         constructorParams="deviceManager" jucerComponentFile="AudioDemoRecordPage.cpp"/>
  </TABBEDCOMPONENT>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
