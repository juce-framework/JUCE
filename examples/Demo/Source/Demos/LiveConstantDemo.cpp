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

#include "../JuceDemoHeader.h"


//==============================================================================
struct LiveConstantDemoComponent  : public Component
{
    LiveConstantDemoComponent() {}

    void paint (Graphics& g) override
    {
        g.fillAll (JUCE_LIVE_CONSTANT (Colour (0xffe5e7a7)));

        g.setColour (JUCE_LIVE_CONSTANT (Colours::red.withAlpha (0.2f)));
        int blockWidth = JUCE_LIVE_CONSTANT (0x120);
        int blockHeight = JUCE_LIVE_CONSTANT (200);
        g.fillRect ((getWidth() - blockWidth) / 2, (getHeight() - blockHeight) / 2, blockWidth, blockHeight);

        Colour fontColour = JUCE_LIVE_CONSTANT (Colour (0xff000a55));
        float fontSize = JUCE_LIVE_CONSTANT (30.0f);

        g.setColour (fontColour);
        g.setFont (fontSize);

        g.drawFittedText (getDemoText(), getLocalBounds(), Justification::centred, 2);
    }

    static String getDemoText()
    {
        return JUCE_LIVE_CONSTANT ("Hello world!");
    }
};

//==============================================================================
class LiveConstantEditorDemo   : public Component,
                                 private Button::Listener
{
public:
    LiveConstantEditorDemo()
        : startButton ("Begin Demo")
    {
        descriptionLabel.setMinimumHorizontalScale (1.0f);
        descriptionLabel.setText ("This demonstrates the JUCE_LIVE_CONSTANT macro, which allows you to quickly "
                                  "adjust primitive values at runtime by just wrapping them in a macro.\n\n"
                                  "To understand what's going on in this demo, you should have a look at the "
                                  "LiveConstantDemoComponent class in LiveConstantDemo.cpp, where you can see "
                                  "the code that's invoking the demo below...",
                                  dontSendNotification);

        addAndMakeVisible (descriptionLabel);
        addAndMakeVisible (startButton);
        addChildComponent (demoComp);
        startButton.addListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (10));

        demoComp.setBounds (r);

        descriptionLabel.setBounds (r.removeFromTop (200));
        startButton.setBounds (r.removeFromTop (22).removeFromLeft (250));

        demoComp.setBounds (r.withTrimmedTop (10));
    }

private:
    Label descriptionLabel;
    TextButton startButton;
    LiveConstantDemoComponent demoComp;

    void buttonClicked (Button*) override
    {
        startButton.setVisible (false);
        demoComp.setVisible (true);

        descriptionLabel.setText ("Tweak some of the colours and values in the pop-up window to see what "
                                  "the effect of your changes would be on the component below...",
                                  dontSendNotification);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveConstantEditorDemo)
};


#if ! (JUCE_IOS || JUCE_ANDROID)
 // This static object will register this demo type in a global list of demos..
 static JuceDemoType<LiveConstantEditorDemo> demo ("10 Components: Live Constants");
#endif
