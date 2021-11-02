#pragma once

#include "JuceHeader.h"
#include "MusicalContextView.h"

class RegionSequenceViewContainer;
class PlaybackRegionView;

//==============================================================================
/**
 DocumentView Class -
    This class manages a visual representation of the ARA document as well as
    the ARA host selection and playback state. 
 */
class DocumentView    : public Component,
                        private ARAEditorView::Listener,
                        private ARADocument::Listener,
                        private juce::Timer
{
public:
    DocumentView (ARAEditorView* editorView, const AudioPlayHead::CurrentPositionInfo& positionInfo);
    ~DocumentView();

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARAViewSelection& viewSelection) override;
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

    // ARA getters
    ARAEditorView* getARAEditorView() const noexcept { return editorView; }
    ARADocumentController* getDocumentController() const noexcept { return getARAEditorView()->getDocumentController<ARADocumentController>(); }
    ARADocument* getDocument() const noexcept { return getDocumentController()->getDocument<ARADocument>(); }

    // total time range
    Range<double> getTimeRange() const { return timeRange; }

    // flag that the time range covered by the playback regions needs to be recalculated
    void invalidateTimeRange();

    // currently visible time range
    Range<double> getVisibleTimeRange() const;

    // musical context view access
    const MusicalContextView& getMusicalContextView() const { return musicallContextView; }

    // convert between time and x coordinate
    int getPlaybackRegionsViewsXForTime (double time) const;
    double getPlaybackRegionsViewsTimeForX (int x) const;

    // view configuration
    void setShowOnlySelectedRegionSequences (bool newVal);
    bool isShowingOnlySelectedRegionSequences() { return showOnlySelectedRegionSequences; }

    void setScrollFollowsPlayHead (bool followPlayHead) { scrollFollowsPlayHead = followPlayHead; }
    bool isScrollFollowingPlayHead() const { return scrollFollowsPlayHead; }

    void zoomBy (double factor);

    // misc. getters
    Component& getPlaybackRegionsView() { return playbackRegionsView; }
    Component& getRegionSequenceHeadersView() { return regionSequenceHeadersView; }
    Viewport& getRegionSequenceHeadersViewport() { return regionSequenceHeadersViewport; }
    Viewport& getMusicalContextViewport() { return musicalContextViewport; }

    AudioFormatManager& getAudioFormatManger() { return audioFormatManger; }

    const AudioPlayHead::CurrentPositionInfo& getPlayHeadPositionInfo() const { return positionInfo; }

    // juce::Component overrides
    void parentHierarchyChanged() override;
    void paint (Graphics&) override;
    void resized() override;

    // juce::Timer overrides
    void timerCallback() override;

    //==============================================================================
private:
    void invalidateRegionSequenceViewContainers();
    void rebuildRegionSequenceViewContainers();
    void calculateTimeRange();

private:
    // simple utility class to show playhead position
    class PlayHeadView    : public Component
    {
    public:
        PlayHeadView (DocumentView& documentView);
        void paint (Graphics&) override;
    private:
        DocumentView& documentView;
    };

    // simple utility class to show selected time range
    class TimeRangeSelectionView  : public Component
    {
    public:
        TimeRangeSelectionView (DocumentView& documentView);
        void paint (Graphics&) override;
    private:
        DocumentView& documentView;
    };

    // simple utility class to partially sync scroll postions of our view ports
    class ScrollMasterViewport    : public Viewport
    {
    public:
        ScrollMasterViewport (DocumentView& docView) : documentView (docView) {}
        void visibleAreaChanged (const Rectangle<int>& newVisibleArea) override;
    private:
        DocumentView& documentView;
    };

    ARAEditorView* const editorView;

    OwnedArray<RegionSequenceViewContainer> regionSequenceViewContainers;

    ScrollMasterViewport playbackRegionsViewport;
    Component playbackRegionsView;
    PlayHeadView playHeadView;
    TimeRangeSelectionView timeRangeSelectionView;
    Viewport regionSequenceHeadersViewport;
    Component regionSequenceHeadersView;
    Viewport musicalContextViewport;
    MusicalContextView musicallContextView;

    AudioFormatManager audioFormatManger;

    // Component View States
    bool scrollFollowsPlayHead { true };
    bool showOnlySelectedRegionSequences { true };

    double pixelsPerSecond;

    bool regionSequenceViewsAreInvalid { true };
    bool timeRangeIsInvalid { true };
    Range<double> timeRange;

    juce::AudioPlayHead::CurrentPositionInfo lastReportedPosition;
    const juce::AudioPlayHead::CurrentPositionInfo& positionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentView)
};
