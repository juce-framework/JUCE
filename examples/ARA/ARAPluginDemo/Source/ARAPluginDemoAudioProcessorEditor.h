#pragma once

#include "JuceHeader.h"
#include "ARAPluginDemoAudioProcessor.h"
#include "DocumentView.h"

//==============================================================================
/**
    Editor class for ARAPluginDemo
*/
class ARAPluginDemoAudioProcessorEditor   : public AudioProcessorEditor,
                                            public AudioProcessorEditorARAExtension,
                                            private juce::Timer
{
public:
    ARAPluginDemoAudioProcessorEditor (ARAPluginDemoAudioProcessor&);

    void paint (Graphics&) override;
    void resized() override;

    // juce::Timer
    void timerCallback() override;

private:
    std::unique_ptr<DocumentView> documentView;

    TextButton followPlayHeadButton;
    TextButton onlySelectedTracksButton;
    Label playheadLinearPositionLabel, playheadMusicalPositionLabel;
    TextButton horizontalZoomInButton, horizontalZoomOutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginDemoAudioProcessorEditor)
};
