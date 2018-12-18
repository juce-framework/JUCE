#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"

class RegionSequenceView;
class RulersView;

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
    void getVisibleTimeRange (double& start, double& end) const;

    // flag that our view needs to be rebuilt
    void invalidateRegionSequenceViews() { regionSequenceViewsAreInvalid = true; }

    Component& getTrackHeadersView() { return trackHeadersView; }
    Component& getPlaybackRegionsView() { return playbackRegionsView; }

    int getPlaybackRegionsViewsXForTime (double time) const;
    double getPlaybackRegionsViewsTimeForX (int x) const;
    double getPixelsPerSecond () const { return pixelsPerSecond; }

    double getPlayheadPositionInSeconds() const { return playheadPositionInSeconds; }

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // ScrollBar::Listener overrides
    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

    // juce::Timer overrides
    void timerCallback() override;

    // ARAEditorView::Listener overrides
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

private:
    void rebuildRegionSequenceViews();
    void storeRelativePosition();

private:
    // simple utility class to show playhead position
    class PlayheadView : public Component
    {
    public:
        PlayheadView (ARASampleProjectAudioProcessorEditor& editorComponent);
        void paint (Graphics&) override;
    private:
        ARASampleProjectAudioProcessorEditor& editorComponent;
        static constexpr int kPlayheadWidth = 3;
    };

    OwnedArray<RegionSequenceView> regionSequenceViews;

    Viewport trackHeadersViewPort, rulersViewPort, playbackRegionsViewPort;
    Component trackHeadersView, playbackRegionsView;
    std::unique_ptr<RulersView> rulersView;
    PlayheadView playheadView;

    TextButton zoomInButton, zoomOutButton;
    ToggleButton followPlayheadToggleButton;

    bool regionSequenceViewsAreInvalid = true;
    double startTime = 0.0;
    double endTime = 0.0;
    double pixelsPerSecond = 0.0;
    double playheadPositionInSeconds = 0.0;
    double pixelsUntilPlayhead = 0.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
