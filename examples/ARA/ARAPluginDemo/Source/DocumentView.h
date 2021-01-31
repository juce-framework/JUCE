#pragma once

#include "MusicalContextView.h"

class RegionSequenceViewContainer;
class PlaybackRegionView;

//==============================================================================
/**
 DocumentView Class -
    This class manages a visual representation of the ARA document as well as
    the ARA host selection and playback state. 
 */
class DocumentView    : public juce::Component,
                        private juce::ARAEditorView::Listener,
                        private juce::ARADocument::Listener,
                        private juce::Timer
{
public:
    DocumentView (juce::ARAEditorView* editorView, const juce::AudioPlayHead::CurrentPositionInfo& positionInfo);
    ~DocumentView() override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const juce::ARAViewSelection& viewSelection) override;
    void onHideRegionSequences (std::vector<juce::ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (juce::ARADocument* document) override;
    void didReorderRegionSequencesInDocument (juce::ARADocument* document) override;

    // ARA getters
    juce::ARAEditorView* getARAEditorView() const noexcept { return editorView; }
    juce::ARADocumentController* getDocumentController() const noexcept { return getARAEditorView()->getDocumentController<juce::ARADocumentController>(); }
    juce::ARADocument* getDocument() const noexcept { return getDocumentController()->getDocument<juce::ARADocument>(); }

    // total time range
    juce::Range<double> getTimeRange() const { return timeRange; }

    // flag that the time range covered by the playback regions needs to be recalculated
    void invalidateTimeRange();

    // currently visible time range
    juce::Range<double> getVisibleTimeRange() const;

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
    juce::Viewport& getRegionSequenceHeadersViewport() { return regionSequenceHeadersViewport; }
    juce::Viewport& getMusicalContextViewport() { return musicalContextViewport; }

    juce::AudioFormatManager& getAudioFormatManger() { return audioFormatManger; }

    const juce::AudioPlayHead::CurrentPositionInfo& getPlayHeadPositionInfo() const { return positionInfo; }

    // juce::Component overrides
    void parentHierarchyChanged() override;
    void paint (juce::Graphics&) override;
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
    class PlayHeadView    : public juce::Component
    {
    public:
        PlayHeadView (DocumentView& documentView);
        void paint (juce::Graphics&) override;
    private:
        DocumentView& documentView;
    };

    // simple utility class to show selected time range
    class TimeRangeSelectionView  : public juce::Component
    {
    public:
        TimeRangeSelectionView (DocumentView& documentView);
        void paint (juce::Graphics&) override;
    private:
        DocumentView& documentView;
    };

    // simple utility class to show a common tooltip for multiple views
    class TooltipComponent    : public juce::Component,
                                public juce::SettableTooltipClient
    {
    };

    // simple utility class to partially sync scroll postions of our view ports
    class ScrollMasterViewport    : public juce::Viewport
    {
    public:
        ScrollMasterViewport (DocumentView& docView) : documentView (docView) {}
        void visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea) override;
    private:
        DocumentView& documentView;
    };

    juce::ARAEditorView* const editorView;

    juce::OwnedArray<RegionSequenceViewContainer> regionSequenceViewContainers;

    ScrollMasterViewport playbackRegionsViewport;
    juce::Component playbackRegionsView;
    PlayHeadView playHeadView;
    TimeRangeSelectionView timeRangeSelectionView;
    juce::Viewport regionSequenceHeadersViewport;
    juce::Component regionSequenceHeadersView;
    TooltipComponent regionSequenceHeadersTooltipView;
    juce::Viewport musicalContextViewport;
    MusicalContextView musicallContextView;

    juce::AudioFormatManager audioFormatManger;

    // Component View States
    bool scrollFollowsPlayHead { true };
    bool showOnlySelectedRegionSequences { true };

    double pixelsPerSecond;

    bool regionSequenceViewsAreInvalid { true };
    bool timeRangeIsInvalid { true };
    juce::Range<double> timeRange;

    juce::AudioPlayHead::CurrentPositionInfo lastReportedPosition;
    const juce::AudioPlayHead::CurrentPositionInfo& positionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentView)
};
