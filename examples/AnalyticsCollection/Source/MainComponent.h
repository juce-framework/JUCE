#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "DemoAnalyticsEventTypes.h"

class MainContentComponent   : public Component,
                               private Button::Listener
{
public:
    //==============================================================================
    MainContentComponent()
    {
        crashButton.addListener (this);

        addAndMakeVisible (eventButton);
        addAndMakeVisible (crashButton);

        setSize (300, 200);

        StringPairArray logButtonPressParameters;
        logButtonPressParameters.set ("id", "a");
        logEventButtonPress = new ButtonTracker (eventButton, "button_press", logButtonPressParameters);
    }

    ~MainContentComponent()
    {
        crashButton.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        eventButton.centreWithSize (100, 40);
        eventButton.setBounds (eventButton.getBounds().translated (0, 25));
        crashButton.setBounds (eventButton.getBounds().translated (0, -50));
    }

private:
    //==============================================================================
    void buttonClicked (Button*) override
    {
        // In a more advanced application you would probably use a different event
        // type here.
        Analytics::getInstance()->logEvent ("crash", {}, DemoAnalyticsEventTypes::event);
        Analytics::getInstance()->getDestinations().clear();
        JUCEApplication::getInstance()->shutdown();
    }

    TextButton eventButton { "Press me!" }, crashButton { "Simulate crash!" };
    ScopedPointer<ButtonTracker> logEventButtonPress;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
