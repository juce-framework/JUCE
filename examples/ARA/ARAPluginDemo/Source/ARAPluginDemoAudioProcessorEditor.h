#pragma once

#include "ARAPluginDemoAudioProcessor.h"
#include "DocumentView.h"

//==============================================================================
/**
    Editor class for ARAPluginDemo
*/
class ARAPluginDemoAudioProcessorEditor   : public juce::AudioProcessorEditor,
                                            public juce::AudioProcessorEditorARAExtension,
                                            private juce::Timer
{
public:
    ARAPluginDemoAudioProcessorEditor (ARAPluginDemoAudioProcessor&);

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // juce::Timer
    void timerCallback() override;

private:
    std::unique_ptr<DocumentView> documentView;

    juce::TooltipWindow tooltip;
    juce::TextButton onlySelectedTracksButton;
    juce::TextButton followPlayHeadButton;
    juce::Label playheadLinearPositionLabel, playheadMusicalPositionLabel;
    juce::TextButton horizontalZoomInButton, horizontalZoomOutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginDemoAudioProcessorEditor)
};
