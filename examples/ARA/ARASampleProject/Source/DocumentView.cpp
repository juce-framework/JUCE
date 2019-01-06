#include "DocumentView.h"

#include "RegionSequenceView.h"
#include "TrackHeaderView.h"
#include "PlaybackRegionView.h"
#include "RulersView.h"

constexpr int kRulersViewHeight = 3*20;
constexpr int kTrackHeaderWidth = 120;
constexpr int kStatusBarHeight = 20;
constexpr double kMinSecondDuration = 1.0;
constexpr double kMinBorderSeconds = 1.0;

//==============================================================================
DocumentView::DocumentView (AudioProcessor& p)
: AudioProcessorEditor (&p),
AudioProcessorEditorARAExtension (&p),
playbackRegionsViewPort (*this),
playheadView (*this),
visibleRange (-kMinBorderSeconds, kMinSecondDuration + kMinBorderSeconds),
positionInfoPtr (nullptr)
{
    playheadView.setAlwaysOnTop (true);
    playbackRegionsView.addAndMakeVisible (playheadView);
    
    playbackRegionsViewPort.setScrollBarsShown (true, true, false, false);
    playbackRegionsViewPort.setViewedComponent (&playbackRegionsView, false);
    addAndMakeVisible (playbackRegionsViewPort);
    
    trackHeadersViewPort.setScrollBarsShown (false, false, false, false);
    trackHeadersViewPort.setViewedComponent (&trackHeadersView, false);
    addAndMakeVisible (trackHeadersViewPort);

    // TODO: Zoom components should be decided later
    //       I guess they should be customizable so we might
    //       just provide methods so they could be called by methods
    //       by developers own views.
    horizontalZoomLabel.setText ("H:", dontSendNotification);
    verticalZoomLabel.setText ("V:", dontSendNotification);
    verticalZoomInButton.setButtonText("+");
    verticalZoomOutButton.setButtonText("-");
    horizontalZoomInButton.setButtonText("+");
    horizontalZoomOutButton.setButtonText("-");
    constexpr double zoomStepFactor = 1.5;
    horizontalZoomInButton.onClick = [this, zoomStepFactor]
    {
        pixelsPerSecond *= zoomStepFactor;
        resized();
    };
    horizontalZoomOutButton.onClick = [this, zoomStepFactor]
    {
        pixelsPerSecond /= zoomStepFactor;
        resized();
    };
    verticalZoomInButton.onClick = [this, zoomStepFactor]
    {
        trackHeight *= zoomStepFactor;
        resized();
    };
    verticalZoomOutButton.onClick = [this, zoomStepFactor]
    {
        trackHeight /= zoomStepFactor;
        resized();
    };

    addAndMakeVisible (horizontalZoomLabel);
    addAndMakeVisible (verticalZoomLabel);
    addAndMakeVisible (verticalZoomInButton);
    addAndMakeVisible (verticalZoomOutButton);
    addAndMakeVisible (horizontalZoomInButton);
    addAndMakeVisible (horizontalZoomOutButton);
    
    followPlayheadToggleButton.setButtonText ("Viewport follows playhead");
    followPlayheadToggleButton.setToggleState (true, dontSendNotification);
    addAndMakeVisible (followPlayheadToggleButton);
    
    if (isARAEditorView())
    {
        getARAEditorView()->addListener (this);
        getARADocumentController()->getDocument<ARADocument>()->addListener (this);
        
        rulersView.reset (new RulersView (*this));
        rulersViewPort.setScrollBarsShown (false, false, false, false);
        rulersViewPort.setViewedComponent (rulersView.get(), false);
        addAndMakeVisible (rulersViewPort);
    }
    startTimerHz (60);
}

DocumentView::~DocumentView()
{
    if (isARAEditorView())
    {
        getARADocumentController()->getDocument<ARADocument>()->removeListener (this);
        getARAEditorView()->removeListener (this);
    }
}

//==============================================================================
int DocumentView::getPlaybackRegionsViewsXForTime (double time) const
{
    return roundToInt ((time - visibleRange.getStart()) / visibleRange.getLength() * playbackRegionsView.getWidth());
}

double DocumentView::getPlaybackRegionsViewsTimeForX (int x) const
{
    return visibleRange.getStart() + ((double) x / (double) playbackRegionsView.getWidth()) * visibleRange.getLength();
}

