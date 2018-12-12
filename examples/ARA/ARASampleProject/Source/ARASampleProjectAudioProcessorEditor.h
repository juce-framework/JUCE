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
class ARASampleProjectAudioProcessorEditor  : public AudioProcessorEditor,
                                              public AudioProcessorEditorARAExtension,
                                              private ARAEditorView::Listener,
                                              private ARADocument::Listener,
                                              private ScrollBar::Listener
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    // total time range
    void getTimeRange (double& start, double& end) const { start = startTime; end = endTime; }

    // total visible time range
    void getVisibleTimeRange (double& start, double& end);

    // flag that our view needs to be rebuilt
    void setDirty() { isViewDirty = true; }

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // ScrollBar::Listener overrides
    void scrollBarMoved (ScrollBar *scrollBarThatHasMoved, double newRangeStart) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

public:
    static constexpr int kMinWidth = 500;
    static constexpr int kWidth = 1000;
    static constexpr int kMinHeight = 1 * RegionSequenceView::kHeight;
    static constexpr int kHeight = 5 * RegionSequenceView::kHeight;
    static constexpr int kStatusBarHeight = 20;

private:
    void rebuildView();
    void clearView();

private:

    // we'll be displaying all region sequences in the document in a scrollable view
    Viewport regionSequencesViewPort, tracksViewPort;
    Component regionSequenceListView, tracksView;

    OwnedArray<RegionSequenceView> regionSequenceViews;
    // custom ScrollBar neededed to have ScrollBar for internal Viewport
    ScrollBar horizontalScrollBar;
    TextButton zoomInButton, zoomOutButton;

    bool isViewDirty = false;
    double startTime = 0.0;
    double endTime = 0.0;
    double pixelsPerSecond = 100.0;
    double minPixelsPerSecond = 1.0;
    double maxPixelsPerSecond = 2000.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
