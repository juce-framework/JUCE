#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MainContentComponent   : public Component
{
public:
    //==============================================================================
    MainContentComponent()
    {
        addAndMakeVisible (eventButton);

        setSize (300, 200);

        StringPairArray logButtonPressParameters;
        logButtonPressParameters.set ("id", "a");
        logEventButtonPress = new ButtonTracker (eventButton, "button_press", logButtonPressParameters);
    }

    ~MainContentComponent() {}

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        eventButton.centreWithSize (100, 50);
    }

private:
    //==============================================================================
    TextButton eventButton { "Press me!" };
    ScopedPointer<ButtonTracker> logEventButtonPress;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
