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

    void paint (Graphics&) override;
    void resized() override;

    // DocumentView::Listener overrides
    void visibleTimeRangeChanged (Range<double> newVisibleTimeRange, double pixelsPerSecond) override;
    void trackHeightChanged (int newTrackHeight) override;

private:
    std::unique_ptr<DocumentView> documentView;
    void loadEditorDefaultSettings();

    TextButton hideTrackHeaderButton;
    TextButton followPlayheadButton;
    TextButton onlySelectedTracksButton;
    Label horizontalZoomLabel, verticalZoomLabel;
    TextButton horizontalZoomInButton, horizontalZoomOutButton;
    TextButton verticalZoomInButton, verticalZoomOutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
