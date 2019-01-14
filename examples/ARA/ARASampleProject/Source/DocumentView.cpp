#include "DocumentView.h"

#include "RegionSequenceView.h"
#include "TrackHeaderView.h"
#include "PlaybackRegionView.h"
#include "RulersView.h"

constexpr double kMinSecondDuration = 1.0;
constexpr double kMinBorderSeconds = 1.0;

//==============================================================================
DocumentView::DocumentView (const AudioProcessorEditorARAExtension& extension, const AudioPlayHead::CurrentPositionInfo& posInfo)
    : araExtension (extension),
      playbackRegionsViewport (*this),
      playHeadView (*this),
      trackHeadersViewport (*this),
      timeRange (-kMinBorderSeconds, kMinSecondDuration + kMinBorderSeconds),
      positionInfo (posInfo)
{
    if (! araExtension.isARAEditorView())
    {
        // you shouldn't create a DocumentView if your instance can't support ARA.
        // notify user on your AudioProcessorEditorView or provide your own capture
        // alternative to ARA workflow.
        jassertfalse;
        return;
    }

    playHeadView.setAlwaysOnTop (true);
    playbackRegionsView.addAndMakeVisible (playHeadView);

    playbackRegionsViewport.setScrollBarsShown (true, true, false, false);
    playbackRegionsViewport.setViewedComponent (&playbackRegionsView, false);
    addAndMakeVisible (playbackRegionsViewport);

    trackHeadersViewport.setScrollBarsShown (false, false, false, false);
    trackHeadersViewport.setViewedComponent (&trackHeadersView, false);
    addAndMakeVisible (trackHeadersViewport);

    rulersView.reset (new RulersView (*this));
    rulersViewport.setScrollBarsShown (false, false, false, false);
    rulersViewport.setViewedComponent (rulersView.get(), false);
    addAndMakeVisible (rulersViewport);

    getARAEditorView()->addListener (this);
    getARADocumentController()->getDocument<ARADocument>()->addListener (this);

    lastReportedPosition.resetToDefault();

    startTimerHz (60);
}

DocumentView::~DocumentView()
{
    if (! araExtension.isARAEditorView())
        return;

    getARADocumentController()->getDocument<ARADocument>()->removeListener (this);
    getARAEditorView()->removeListener (this);
}

//==============================================================================
PlaybackRegionView* DocumentView::createViewForPlaybackRegion (ARAPlaybackRegion* playbackRegion)
{
    return new PlaybackRegionView (*this, playbackRegion);
}

TrackHeaderView* DocumentView::createHeaderViewForRegionSequence (ARARegionSequence* regionSequence)
{
    return new TrackHeaderView (getARAEditorView(), regionSequence);
}

RegionSequenceView* DocumentView::createViewForRegionSequence (ARARegionSequence* regionSequence)
{
    return new RegionSequenceView (*this, regionSequence);
}

//==============================================================================
Range<double> DocumentView::getVisibleTimeRange() const
{
    const double start = getPlaybackRegionsViewsTimeForX (playbackRegionsViewport.getViewArea().getX());
    const double end = getPlaybackRegionsViewsTimeForX (playbackRegionsViewport.getViewArea().getRight());
    return { start, end };
}

int DocumentView::getPlaybackRegionsViewsXForTime (double time) const
{
    return roundToInt ((time - timeRange.getStart()) / timeRange.getLength() * playbackRegionsView.getWidth());
}

double DocumentView::getPlaybackRegionsViewsTimeForX (int x) const
{
    return timeRange.getStart() + ((double) x / (double) playbackRegionsView.getWidth()) * timeRange.getLength();
}

void DocumentView::invalidateRegionSequenceViews()
{
    if (getARADocumentController()->isHostEditingDocument() || getParentComponent() == nullptr)
        regionSequenceViewsAreInvalid = true;
    else
        rebuildRegionSequenceViews();
}

//==============================================================================
void DocumentView::setShowOnlySelectedRegionSequences (bool newVal)
{
    showOnlySelectedRegionSequences = newVal;
    invalidateRegionSequenceViews();
}

void DocumentView::setIsRulersVisible (bool shouldBeVisible)
{
    rulersViewport.setVisible (shouldBeVisible);
    resized();
}

void DocumentView::setIsTrackHeadersVisible (bool shouldBeVisible)
{
    trackHeadersViewport.setVisible (shouldBeVisible);
    resized();
}

void DocumentView::setTrackHeaderWidth (int newWidth)
{
    trackHeadersViewport.setBoundsForComponent (&trackHeadersViewport, trackHeadersViewport.getBounds().withWidth (newWidth), false, false, false, true);
}

void DocumentView::setTrackHeaderMaximumWidth (int newWidth)
{
    trackHeadersViewport.setIsResizable (getTrackHeaderMinimumWidth() < newWidth);
    trackHeadersViewport.setMaximumWidth (newWidth);
    trackHeadersViewport.checkComponentBounds (&trackHeadersViewport);
}

