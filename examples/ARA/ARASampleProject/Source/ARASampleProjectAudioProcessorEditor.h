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
                                              private DocumentView::Listener,
                                              private juce::Timer
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

    // DocumentView::Listener overrides
    void visibleTimeRangeChanged (Range<double> newVisibleTimeRange, double pixelsPerSecond) override;
    void trackHeightChanged (int newTrackHeight) override;

    // juce::Timer
    void timerCallback() override;

private:
    std::unique_ptr<DocumentView> documentView;

    TextButton hideTrackHeaderButton;
    TextButton followPlayHeadButton;
    TextButton onlySelectedTracksButton;
    Label horizontalZoomLabel, verticalZoomLabel;
    Label playheadLinearPositionLabel, playheadMusicalPositionLabel;
    TextButton horizontalZoomInButton, horizontalZoomOutButton;
    TextButton verticalZoomInButton, verticalZoomOutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
