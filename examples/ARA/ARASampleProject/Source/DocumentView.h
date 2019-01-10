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
    - a setting to make track header width optionally be resizable by user
    - refactor RulersViews to have RulersView::RulerBase and subclasses.
      maybe we don't need a shared base class other than Component, that would be preferrable.
    - option to show regions including their head and tail
      (for crossfades mostly, renderer will already provide proper samples,
       but time ranges must be adjusted for this and updated if head/tail change)
    - optionally visualize ARA selected time range
    - optionally visualize playback cycle state in rulers
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

    // convert between time and x coordinate
    int getPlaybackRegionsViewsXForTime (double time) const;
    double getPlaybackRegionsViewsTimeForX (int x) const;

    // flag that our view needs to be rebuilt
    void invalidateRegionSequenceViews();

    Component& getPlaybackRegionsView() { return playbackRegionsView; }
    Component& getTrackHeadersView() { return trackHeadersView; }
    Viewport& getTrackHeadersViewPort() { return trackHeadersViewPort; }
    Viewport& getRulersViewPort() { return rulersViewPort; }

    AudioFormatManager& getAudioFormatManger() { return audioFormatManger; }

    double getPlayheadTimePosition() const { return playheadTimePosition; }

    // DocumentView States
    void setShowOnlySelectedRegionSequences (bool newVal);
    bool isShowingOnlySelectedRegionSequences() { return showOnlySelectedRegionSequences; }

    void setIsRulersVisible (bool shouldBeVisible);
    bool isRulersVisible() const { return rulersViewPort.isVisible(); }

    void setIsTrackHeadersVisible (bool shouldBeVisible);
    bool isTrackHeadersVisible() const { return trackHeadersViewPort.isVisible(); }

    void setTrackHeaderWidth (int newWidth);
    int getTrackHeaderWidth() const { return trackHeaderWidth; }

    void setScrollFollowsPlaybackState (bool followPlayhead) { shouldFollowPlayhead.setValue (followPlayhead); }
    bool getScrollFollowPlaybackState() const { return shouldFollowPlayhead.getValue(); }
    juce::Value& getScrollFollowsPlaybackStateValue() { return shouldFollowPlayhead; }

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
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;
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

    const AudioProcessorEditorARAExtension& araExtension;

    OwnedArray<RegionSequenceView> regionSequenceViews;

    ScrollMasterViewPort playbackRegionsViewPort;
    Component playbackRegionsView;
    PlayheadView playheadView;
    Viewport trackHeadersViewPort;
    Component trackHeadersView;
    Viewport rulersViewPort;
    std::unique_ptr<RulersView> rulersView;

    AudioFormatManager audioFormatManger;

    // Component View States
    Value shouldFollowPlayhead = Value(true);
    bool showOnlySelectedRegionSequences = false;

    double pixelsPerSecond;
    double maxPixelsPerSecond, minPixelsPerSecond;

    int trackHeight = 80;
    int trackHeaderWidth = 120;

    bool regionSequenceViewsAreInvalid = true;
    Range<double> timeRange;

    double playheadTimePosition = 0.0;
    const juce::AudioPlayHead::CurrentPositionInfo& positionInfo;

    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentView)
};
