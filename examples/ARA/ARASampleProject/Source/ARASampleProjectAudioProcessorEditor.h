#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"
#include "DocumentView.h"

//==============================================================================
/**
    Editor class for ARA sample project
*/
class ARASampleProjectAudioProcessorEditor  : public AudioProcessorEditor,
                                              public AudioProcessorEditorARAExtension,
                                              private juce::Value::Listener
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // Value::Listener
    void valueChanged (juce::Value &value) override;
private:
    std::unique_ptr<DocumentView> documentView;

    // controls for DocumentView internal state.
    ToggleButton followPlayheadToggleButton;
    Label horizontalZoomLabel, verticalZoomLabel;
    TextButton horizontalZoomInButton, horizontalZoomOutButton,
    verticalZoomInButton, verticalZoomOutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
