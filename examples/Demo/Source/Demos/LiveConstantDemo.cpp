/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-12 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"


//==============================================================================
struct LiveConstantDemoComponent  : public Component
{
    LiveConstantDemoComponent() {}

    void paint (Graphics& g)
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
                                 private ButtonListener
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
        fillTiledBackground (g);
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveConstantEditorDemo);
};


#if ! (JUCE_IOS || JUCE_ANDROID)
 // This static object will register this demo type in a global list of demos..
 static JuceDemoType<LiveConstantEditorDemo> demo ("10 Components: Live Constants");
#endif