//==============================================================================
void DocumentView::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    if (! isARAEditorView())
    {
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Non ARA Instance. Please re-open as ARA2!", getLocalBounds(), Justification::centred, 1);
    }
    else
    {
        if (regionSequenceViewsAreInvalid)
        {
            rebuildRegionSequenceViews();
            regionSequenceViewsAreInvalid = false;
        }
    }
}

void DocumentView::resized()
{
    // store visible playhead postion (in main view coordinates)
    int previousPlayheadX = getPlaybackRegionsViewsXForTime (playheadTimePosition) - playbackRegionsViewPort.getViewPosition().getX();

    // calculate maximum visible time range
    visibleRange = { 0.0, 0.0 };
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
                visibleRange = sequenceTimeRange;
                isFirst = false;
                continue;
            }

            visibleRange = visibleRange.getUnionWith (sequenceTimeRange);
        }
    }

    // ensure visible range covers kMinSecondDuration
    if (visibleRange.getLength() < kMinSecondDuration)
    {
        double startAdjustment = (kMinSecondDuration - visibleRange.getLength()) / 2.0;
        visibleRange.setStart (visibleRange.getStart() - startAdjustment);
        visibleRange.setEnd (visibleRange.getStart() + kMinSecondDuration);
    }

    // apply kMinBorderSeconds offset to start and end
    visibleRange.setStart (visibleRange.getStart() - kMinBorderSeconds);
    visibleRange.setEnd (visibleRange.getEnd() + kMinBorderSeconds);

    // max zoom 1px : 1sample (this is a naive assumption as audio can be in different sample rate)
    double maxPixelsPerSecond = jmax (processor.getSampleRate(), 300.0);

    // min zoom covers entire view range
    double minPixelsPerSecond = (getWidth() - kTrackHeaderWidth - rulersViewPort.getScrollBarThickness()) / visibleRange.getLength();

    // enforce zoom in/out limits, update zoom buttons
    pixelsPerSecond = jmax (minPixelsPerSecond, jmin (pixelsPerSecond, maxPixelsPerSecond));
    horizontalZoomOutButton.setEnabled (pixelsPerSecond > minPixelsPerSecond);
    horizontalZoomInButton.setEnabled (pixelsPerSecond < maxPixelsPerSecond);

    // update sizes and positions of all views
    playbackRegionsViewPort.setBounds (kTrackHeaderWidth, kRulersViewHeight, getWidth() - kTrackHeaderWidth, getHeight() - kRulersViewHeight - kStatusBarHeight);
    playbackRegionsView.setBounds (0, 0, roundToInt (visibleRange.getLength() * pixelsPerSecond), jmax (trackHeight * regionSequenceViews.size(), playbackRegionsViewPort.getHeight() - playbackRegionsViewPort.getScrollBarThickness()));
    pixelsPerSecond = playbackRegionsView.getWidth() / visibleRange.getLength();       // prevent potential rounding issues

    trackHeadersViewPort.setBounds (0, kRulersViewHeight, kTrackHeaderWidth, playbackRegionsViewPort.getMaximumVisibleHeight());
    trackHeadersView.setBounds (0, 0, kTrackHeaderWidth, playbackRegionsView.getHeight());

    if (rulersView != nullptr)
    {
        rulersViewPort.setBounds (kTrackHeaderWidth, 0, playbackRegionsViewPort.getMaximumVisibleWidth(), kRulersViewHeight);
        rulersView->setBounds (0, 0, playbackRegionsView.getWidth(), kRulersViewHeight);
    }

    int y = 0;
    for (auto v : regionSequenceViews)
    {
        v->setRegionsViewBoundsByYRange (y, trackHeight);
        y += trackHeight;
    }

    playheadView.setBounds (playbackRegionsView.getBounds());

    horizontalZoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
    horizontalZoomOutButton.setBounds (horizontalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
    horizontalZoomLabel.setBounds (horizontalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
    verticalZoomInButton.setBounds (horizontalZoomLabel.getBounds().translated (-kStatusBarHeight, 0));
    verticalZoomOutButton.setBounds (verticalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
    verticalZoomLabel.setBounds (verticalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
    followPlayheadToggleButton.setBounds (0, horizontalZoomInButton.getY(), 200, kStatusBarHeight);

    // keep viewport position relative to playhead
    // TODO JUCE_ARA if playhead is not visible in new position, we should rather keep the
    //               left or right border stable, depending on which side the playhead is.
    auto relativeViewportPosition = playbackRegionsViewPort.getViewPosition();
    relativeViewportPosition.setX (getPlaybackRegionsViewsXForTime (playheadTimePosition) - previousPlayheadX);
    playbackRegionsViewPort.setViewPosition (relativeViewportPosition);
    rulersViewPort.setViewPosition (relativeViewportPosition.getX(), 0);
}

void DocumentView::rebuildRegionSequenceViews()
{
    regionSequenceViews.clear();

    for (auto regionSequence : getARADocumentController()->getDocument()->getRegionSequences<ARARegionSequence>())
    {
        if (!showOnlySelectedRegionSequence && ! ARA::contains (getARAEditorView()->getHiddenRegionSequences(), regionSequence))
        {
            regionSequenceViews.add (getViewForRegionSequence(regionSequence));
        }
        else
        {
            auto selectedSequences = getARAEditorView()->getViewSelection().getRegionSequences();
            if (ARA::contains (selectedSequences, regionSequence))
            {
                regionSequenceViews.add (getViewForRegionSequence(regionSequence));
            }
        }
    }
    
    resized();
}

//==============================================================================
void DocumentView::onHideRegionSequences (std::vector<ARARegionSequence*> const& /*regionSequences*/)
{
    rebuildRegionSequenceViews();
}

void DocumentView::didEndEditing (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());
    
    if (regionSequenceViewsAreInvalid)
    {
        rebuildRegionSequenceViews();
        regionSequenceViewsAreInvalid = false;
    }
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

PlaybackRegionView* DocumentView::getViewForPlaybackRegion (ARAPlaybackRegion* playbackRegion)
{
    return new PlaybackRegionView (*this, playbackRegion);
}

TrackHeaderView* DocumentView::getHeaderViewForRegionSequence (ARARegionSequence* regionSequence)
{
    return new TrackHeaderView (getARAEditorView(), regionSequence);
}

RegionSequenceView* DocumentView::getViewForRegionSequence (ARARegionSequence* regionSequence)
{
    return new RegionSequenceView (*this, regionSequence);
}

Range<double> DocumentView::getVisibleTimeRange() const
{
    double start = getPlaybackRegionsViewsTimeForX (playbackRegionsViewPort.getViewArea().getX());
    double end = getPlaybackRegionsViewsTimeForX (playbackRegionsViewPort.getViewArea().getRight());
    return Range<double> (start, end);
}

void DocumentView::timerCallback()
{
    if (positionInfoPtr == nullptr)
        return;
        
    if (playheadTimePosition != positionInfoPtr->timeInSeconds)
    {
        playheadTimePosition = positionInfoPtr->timeInSeconds;

        if (followPlayheadToggleButton.getToggleState())
        {
            Range<double> visibleRange = getVisibleTimeRange();
            if (playheadTimePosition < visibleRange.getStart() || playheadTimePosition > visibleRange.getEnd())
                playbackRegionsViewPort.setViewPosition (playbackRegionsViewPort.getViewPosition().withX (getPlaybackRegionsViewsXForTime (playheadTimePosition)));
        };

        playheadView.repaint();
    }
}

void DocumentView::setCurrentPositionInfo (const AudioPlayHead::CurrentPositionInfo* curPosInfoPtr)
{
    positionInfoPtr = curPosInfoPtr;
}

//==============================================================================
DocumentView::PlayheadView::PlayheadView (DocumentView& documentView)
: documentView (documentView)
{}

void DocumentView::PlayheadView::paint (juce::Graphics &g)
{
    static constexpr int kPlayheadWidth = 1;
    const int playheadX = documentView.getPlaybackRegionsViewsXForTime (documentView.getPlayheadTimePosition());
    g.setColour (findColour (ScrollBar::ColourIds::thumbColourId));
    g.fillRect (playheadX - kPlayheadWidth / 2, 0, kPlayheadWidth, getHeight());
}

//==============================================================================
// see https://forum.juce.com/t/viewport-scrollbarmoved-mousewheelmoved/20226
void DocumentView::ScrollMasterViewPort::visibleAreaChanged (const Rectangle<int>& newVisibleArea)
{
    Viewport::visibleAreaChanged (newVisibleArea);
    
    documentView.getRulersViewPort().setViewPosition (newVisibleArea.getX(), 0);
    documentView.getTrackHeadersViewPort().setViewPosition (0, newVisibleArea.getY());
}
