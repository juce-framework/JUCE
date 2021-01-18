#include "DocumentView.h"

#include "RegionSequenceViewContainer.h"
#include "PlaybackRegionView.h"
#include "MusicalContextView.h"

using namespace juce;

constexpr int kTrackHeight { 80 };

static double lastPixelsPerSecond { 1.0 };

//==============================================================================
DocumentView::DocumentView (ARAEditorView* ev, const AudioPlayHead::CurrentPositionInfo& posInfo)
    : editorView (ev),
      playbackRegionsViewport (*this),
      playHeadView (*this),
      timeRangeSelectionView (*this),
      musicallContextView (*this),
      pixelsPerSecond (lastPixelsPerSecond),
      positionInfo (posInfo)
{
    calculateTimeRange();

    playHeadView.setAlwaysOnTop (true);
    playHeadView.setInterceptsMouseClicks (false, false);
    playbackRegionsView.addAndMakeVisible (playHeadView);
    timeRangeSelectionView.setAlwaysOnTop (true);
    timeRangeSelectionView.setInterceptsMouseClicks (false, false);
    playbackRegionsView.addAndMakeVisible (timeRangeSelectionView);

    playbackRegionsViewport.setScrollBarsShown (true, true, false, false);
    playbackRegionsViewport.setViewedComponent (&playbackRegionsView, false);
    addAndMakeVisible (playbackRegionsViewport);

    regionSequenceHeadersTooltipView.setAlwaysOnTop (true);
    regionSequenceHeadersView.addAndMakeVisible (regionSequenceHeadersTooltipView);
    regionSequenceHeadersViewport.setSize (120, getHeight());
    regionSequenceHeadersViewport.setScrollBarsShown (false, false, false, false);
    regionSequenceHeadersViewport.setViewedComponent (&regionSequenceHeadersView, false);
    addAndMakeVisible (regionSequenceHeadersViewport);

    musicalContextViewport.setScrollBarsShown (false, false, false, false);
    musicalContextViewport.setViewedComponent (&musicallContextView, false);
    addAndMakeVisible (musicalContextViewport);

    getARAEditorView()->addListener (this);
    getDocument()->addListener (this);

    lastReportedPosition.resetToDefault();

    startTimerHz (60);
}

DocumentView::~DocumentView()
{
    getDocument()->removeListener (this);
    getARAEditorView()->removeListener (this);
}

//==============================================================================
void DocumentView::onNewSelection (const ARAViewSelection& /*viewSelection*/)
{
    if (showOnlySelectedRegionSequences)
        invalidateRegionSequenceViewContainers();

    timeRangeSelectionView.repaint();
}

void DocumentView::onHideRegionSequences (std::vector<ARARegionSequence*> const& /*regionSequences*/)
{
    if (! showOnlySelectedRegionSequences)
        invalidateRegionSequenceViewContainers();
}

void DocumentView::didEndEditing (ARADocument* /*document*/)
{
    if (regionSequenceViewsAreInvalid)
        rebuildRegionSequenceViewContainers();

    if (timeRangeIsInvalid)
        calculateTimeRange();
}

void DocumentView::didReorderRegionSequencesInDocument (ARADocument* /*document*/)
{
    invalidateRegionSequenceViewContainers();
}

//==============================================================================
void DocumentView::invalidateTimeRange()
{
    timeRangeIsInvalid = true;
}

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

//==============================================================================
void DocumentView::setShowOnlySelectedRegionSequences (bool newVal)
{
    showOnlySelectedRegionSequences = newVal;
    invalidateRegionSequenceViewContainers();
}

void DocumentView::zoomBy (double factor)
{
    pixelsPerSecond *= factor;
    if (getParentComponent() != nullptr)
        resized();  // this will both constrain pixelsPerSecond range properly and update all views
}

//==============================================================================
void DocumentView::parentHierarchyChanged()
{
    // trigger initial update lazily after construction
    if (regionSequenceViewsAreInvalid && ! getDocumentController()->isHostEditingDocument())
        rebuildRegionSequenceViewContainers();
}

