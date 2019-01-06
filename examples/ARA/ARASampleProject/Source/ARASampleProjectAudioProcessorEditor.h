#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"
#include "DocumentView.h"

//==============================================================================
/**
    Editor class for ARA sample project
*/
class ARASampleProjectAudioProcessorEditor  : public AudioProcessorEditor,
    public AudioProcessorEditorARAExtension
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
private:
    std::unique_ptr<DocumentView> documentView;

    // controls for DocumentView internal state.
    ToggleButton followPlayheadToggleButton;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
