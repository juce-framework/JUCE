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
                                              private DocumentView::Listener
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // Value::Listener
    void timelineSelectionChanged (double newRangeStartInSeconds, double newRangeEndInSeconds, double pixelsPerSecond);
private:
    std::unique_ptr<DocumentView> documentView;

    // controls for DocumentView internal state.
    ToggleButton followPlayheadToggleButton;
    Label horizontalZoomLabel, verticalZoomLabel;
    TextButton horizontalZoomInButton, horizontalZoomOutButton,
    verticalZoomInButton, verticalZoomOutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