void DocumentView::setTrackHeaderMinimumWidth (int newWidth)
{
    trackHeadersViewport.setIsResizable (newWidth < getTrackHeaderMaximumWidth());
    trackHeadersViewport.setMinimumWidth (newWidth);
    trackHeadersViewport.checkComponentBounds (&trackHeadersViewport);
}

void DocumentView::setPixelsPerSecond (double newValue)
{
    if (newValue == pixelsPerSecond)
        return;

    pixelsPerSecond = newValue;
    resized();  // this will constrain pixelsPerSecond range, also it might call again after rounding.

    listeners.callExpectingUnregistration ([&] (Listener& l)
                                           {
                                               l.visibleTimeRangeChanged (getVisibleTimeRange(), pixelsPerSecond);
                                           });
}

void DocumentView::setTrackHeight (int newHeight)
{
    if (newHeight == trackHeight)
        return;

    trackHeight = newHeight;
    resized();

    listeners.callExpectingUnregistration ([&] (Listener& l)
                                           {
                                               l.trackHeightChanged (trackHeight);
                                           });
}

//==============================================================================
void DocumentView::parentHierarchyChanged()
{
    // trigger lazy initial update after construction if needed
    if (regionSequenceViewsAreInvalid && ! getARADocumentController()->isHostEditingDocument())
        rebuildRegionSequenceViews();
}

void DocumentView::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void DocumentView::resized()
{
    // store visible playhead postion (in main view coordinates)
    int previousPlayHeadX = getPlaybackRegionsViewsXForTime (lastReportedPosition.timeInSeconds) - playbackRegionsViewport.getViewPosition().getX();

    // calculate maximum visible time range
    timeRange = { 0.0, 0.0 };
    if (! regionSequenceViews.isEmpty())
    {
        bool isFirst = true;
        for (auto v : regionSequenceViews)
        {
            if (v->isEmpty())
                continue;

            const auto sequenceTimeRange = v->getTimeRange();
            if (isFirst)
            {
                timeRange = sequenceTimeRange;
                isFirst = false;
                continue;
            }

            timeRange = timeRange.getUnionWith (sequenceTimeRange);
        }
    }

    // ensure visible range covers kMinSecondDuration
    if (timeRange.getLength() < kMinSecondDuration)
    {
        double startAdjustment = (kMinSecondDuration - timeRange.getLength()) / 2.0;
        timeRange.setStart (timeRange.getStart() - startAdjustment);
        timeRange.setEnd (timeRange.getStart() + kMinSecondDuration);
    }

    // apply kMinBorderSeconds offset to start and end
    timeRange.setStart (timeRange.getStart() - kMinBorderSeconds);
    timeRange.setEnd (timeRange.getEnd() + kMinBorderSeconds);

    const int trackHeaderWidth = trackHeadersViewport.isVisible() ? trackHeadersViewport.getWidth() : 0;
    const int rulersViewHeight = rulersViewport.isVisible() ? 3*20 : 0;

    // max zoom 1px : 1sample (this is a naive assumption as audio can be in different sample rate)
    maxPixelsPerSecond = 192000.0; // TODO JUCE_ARA make configurabe from the outside, was: jmax (processor.getSampleRate(), 300.0);

    // min zoom covers entire view range
    // TODO JUCE_ARA getScrollBarThickness() should only be substracted if vertical scroll bar is actually visible
    minPixelsPerSecond = (getWidth() - trackHeaderWidth - playbackRegionsViewport.getScrollBarThickness()) / timeRange.getLength();

    // enforce zoom in/out limits
    const double validPixelsPerSecond = jlimit (minPixelsPerSecond, maxPixelsPerSecond, getPixelsPerSecond());
    const int playbackRegionsWidth = roundToInt (timeRange.getLength() * validPixelsPerSecond);
    setPixelsPerSecond (playbackRegionsWidth / timeRange.getLength());       // prevent potential rounding issues

    // update sizes and positions of all views
    playbackRegionsViewport.setBounds (trackHeaderWidth, rulersViewHeight, getWidth() - trackHeaderWidth, getHeight() - rulersViewHeight);
    playbackRegionsView.setBounds (0, 0, playbackRegionsWidth, jmax (getTrackHeight() * regionSequenceViews.size(), playbackRegionsViewport.getHeight() - playbackRegionsViewport.getScrollBarThickness()));

    rulersViewport.setBounds (trackHeaderWidth, 0, playbackRegionsViewport.getMaximumVisibleWidth(), rulersViewHeight);
    rulersView->setBounds (0, 0, playbackRegionsWidth, rulersViewHeight);

    trackHeadersViewport.setBounds (0, rulersViewHeight, trackHeadersViewport.getWidth(), playbackRegionsViewport.getMaximumVisibleHeight());
    trackHeadersView.setBounds (0, 0, trackHeadersViewport.getWidth(), playbackRegionsView.getHeight());

    int y = 0;
    const int trackHeight = getTrackHeight();
    for (auto v : regionSequenceViews)
    {
        v->setRegionsViewBoundsByYRange (y, trackHeight);
        y += trackHeight;
    }

    playHeadView.setBounds (playbackRegionsView.getBounds());

    // keep viewport position relative to playhead
    // TODO JUCE_ARA if playhead is not visible in new position, we should rather keep the
    //               left or right border stable, depending on which side the playhead is.
    auto relativeViewportPosition = playbackRegionsViewport.getViewPosition();
    relativeViewportPosition.setX (getPlaybackRegionsViewsXForTime (lastReportedPosition.timeInSeconds) - previousPlayHeadX);
    playbackRegionsViewport.setViewPosition (relativeViewportPosition);
    rulersViewport.setViewPosition (relativeViewportPosition.getX(), 0);
}

