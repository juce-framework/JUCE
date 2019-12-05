#pragma once

#include "JuceHeader.h"

class RulersView;
class TrackHeaderView;
class RegionSequenceView;
class PlaybackRegionView;

//==============================================================================
/**
 DocumentView Class -
    This class manages a visual representation of the ARA document as well as
    the ARA host selection and playback state. 

 TODO JUCE_ARA:
    - maybe add option to show regions including their head and tail?
      (for crossfades mostly, renderer will already provide proper samples,
       but time ranges must be adjusted for this and updated if head/tail change)
    - properly compensate for presentation latency (IAudioPresentationLatency/contextPresentationLatency)
      when drawing play head (requires minor additons to the VST and AU wrapper)
 */
class DocumentView  : public Component,
                      private ARAEditorView::Listener,
                      private ARADocument::Listener,
                      private juce::Timer
{
public:
    DocumentView (ARAEditorView* editorView, const AudioPlayHead::CurrentPositionInfo& positionInfo);
    ~DocumentView();

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& viewSelection) override;
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void didAddRegionSequenceToDocument (ARADocument* document, ARARegionSequence* regionSequence) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

    // ARA getters
    ARAEditorView* getARAEditorView() const noexcept { return editorView; }
    ARADocumentController* getDocumentController() const noexcept { return getARAEditorView()->getDocumentController<ARADocumentController>(); }
    ARADocument* getDocument() const noexcept { return getDocumentController()->getDocument<ARADocument>(); }

    // total time range
    Range<double> getTimeRange() const { return timeRange; }

    // currently visible time range
    Range<double> getVisibleTimeRange() const;

    // may return nullptr
    ARAMusicalContext* getCurrentMusicalContext() const;

    // convert between time and x coordinate
    int getPlaybackRegionsViewsXForTime (double time) const;
    double getPlaybackRegionsViewsTimeForX (int x) const;

    // flag that our track list needs to be rebuilt
    void invalidateRegionSequenceViews();

    // flag that the time range covered by the playback regions needs to be recalculated
    void invalidateTimeRange();

    // view configuration
    void setShowOnlySelectedRegionSequences (bool newVal);
    bool isShowingOnlySelectedRegionSequences() { return showOnlySelectedRegionSequences; }

    void setScrollFollowsPlayHead (bool followPlayHead) { scrollFollowsPlayHead = followPlayHead; }
    bool isScrollFollowingPlayHead() const { return scrollFollowsPlayHead; }

    void zoomBy (double factor);

    // misc. getters
    Component& getPlaybackRegionsView () { return playbackRegionsView; }
    Component& getTrackHeadersView () { return trackHeadersView; }
    Viewport& getTrackHeadersViewport () { return trackHeadersViewport; }
    Viewport& getRulersViewport () { return rulersViewport; }

    AudioFormatManager& getAudioFormatManger () { return audioFormatManger; }

    const AudioPlayHead::CurrentPositionInfo& getPlayHeadPositionInfo () const { return positionInfo; }

    // juce::Component overrides
    void parentHierarchyChanged() override;
    void paint (Graphics&) override;
    void resized() override;

    // juce::Timer overrides
    void timerCallback() override;

    //==============================================================================
private:
    void rebuildRegionSequenceViews();
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

    OwnedArray<RegionSequenceView> regionSequenceViews;

    ScrollMasterViewport playbackRegionsViewport;
    Component playbackRegionsView;
    PlayHeadView playHeadView;
    TimeRangeSelectionView timeRangeSelectionView;
    Viewport trackHeadersViewport;
    Component trackHeadersView;
    Viewport rulersViewport;
    std::unique_ptr<RulersView> rulersView;

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