void DocumentView::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void DocumentView::resized()
{
    // store visible playhead postion (in main view coordinates)
    int previousPlayHeadX = getPlaybackRegionsViewsXForTime (lastReportedPosition.timeInSeconds) - playbackRegionsViewport.getViewPosition().getX();

    const int regionSequenceHeaderWidth = regionSequenceHeadersViewport.getWidth();
    const int musicalContextViewHeight = musicalContextViewport.isVisible() ? 3*20 : 0;

    // update zoom
    double playbackRegionsViewWidthDbl = timeRange.getLength() * pixelsPerSecond;

    // limit max zoom by roughly 2 pixel per sample (we're just assuming some arbitrary high sample rate here),
    // but we also must make sure playbackRegionsViewWidth does not exceed integer range (with additional safety margin for rounding)
    playbackRegionsViewWidthDbl = jmin (playbackRegionsViewWidthDbl, timeRange.getLength() * 2.0 * 192000.0);
    playbackRegionsViewWidthDbl = jmin (playbackRegionsViewWidthDbl, static_cast<double> (std::numeric_limits<int>::max() - 1));
    int playbackRegionsViewWidth = roundToInt (floor (playbackRegionsViewWidthDbl));

    // min zoom is limited by covering entire view range
    // TODO JUCE_ARA getScrollBarThickness() should only be substracted if vertical scroll bar is actually visible
    const int minPlaybackRegionsViewWidth = getWidth() - regionSequenceHeaderWidth - playbackRegionsViewport.getScrollBarThickness();
    playbackRegionsViewWidth = jmax (minPlaybackRegionsViewWidth, playbackRegionsViewWidth);
    pixelsPerSecond = playbackRegionsViewWidth / timeRange.getLength();
    lastPixelsPerSecond = pixelsPerSecond;

    // update sizes and positions of all views
    playbackRegionsViewport.setBounds (regionSequenceHeaderWidth, musicalContextViewHeight, getWidth() - regionSequenceHeaderWidth, getHeight() - musicalContextViewHeight);
    playbackRegionsView.setBounds (0, 0, playbackRegionsViewWidth, jmax (kTrackHeight * regionSequenceViewContainers.size(), playbackRegionsViewport.getHeight() - playbackRegionsViewport.getScrollBarThickness()));

    musicalContextViewport.setBounds (regionSequenceHeaderWidth, 0, playbackRegionsViewport.getMaximumVisibleWidth(), musicalContextViewHeight);
    musicallContextView.setBounds (0, 0, playbackRegionsViewWidth, musicalContextViewHeight);

    regionSequenceHeadersViewport.setBounds (0, musicalContextViewHeight, regionSequenceHeadersViewport.getWidth(), playbackRegionsViewport.getMaximumVisibleHeight());
    regionSequenceHeadersView.setBounds (0, 0, regionSequenceHeadersViewport.getWidth(), playbackRegionsView.getHeight());
    regionSequenceHeadersTooltipView.setBounds (regionSequenceHeadersView.getBounds());

    int y = 0;
    for (auto v : regionSequenceViewContainers)
    {
        v->setRegionsViewBoundsByYRange (y, kTrackHeight);
        y += kTrackHeight;
    }

    playHeadView.setBounds (playbackRegionsView.getBounds());
    timeRangeSelectionView.setBounds (playbackRegionsView.getBounds());

    // keep viewport position relative to playhead
    // TODO JUCE_ARA if playhead is not visible in new position, we should rather keep the
    //               left or right border stable, depending on which side the playhead is.
    auto relativeViewportPosition = playbackRegionsViewport.getViewPosition();
    relativeViewportPosition.setX (getPlaybackRegionsViewsXForTime (lastReportedPosition.timeInSeconds) - previousPlayHeadX);
    playbackRegionsViewport.setViewPosition (relativeViewportPosition);
    musicalContextViewport.setViewPosition (relativeViewportPosition.getX(), 0);
}