void DocumentView::rebuildRegionSequenceViews()
{
    // TODO JUCE_ARA always deleting the region sequence views and in turn their playback regions
    //               with their audio thumbs isn't particularly effective. we should optimized this
    //               and preserve all views that can still be used. We could also try to build some
    //               sort of LRU cache for the audio thumbs if that is easier...

    regionSequenceViews.clear();

    if (showOnlySelectedRegionSequences)
    {
        for (auto selectedSequence : getARAEditorView()->getViewSelection().getEffectiveRegionSequences<ARARegionSequence>())
            regionSequenceViews.add (createViewForRegionSequence (selectedSequence));
    }
    else    // show all RegionSequences of Document...
    {
        for (auto regionSequence : getARADocumentController()->getDocument()->getRegionSequences<ARARegionSequence>())
        {
            if (! ARA::contains (getARAEditorView()->getHiddenRegionSequences(), regionSequence))
                regionSequenceViews.add (createViewForRegionSequence(regionSequence));
        }
    }

    regionSequenceViewsAreInvalid = false;

    resized();
}

//==============================================================================
void DocumentView::onNewSelection (const ARA::PlugIn::ViewSelection& /*currentSelection*/)
{
    if (showOnlySelectedRegionSequences)
        invalidateRegionSequenceViews();
}

void DocumentView::onHideRegionSequences (std::vector<ARARegionSequence*> const& /*regionSequences*/)
{
    invalidateRegionSequenceViews();
}

void DocumentView::didEndEditing (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());
    
    if (regionSequenceViewsAreInvalid)
        rebuildRegionSequenceViews();
}

void DocumentView::didAddRegionSequenceToDocument (ARADocument* document, ARARegionSequence* regionSequence)
{
    jassert (document == getARADocumentController()->getDocument());

    invalidateRegionSequenceViews();
}

void DocumentView::didReorderRegionSequencesInDocument (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());

    invalidateRegionSequenceViews();
}

void DocumentView::timerCallback()
{
    if (lastReportedPosition.timeInSeconds != positionInfo.timeInSeconds)
    {
        lastReportedPosition.timeInSeconds = positionInfo.timeInSeconds;

        if (scrollFollowsPlayHead)
        {
            const auto visibleRange = getVisibleTimeRange();
            if (lastReportedPosition.timeInSeconds < visibleRange.getStart() || lastReportedPosition.timeInSeconds > visibleRange.getEnd())
                playbackRegionsViewport.setViewPosition (playbackRegionsViewport.getViewPosition().withX (getPlaybackRegionsViewsXForTime (lastReportedPosition.timeInSeconds)));
        };

        playHeadView.repaint();
    }
}

//==============================================================================
void DocumentView::addListener (Listener* const listener)
{
    listeners.add (listener);
}

void DocumentView::removeListener (Listener* const listener)
{
    listeners.remove (listener);
}

//==============================================================================
DocumentView::PlayHeadView::PlayHeadView (DocumentView& documentView)
    : documentView (documentView)
{}

void DocumentView::PlayHeadView::paint (juce::Graphics &g)
{
    const int playheadX = documentView.getPlaybackRegionsViewsXForTime (documentView.getPlayHeadPositionInfo().timeInSeconds);
    g.setColour (findColour (ScrollBar::ColourIds::thumbColourId));
    g.fillRect (playheadX, 0, 1, getHeight());
}

//==============================================================================
DocumentView::TrackHeadersViewport::TrackHeadersViewport (DocumentView &documentView)
    : documentView (documentView),
      resizeBorder (this, this, ResizableEdgeComponent::Edge::rightEdge)
{
    setSize (120, getHeight());
    setMinimumWidth (60);
    setMaximumWidth (240);
    addAndMakeVisible (resizeBorder);
}

void DocumentView::TrackHeadersViewport::setIsResizable (bool isResizable)
{
    resizeBorder.setVisible (isResizable);
}

void DocumentView::TrackHeadersViewport::resized()
{
    resizeBorder.setBounds (getWidth() - 1, 0, 1, getHeight());

    if (isShowing())
        documentView.resized();
}

//==============================================================================
// see https://forum.juce.com/t/viewport-scrollbarmoved-mousewheelmoved/20226
void DocumentView::ScrollMasterViewport::visibleAreaChanged (const Rectangle<int>& newVisibleArea)
{
    Viewport::visibleAreaChanged (newVisibleArea);
    
    documentView.getRulersViewport().setViewPosition (newVisibleArea.getX(), 0);
    documentView.getTrackHeadersViewport().setViewPosition (0, newVisibleArea.getY());
}
