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

    It is currently work-in-progress, with the goal of making it a reusable base class
    that is part of the JUCE_ARA framework module, not just example code.
    Any JUCE-based ARA plug-in should be able to utilize this to ease its view implementation.

 TODO JUCE_ARA:
    - provide juce::LookAndFeel mechanism so it could be customized for developer needs.
    - configuration for all sizes: track height, ruler height, track header width etc.
    - refactor RulersViews to have RulersView::RulerBase and subclasses.
      maybe we don't need a shared base class other than Component, that would be preferrable.
    - option to show regions including their head and tail
      (for crossfades mostly, renderer will already provide proper samples,
       but time ranges must be adjusted for this and updated if head/tail change)
    - properly compensate for presentation latency (IAudioPresentationLatency/contextPresentationLatency)
      when drawing play head (requires minor additons to the VST and AU wrapper)
    - replace Viewport with better mechanism to avoid integer overflow with long documents and high zoom level.
 */
class DocumentView  : public Component,
                      private ARAEditorView::Listener,
                      private ARADocument::Listener,
                      private juce::Timer
{
public:
    /** Creation.

     @param editorARAExtension  the editor extension used for viewing the document
     @param positionInfo        the time info to be used for showing the playhead
                                This needs to be updated from the processBlock() method of the
                                audio processor showing the editor. The view code can deal with
                                this struct being updated concurrently from the render thread.
     */
    DocumentView (const AudioProcessorEditorARAExtension& editorARAExtension, const AudioPlayHead::CurrentPositionInfo& positionInfo);

    /** Destructor. */
    ~DocumentView();

    /*
     Creates a new PlaybackRegionView which will be owned.
     This allows customizing PlaybackRegionView Component to desired behavior.
     (for example: showing notes)
     */
    virtual PlaybackRegionView* createViewForPlaybackRegion (ARAPlaybackRegion*);

    /*
     Creates a new RegionSequenceView which will be owned.
     This allows customizing RegionSequenceView Component to desired behavior.
     (for example: allow showing cross-fades or interaction between regions)
     */
    virtual RegionSequenceView* createViewForRegionSequence (ARARegionSequence*);

    /*
     Creates a new TrackHeaderView which will be owned.
     This allows customizing TrackHeaderView Component to desired behavior.
     */
    virtual TrackHeaderView* createHeaderViewForRegionSequence (ARARegionSequence*);


    template<typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept { return araExtension.getARAEditorView<EditorView_t>(); }

    template<typename DocumentController_t = ARADocumentController>
    DocumentController_t* getARADocumentController() const noexcept { return araExtension.getARADocumentController<DocumentController_t>(); }

    // total time range
    Range<double> getTimeRange() const { return timeRange; }

    // currently visible time range
    Range<double> getVisibleTimeRange() const;
// TODO JUCE_ARA if we want to make this into a reusable view, then zooming should use this primitive:
//  void setVisibleTimeRange (double start, double end);
//  It would limit the new visibile range to getTimeRange(), trying to keep requested duration unchanged.
//  Another method zoomBy(float factor) can be added on top of this, which deals with keeping the relative
//  playhead positon unchanged if it is visible while zooming, otherwise keeps current view centered.
//  This will be easy to do since it is all in linear time now.

    // may return nullptr
    ARAMusicalContext* getCurrentMusicalContext() const;

    // convert between time and x coordinate
    int getPlaybackRegionsViewsXForTime (double time) const;
    double getPlaybackRegionsViewsTimeForX (int x) const;

    // flag that our view needs to be rebuilt
    void invalidateRegionSequenceViews();

    Component& getPlaybackRegionsView() { return playbackRegionsView; }
    Component& getTrackHeadersView() { return trackHeadersView; }
    Viewport& getTrackHeadersViewport() { return trackHeadersViewport; }
    Viewport& getRulersViewport() { return rulersViewport; }

    AudioFormatManager& getAudioFormatManger() { return audioFormatManger; }

    const AudioPlayHead::CurrentPositionInfo& getPlayHeadPositionInfo() const { return positionInfo; }

    // DocumentView States
    void setShowOnlySelectedRegionSequences (bool newVal);
    bool isShowingOnlySelectedRegionSequences() { return showOnlySelectedRegionSequences; }

    void setIsRulersVisible (bool shouldBeVisible);
    bool isRulersVisible() const { return rulersViewport.isVisible(); }

    void setIsTrackHeadersVisible (bool shouldBeVisible);
    bool isTrackHeadersVisible() const { return trackHeadersViewport.isVisible(); }

    int getTrackHeaderWidth() const { return trackHeadersViewport.getWidth(); }
    int getTrackHeaderMaximumWidth () { return trackHeadersViewport.getMaximumWidth(); }
    int getTrackHeaderMinimumWidth () { return trackHeadersViewport.getMinimumWidth(); }
    void setTrackHeaderWidth (int newWidth);
    void setTrackHeaderMaximumWidth (int newWidth);
    void setTrackHeaderMinimumWidth (int newWidth);

    void setScrollFollowsPlayHead (bool followPlayHead) { scrollFollowsPlayHead = followPlayHead; }
    bool isScrollFollowingPlayHead() const { return scrollFollowsPlayHead; }

    void setPixelsPerSecond (double newValue);
    double getPixelsPerSecond() const { return pixelsPerSecond; }
    bool isMaximumPixelsPerSecond() const { return pixelsPerSecond > minPixelsPerSecond; }
    bool isMinimumPixelsPerSecond() const { return pixelsPerSecond < maxPixelsPerSecond; }

    void setTrackHeight (int newHeight);
    int getTrackHeight() const { return trackHeight; }

    //==============================================================================
    void parentHierarchyChanged() override;
    void paint (Graphics&) override;
    void resized() override;

    // juce::Timer overrides
    void timerCallback() override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& viewSelection) override;
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void didAddRegionSequenceToDocument (ARADocument* document, ARARegionSequence* regionSequence) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

    //==============================================================================
    /**
     A class for receiving events from a DocumentView.

     You can register a DocumentView::Listener with a DocumentView using DocumentView::addListener()
     method, and it will be called on changes.

     @see DocumentView::addListener, DocumentView::removeListener
     */
    class Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() {}

        /** Called when a DocumentView visible time range is changed.
            This happens when being scrolled or zoomed/scaled on the horizontal axis.

         @param newVisibleTimeRange       the new range of the document that's currently visible.
         @param pixelsPerSecond           current pixels per second.
         */
        virtual void visibleTimeRangeChanged (Range<double> newVisibleTimeRange, double pixelsPerSecond) = 0;

        /** Called when a trackHeight is changed.

         @param newTrackHeight           new trackHeight in pixels.
         */
        virtual void trackHeightChanged (int newTrackHeight) {};

        /** Called when a rulersHeight is changed.

         @param newRulersHeight           new rulersHeight in pixels.
         */
        virtual void rulersHeightChanged (int newRulersHeight) {};
    };

    /** Registers a listener that will be called for changes of the DocumentView. */
    void addListener (Listener* listener);

    /** Deregisters a previously-registered listener. */
    void removeListener (Listener* listener);

