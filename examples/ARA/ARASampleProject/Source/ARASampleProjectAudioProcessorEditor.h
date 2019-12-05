#pragma once

#include "JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"
#include "DocumentView.h"

//==============================================================================
/**
    Editor class for ARA sample project
*/
class ARASampleProjectAudioProcessorEditor  : public AudioProcessorEditor,
                                              public AudioProcessorEditorARAExtension,
                                              private juce::Timer
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);

    void paint (Graphics&) override;
    void resized() override;

    // juce::Timer
    void timerCallback() override;

private:
    std::unique_ptr<DocumentView> documentView;

    TextButton followPlayHeadButton;
    TextButton onlySelectedTracksButton;
    Label horizontalZoomLabel;
    Label playheadLinearPositionLabel, playheadMusicalPositionLabel;
    TextButton horizontalZoomInButton, horizontalZoomOutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
