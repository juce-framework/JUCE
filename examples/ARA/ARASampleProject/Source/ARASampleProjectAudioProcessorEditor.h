#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"

class RegionSequenceView;

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
                                              private ScrollBar::Listener,
                                              private juce::Timer
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

    // juce::Timer overrides
    void timerCallback() override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

    const double getPlayheadPositionInSeconds() { return jmax (0.0, playheadPositionInSeconds); }
    const double getPixelsPerSeconds() { return pixelsPerSecond; }

private:
    void rebuildView();
    void clearView();
    void storeRelativePosition();

private:
    // simple utility class to show playhead position
    class PlayheadView : public Component
    {
    public:
        PlayheadView (ARASampleProjectAudioProcessorEditor& owner);
        void paint (Graphics&) override;
    private:
        ARASampleProjectAudioProcessorEditor& owner;
        static constexpr int kPlayheadWidth = 3;
    };

    // we'll be displaying all region sequences in the document in a scrollable view
    Viewport regionSequencesViewPort, tracksViewPort;
    Component regionSequenceListView, tracksView;
    PlayheadView playheadView;

    OwnedArray<RegionSequenceView> regionSequenceViews;

    // custom ScrollBar neededed to have ScrollBar for internal Viewport
    ScrollBar horizontalScrollBar;
    TextButton zoomInButton, zoomOutButton;
    ToggleButton followPlayheadToggleButton;

    bool isViewDirty = false;
    double startTime = 0.0;
    double endTime = 0.0;
    double pixelsPerSecond = 100.0;
    double minPixelsPerSecond = 1.0;
    double maxPixelsPerSecond = 2000.0;
    double playheadPositionInSeconds = 0.0;
    double pixelsUntilPlayhead = 0.0;

    ARASampleProjectAudioProcessor& araSampleProcessor;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