private:
    void rebuildRegionSequenceViews();
    void updatePlayHeadBounds();

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
        ScrollMasterViewport (DocumentView& documentView) : documentView (documentView) {};
        void visibleAreaChanged (const Rectangle<int>& newVisibleArea) override;
    private:
        DocumentView& documentView;
    };

    // resizable container of TrackHeaderViews
    class TrackHeadersViewport    : public Viewport,
                                    public ComponentBoundsConstrainer
    {
    public:
        TrackHeadersViewport (DocumentView& documentView);
        void setIsResizable (bool isResizable);
        void resized() override;
    private:
        DocumentView& documentView;
        ResizableEdgeComponent resizeBorder;
    };

    const AudioProcessorEditorARAExtension& araExtension;

    OwnedArray<RegionSequenceView> regionSequenceViews;

    ScrollMasterViewport playbackRegionsViewport;
    Component playbackRegionsView;
    PlayHeadView playHeadView;
    TimeRangeSelectionView timeRangeSelectionView;
    TrackHeadersViewport trackHeadersViewport;
    Component trackHeadersView;
    Viewport rulersViewport;
    std::unique_ptr<RulersView> rulersView;

    AudioFormatManager audioFormatManger;

    // Component View States
    bool scrollFollowsPlayHead = true;
    bool showOnlySelectedRegionSequences = true;

    double pixelsPerSecond;
    double maxPixelsPerSecond, minPixelsPerSecond;

    int trackHeight = 80;

    bool regionSequenceViewsAreInvalid = true;
    Range<double> timeRange;

    juce::AudioPlayHead::CurrentPositionInfo lastReportedPosition;
    const juce::AudioPlayHead::CurrentPositionInfo& positionInfo;

    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentView)
};
