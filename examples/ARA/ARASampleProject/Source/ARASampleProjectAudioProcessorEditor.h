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
                                              private ValueTree::Listener
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    void paint (Graphics&) override;
    void resized() override;

    // DocumentView::Listener overrides
    void visibleTimeRangeChanged (Range<double> newVisibleTimeRange, double pixelsPerSecond) override;

    // ValueTree::Listener overrides
    void valueTreePropertyChanged (juce::ValueTree &treeWhosePropertyHasChanged, const juce::Identifier &property) override;
    void valueTreeChildAdded (juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenAdded) override {};
    void valueTreeChildRemoved (juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override {};
    void valueTreeChildOrderChanged (juce::ValueTree &parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override {};
    void valueTreeParentChanged (juce::ValueTree &treeWhoseParentHasChanged) override {};
private:
    ValueTree* globalSettings;
    std::unique_ptr<DocumentView> documentView;

    TextButton hideTrackHeaderButton;
    TextButton followPlayheadButton;
    TextButton onlySelectedTracksButton;
    Label horizontalZoomLabel, verticalZoomLabel;
    TextButton horizontalZoomInButton, horizontalZoomOutButton;
    TextButton verticalZoomInButton, verticalZoomOutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
