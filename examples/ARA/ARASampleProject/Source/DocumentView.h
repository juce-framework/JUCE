#pragma once

#include "JuceHeader.h"

class RulersView;
class TrackHeaderView;
class RegionSequenceView;
class PlaybackRegionView;

//==============================================================================
/**
 DocumentView Class -
    This class provides basic foundation to show the ARA Document as well as
    their current selection state

    currently, even though it is developed under Celemony's repository it's
    a non-suprvised branch to be reviewed and decided what to do with later.
 
 Goals (once finished):
    - become part of ARA or JUCE-ARA SDK
    - provide juce::LookAndFeel mechanism so it could be customized for developer needs.
    - become a baseclass to use as view component for most ARA-JUCE based products..
 
 */
class DocumentView  : public AudioProcessorEditor,
public AudioProcessorEditorARAExtension,
private ARAEditorView::Listener,
private ARADocument::Listener,
private juce::Timer
{
public:
    DocumentView (AudioProcessor&);
    ~DocumentView();

    /*
     Creates a new PlaybackRegionView which will be owned.
     This allows customizing PlaybackRegionView Component to desired behavior.
     (for example: showing notes)
     */
    virtual PlaybackRegionView* getViewForPlaybackRegion (ARAPlaybackRegion*);

    /*
     Creates a new RegionSequenceView which will be owned.
     This allows customizing RegionSequenceView Component to desired behavior.
     (for example: allow showing cross-fades or interaction between regions)
     */
    virtual RegionSequenceView* getViewForRegionSequence (ARARegionSequence*);

    /*
     Creates a new TrackHeaderView which will be owned.
     This allows customizing TrackHeaderView Component to desired behavior.
     */
    virtual TrackHeaderView* getHeaderViewForRegionSequence (ARARegionSequence*);

    // total time range
    Range<double> getTimeRange() const { return Range<double> (startTime, endTime); }

    // visible time range
    Range<double> getVisibleTimeRange() const;
    // TODO JUCE_ARA if we want to make this into a reusable view, then zooming should use this primitive:
    //  void setVisibleTimeRange (double start, double end);
    //  It would limit the new visibile range to getTimeRange(), trying to keep requested duration unchanged.
    //  Another method zoomBy(float factor) can be added on top of this, which deals with keeping the relative
    //  playhead positon unchanged if it is visible while zooming, otherwise keeps current view centered.
    //  This will be easy to do since it is all in linear time now.

    // flag that our view needs to be rebuilt
    void invalidateRegionSequenceViews() { regionSequenceViewsAreInvalid = true; }

    Component& getPlaybackRegionsView() { return playbackRegionsView; }
    Component& getTrackHeadersView() { return trackHeadersView; }
    Viewport& getTrackHeadersViewPort() { return trackHeadersViewPort; }
    Viewport& getRulersViewPort() { return rulersViewPort; }

    int getPlaybackRegionsViewsXForTime (double time) const;
    double getPlaybackRegionsViewsTimeForX (int x) const;

    /*
     Sets a juce::AudioPlayHead::CurrentPositionInfo pointer that
     should be used to show playhead.
     Note: CurrentPositionInfo is only valid within processBlock calls,
           The object should be updated only within audio thread.
     */
    void setCurrentPositionInfo (const AudioPlayHead::CurrentPositionInfo*);

    double getPlayheadTimePosition() const { return playheadTimePosition; }

    void setShowOnlySelectedRegionSequence (bool newVal) { showOnlySelectedRegionSequence = newVal; }
    bool isShowingOnlySelectedRegionSequence() { return showOnlySelectedRegionSequence; }

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // juce::Timer overrides
    void timerCallback() override;

    // ARAEditorView::Listener overrides
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

private:
    void rebuildRegionSequenceViews();
    void updatePlayheadBounds();

private:
    // simple utility class to show playhead position
    class PlayheadView : public Component
    {
    public:
        PlayheadView (DocumentView& documentView);
        void paint (Graphics&) override;
    private:
        DocumentView& documentView;
    };

    // simple utility class to partially sync scroll postions of our view ports
    class ScrollMasterViewPort : public Viewport
    {
    public:
        ScrollMasterViewPort (DocumentView& documentView) : documentView (documentView) {};
        void visibleAreaChanged (const Rectangle<int>& newVisibleArea) override;
    private:
        DocumentView& documentView;
    };

    OwnedArray<RegionSequenceView> regionSequenceViews;

    ScrollMasterViewPort playbackRegionsViewPort;
    Component playbackRegionsView;
    PlayheadView playheadView;
    Viewport trackHeadersViewPort;
    Component trackHeadersView;
    Viewport rulersViewPort;
    std::unique_ptr<RulersView> rulersView;

    Label horizontalZoomLabel, verticalZoomLabel;
    TextButton horizontalZoomInButton, horizontalZoomOutButton,
               verticalZoomInButton, verticalZoomOutButton;
    ToggleButton followPlayheadToggleButton;

    bool regionSequenceViewsAreInvalid = true;
    bool showOnlySelectedRegionSequence = false;
    double startTime = 0.0;
    double endTime = 1.0;
    double pixelsPerSecond = 0.0;
    double playheadTimePosition = 0.0;
    int trackHeight = 80;
    
    const juce::AudioPlayHead::CurrentPositionInfo* positionInfoPtr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentView)
};