//==============================================================================
void DocumentView::timerCallback()
{
    if (lastReportedPosition.timeInSeconds != positionInfo.timeInSeconds)
    {
        lastReportedPosition = positionInfo;

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
void DocumentView::invalidateRegionSequenceViewContainers()
{
    if (getDocumentController()->isHostEditingDocument() || getParentComponent() == nullptr)
        regionSequenceViewsAreInvalid = true;
    else
        rebuildRegionSequenceViewContainers();
}

void DocumentView::rebuildRegionSequenceViewContainers()
{
    // always deleting all region sequence views and in turn their playback regions including their
    // audio thumbs isn't particularly effective - in an actual plug-in this would need to be optimized.
    regionSequenceViewContainers.clear();

    if (showOnlySelectedRegionSequences)
    {
        for (auto selectedSequence : getARAEditorView()->getViewSelection().getEffectiveRegionSequences<ARARegionSequence>())
            regionSequenceViewContainers.add (new RegionSequenceViewContainer (*this, selectedSequence));
    }
    else    // show all RegionSequences of Document...
    {
        for (auto regionSequence : getDocument()->getRegionSequences<ARARegionSequence>())
            if (! ARA::contains (getARAEditorView()->getHiddenRegionSequences(), regionSequence))
                regionSequenceViewContainers.add (new RegionSequenceViewContainer (*this, regionSequence));
    }

    calculateTimeRange();

    regionSequenceViewsAreInvalid = false;
    resized();

    // update region header tooltips
    const auto total = getDocument()->getRegionSequences<ARARegionSequence>().size();
    const auto hidden = getARAEditorView()->getHiddenRegionSequences().size();
    String s ("Showing " + String (regionSequenceViewContainers.size()));
    if (showOnlySelectedRegionSequences)
        s += " selected";
    s += String (" out of ") + String (total - hidden) + String (" tracks");
    if (hidden)
        s += String ("(") + String (hidden) + String (" hidden)");
    s+= String (".");
    regionSequenceHeadersTooltipView.setTooltip(s);
}

void DocumentView::calculateTimeRange()
{
    Range<double> newTimeRange;
    if (! regionSequenceViewContainers.isEmpty())
    {
        bool isFirst = true;
        for (auto v : regionSequenceViewContainers)
        {
            if (v->isEmpty())
                continue;

            const auto sequenceTimeRange = v->getTimeRange();
            if (isFirst)
            {
                newTimeRange = sequenceTimeRange;
                isFirst = false;
                continue;
            }

            newTimeRange = newTimeRange.getUnionWith (sequenceTimeRange);
        }
    }

    newTimeRange = newTimeRange.expanded (1.0);   // add a 1 second border left and right of first/last region

    timeRangeIsInvalid = false;
    if (timeRange != newTimeRange)
    {
        timeRange = newTimeRange;
        if (getParentComponent() != nullptr)
            resized();
    }
}

//==============================================================================
DocumentView::PlayHeadView::PlayHeadView (DocumentView& docView)
    : documentView (docView)
{}

void DocumentView::PlayHeadView::paint (juce::Graphics &g)
{
    const int playheadX = documentView.getPlaybackRegionsViewsXForTime (documentView.getPlayHeadPositionInfo().timeInSeconds);
    g.setColour (findColour (ScrollBar::ColourIds::thumbColourId));
    g.fillRect (playheadX, 0, 1, getHeight());
}

//==============================================================================
DocumentView::TimeRangeSelectionView::TimeRangeSelectionView (DocumentView& docView)
    : documentView (docView)
{}

void DocumentView::TimeRangeSelectionView::paint (juce::Graphics& g)
{
    const auto selection = documentView.getARAEditorView()->getViewSelection();
    if (selection.getTimeRange() != nullptr && selection.getTimeRange()->duration > 0.0)
    {
        const int startPixel = documentView.getPlaybackRegionsViewsXForTime (selection.getTimeRange()->start);
        const int endPixel = documentView.getPlaybackRegionsViewsXForTime (selection.getTimeRange()->start + selection.getTimeRange()->duration);
        g.setColour (juce::Colours::yellow.withAlpha (0.2f));
        g.fillRect (startPixel, 0, endPixel - startPixel, getHeight());
    }
}

//==============================================================================
// see https://forum.juce.com/t/viewport-scrollbarmoved-mousewheelmoved/20226
void DocumentView::ScrollMasterViewport::visibleAreaChanged (const Rectangle<int>& newVisibleArea)
{
    Viewport::visibleAreaChanged (newVisibleArea);
    
    documentView.getMusicalContextViewport().setViewPosition (newVisibleArea.getX(), 0);
    documentView.getRegionSequenceHeadersViewport().setViewPosition (0, newVisibleArea.getY());
}
