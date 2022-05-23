/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             TimersAndEventsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Application using timers and events.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        TimersAndEventsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** Simple message that holds a Colour. */
struct ColourMessage  : public Message
{
    ColourMessage (Colour col)  : colour (col) {}

    /** Returns the colour of a ColourMessage of white if the message is not a ColourMessage. */
    static Colour getColour (const Message& message)
    {
        if (auto* cm = dynamic_cast<const ColourMessage*> (&message))
            return cm->colour;

        return Colours::white;
    }

    Colour colour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourMessage)
};

//==============================================================================
/** Simple component that can be triggered to flash.
    The flash will then fade using a Timer to repaint itself and will send a change
    message once it is finished.
 */
class FlashingComponent   : public Component,
                            public MessageListener,
                            public ChangeBroadcaster,
                            private Timer
{
public:
    FlashingComponent() {}

    void startFlashing()
    {
        flashAlpha = 1.0f;
        startTimerHz (25);
    }

    /** Stops this component flashing without sending a change message. */
    void stopFlashing()
    {
        flashAlpha = 0.0f;
        stopTimer();
        repaint();
    }

    /** Sets the colour of the component. */
    void setFlashColour (const Colour newColour)
    {
        colour = newColour;
        repaint();
    }

    /** Draws our component. */
    void paint (Graphics& g) override
    {
        g.setColour (colour.overlaidWith (Colours::white.withAlpha (flashAlpha)));
        g.fillEllipse (getLocalBounds().toFloat());
    }

    /** Custom mouse handler to trigger a flash. */
    void mouseDown (const MouseEvent&) override
    {
        startFlashing();
    }

    /** Message listener callback used to change our colour */
    void handleMessage (const Message& message) override
    {
        setFlashColour (ColourMessage::getColour (message));
    }

private:
    float flashAlpha = 0.0f;
    Colour colour { Colours::red };

    void timerCallback() override
    {
        // Reduce the alpha level of the flash slightly so it fades out
        flashAlpha -= 0.075f;

        if (flashAlpha < 0.05f)
        {
            stopFlashing();
            sendChangeMessage();
            // Once we've finished flashing send a change message to trigger the next component to flash
        }

        repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlashingComponent)
};

//==============================================================================
class TimersAndEventsDemo   : public Component,
                              private ChangeListener
{
public:
    TimersAndEventsDemo()
    {
        setOpaque (true);

        // Create and add our FlashingComponents with some random colours and sizes
        for (int i = 0; i < numFlashingComponents; ++i)
        {
            auto* newFlasher = new FlashingComponent();
            flashingComponents.add (newFlasher);

            newFlasher->setFlashColour (getRandomBrightColour());
            newFlasher->addChangeListener (this);

            auto diameter = 25 + random.nextInt (75);
            newFlasher->setSize (diameter, diameter);

            addAndMakeVisible (newFlasher);
        }

        addAndMakeVisible (stopButton);
        stopButton.onClick = [this] { stopButtonClicked(); };

        addAndMakeVisible (randomColourButton);
        randomColourButton.onClick = [this] { randomColourButtonClicked(); };

        // lay out our components in a pseudo random grid
        Rectangle<int> area (0, 100, 150, 150);

        for (auto* comp : flashingComponents)
        {
            auto buttonArea = area.withSize (comp->getWidth(), comp->getHeight());
            buttonArea.translate (random.nextInt (area.getWidth()  - comp->getWidth()),
                                  random.nextInt (area.getHeight() - comp->getHeight()));
            comp->setBounds (buttonArea);

            area.translate (area.getWidth(), 0);

            // if we go off the right start a new row
            if (area.getRight() > (800 - area.getWidth()))
            {
                area.translate (0, area.getWidth());
                area.setX (0);
            }
        }

        setSize (600, 600);
    }

    ~TimersAndEventsDemo() override
    {
        for (auto* fc : flashingComponents)
            fc->removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colours::darkgrey));
    }

    void paintOverChildren (Graphics& g) override
    {
        auto explanationArea = getLocalBounds().removeFromTop (100);

        AttributedString s;
        s.append ("Click on a circle to make it flash. When it has finished flashing it will send a message which causes the next circle to flash");
        s.append (newLine);
        s.append ("Click the \"Set Random Colour\" button to change the colour of one of the circles.");
        s.append (newLine);
        s.setFont (16.0f);
        s.setColour (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::defaultText, Colours::lightgrey));
        s.draw (g, explanationArea.reduced (10).toFloat());
    }

    void resized() override
    {
        auto area = getLocalBounds().removeFromBottom (40);
        randomColourButton.setBounds (area.removeFromLeft (166) .reduced (8));
        stopButton        .setBounds (area.removeFromRight (166).reduced (8));
    }

private:
    enum { numFlashingComponents = 9 };

    OwnedArray<FlashingComponent> flashingComponents;
    TextButton randomColourButton  { "Set Random Colour" },
               stopButton          { "Stop" };
    Random random;

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        for (int i = 0; i < flashingComponents.size(); ++i)
            if (source == flashingComponents.getUnchecked (i))
                flashingComponents.getUnchecked ((i + 1) % flashingComponents.size())->startFlashing();
    }

    void randomColourButtonClicked()
    {
        // Here we post a new ColourMessage with a random colour to a random flashing component.
        // This will send a message to the component asynchronously and trigger its handleMessage callback
        flashingComponents.getUnchecked (random.nextInt (flashingComponents.size()))->postMessage (new ColourMessage (getRandomBrightColour()));
    }

    void stopButtonClicked()
    {
        for (auto* fc : flashingComponents)
            fc->stopFlashing();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimersAndEventsDemo)
};
