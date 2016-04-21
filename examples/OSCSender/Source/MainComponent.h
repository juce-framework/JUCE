/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public Component, private Slider::Listener
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize (200, 200);

        rotaryKnob.setRange (0.0, 1.0);
        rotaryKnob.setSliderStyle (Slider::RotaryVerticalDrag);
        rotaryKnob.setTextBoxStyle (Slider::TextBoxBelow, true, 150, 25);
        rotaryKnob.setBounds (10, 10, 180, 180);
        addAndMakeVisible (rotaryKnob);
        rotaryKnob.addListener (this);

        // specify here where to send OSC messages to: host URL and UDP port number
        if (! sender.connect ("127.0.0.1", 9001))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");
    }

private:
    //==============================================================================
    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &rotaryKnob)
        {
            // create and send an OSC message with an address and a float value:
            if (! sender.send ("/juce/rotaryknob", (float) rotaryKnob.getValue()))
                showConnectionErrorMessage ("Error: could not send OSC message.");
        }
    }

    void showConnectionErrorMessage (const String& messageText)
    {
        AlertWindow::showMessageBoxAsync (
            AlertWindow::WarningIcon,
            "Connection error",
            messageText,
            "OK");
    }

    //==============================================================================
    Slider rotaryKnob;
    OSCSender sender;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
