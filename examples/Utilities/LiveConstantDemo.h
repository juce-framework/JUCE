/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             LiveConstantDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Demonstrates the live constant macro.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        LiveConstantDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct LiveConstantDemoComponent final : public Component
{
    LiveConstantDemoComponent() {}

    void paint (Graphics& g) override
    {
        g.fillAll (JUCE_LIVE_CONSTANT (Colour (0xffe5e7a7)));

        g.setColour (JUCE_LIVE_CONSTANT (Colours::red.withAlpha (0.2f)));
        auto blockWidth  = JUCE_LIVE_CONSTANT (0x120);
        auto blockHeight = JUCE_LIVE_CONSTANT (200);
        g.fillRect ((getWidth() - blockWidth) / 2, (getHeight() - blockHeight) / 2, blockWidth, blockHeight);

        auto fontColour = JUCE_LIVE_CONSTANT (Colour (0xff000a55));
        auto fontSize   = JUCE_LIVE_CONSTANT (30.0f);

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
class LiveConstantDemo final : public Component
{
public:
    LiveConstantDemo()
    {
        descriptionLabel.setMinimumHorizontalScale (1.0f);
        descriptionLabel.setText ("This demonstrates the JUCE_LIVE_CONSTANT macro, which allows you to quickly "
                                  "adjust primitive values at runtime by just wrapping them in a macro.\n\n"
                                  "Editing JUCE_LIVE_CONSTANT values is only enabled in debug builds.\n\n"
                                  "To understand what's going on in this demo, you should have a look at the "
                                  "LiveConstantDemoComponent class, where you can see the code that's invoking the demo below.",
                                  dontSendNotification);

        addAndMakeVisible (descriptionLabel);
        addAndMakeVisible (startButton);
        addChildComponent (demoComp);
        startButton.onClick = [this] { start(); };

        setSize (500, 500);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (10);

        descriptionLabel.setBounds (r.removeFromTop (200));
        startButton     .setBounds (r.removeFromTop (22).removeFromLeft (250));
        demoComp        .setBounds (r.withTrimmedTop (10));
    }

    void start()
    {
        startButton.setVisible (false);
        demoComp   .setVisible (true);

        descriptionLabel.setText ("Tweak some of the colours and values in the pop-up window to see what "
                                  "the effect of your changes would be on the component below...",
                                  dontSendNotification);
    }

private:
    Label descriptionLabel;
    TextButton startButton  { "Begin Demo" };

    LiveConstantDemoComponent demoComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveConstantDemo)
};
