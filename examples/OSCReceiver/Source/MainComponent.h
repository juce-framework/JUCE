/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public Component,
                               private OSCReceiver,
                               private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>
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
        rotaryKnob.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (rotaryKnob);

        // specify here on which UDP port number to receive incoming OSC messages
        if (! connect (9001))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");

        // tell the component to listen for OSC messages matching this address:
        addListener (this, "/juce/rotaryknob");
    }

private:
    //==============================================================================
    void oscMessageReceived (const OSCMessage& message) override
    {
        if (message.size() == 1 && message[0].isFloat32())
            rotaryKnob.setValue (jlimit (0.0f, 10.0f, message[0].getFloat32()));
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
