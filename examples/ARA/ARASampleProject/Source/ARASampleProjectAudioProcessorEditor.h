#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"
#include "RegionSequenceView.h"

//==============================================================================
/**
    Editor class for ARA sample project
    This class manages the UI we use to display region sequences in the
    ARA document as well as their current selection state
*/
class ARASampleProjectAudioProcessorEditor: public AudioProcessorEditor,
                                            public AudioProcessorEditorARAExtension,
                                            public ARAEditorView::Listener,
                                            public ARADocument::Listener
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    // total visible time range
    void getTimeRange (double& start, double& end) const { start = startTime; end = endTime; }

    // flag that our view needs to be rebuilt
    void setDirty() { isViewDirty = true; }

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // ARAEditorView::Listener overrides
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void doEndEditing (ARADocument* document) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

public:
    static constexpr double kPixelsPerSecond = 100.0;
    static constexpr double kPadSeconds = 1.0;
    static constexpr int kMinWidth = 500;
    static constexpr int kWidth = 1000;
    static constexpr int kMinHeight = 1 * RegionSequenceView::kHeight;
    static constexpr int kHeight = 5 * RegionSequenceView::kHeight;

private:
    void rebuildView();
    void clearView();

private:

    // we'll be displaying all region sequences in the document in a scrollable view
    Viewport regionSequenceViewPort;
    Component regionSequenceListView;

    OwnedArray<RegionSequenceView> regionSequenceViews;

    bool isViewDirty = false;
    double startTime = 0.0;
    double endTime = 0.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
