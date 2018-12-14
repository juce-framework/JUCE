#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARASampleProjectDocumentController.h"
#include "RegionSequenceView.h"
#include "RulersView.h"

constexpr int kRulersViewHeight = 3*20;
constexpr int kTrackHeaderWidth = 120;
constexpr int kTrackHeight = 80;
constexpr int kStatusBarHeight = 20;
constexpr int kMinWidth = 3 * kTrackHeaderWidth;
constexpr int kWidth = 1000;
constexpr int kMinHeight = kRulersViewHeight + 1 * kTrackHeight + kStatusBarHeight;
constexpr int kHeight = kMinHeight + 5 * kTrackHeight;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p),
      playheadView (*this)
{
    setSize (kWidth, kHeight);
    setResizeLimits (kMinWidth, kMinHeight, 32768, 32768);
    setResizable (true, false);

    playheadView.setAlwaysOnTop (true);
    playbackRegionsView.addAndMakeVisible (playheadView);

    playbackRegionsViewPort.setScrollBarsShown (true, true, false, false);
    playbackRegionsViewPort.getHorizontalScrollBar().addListener (this);
    playbackRegionsViewPort.getVerticalScrollBar().addListener (this);
    playbackRegionsViewPort.setViewedComponent (&playbackRegionsView, false);
    addAndMakeVisible (playbackRegionsViewPort);

    trackHeadersViewPort.setScrollBarsShown (false, false, false, false);
    trackHeadersViewPort.setViewedComponent (&trackHeadersView, false);
    addAndMakeVisible (trackHeadersViewPort);

    zoomInButton.setButtonText("+");
    zoomOutButton.setButtonText("-");
    zoomInButton.onClick = [this]
    {
        storeRelativePosition();
        pixelsPerSecond *= 2.0;
        resized();
    };
    zoomOutButton.onClick = [this]
    {
        storeRelativePosition();
        pixelsPerSecond *= 0.5;
        resized();
    };
    addAndMakeVisible (zoomInButton);
    addAndMakeVisible (zoomOutButton);

    followPlayheadToggleButton.setButtonText ("Viewport follows playhead");
    addAndMakeVisible (followPlayheadToggleButton);

    if (isARAEditorView())
    {
        getARAEditorView()->addListener (this);

        auto document = static_cast<ARADocument*> (getARADocumentController()->getDocument());

        document->addListener (this);

        rulersView.reset (new RulersView (document));

        rulersViewPort.setScrollBarsShown (false, false, false, false);
        rulersViewPort.setViewedComponent (rulersView.get(), false);
        addAndMakeVisible (rulersViewPort);
    }

    rebuildRegionSequenceViews();
    startTimerHz (60);
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView())
    {
        static_cast<ARADocument*> (getARADocumentController()->getDocument())->removeListener (this);

        getARAEditorView()->removeListener (this);
    }

    playbackRegionsViewPort.getHorizontalScrollBar().removeListener (this);
    playbackRegionsViewPort.getVerticalScrollBar().removeListener (this);
}

//==============================================================================
int ARASampleProjectAudioProcessorEditor::getPlaybackRegionsViewsXForTime (double time) const
{
    return roundToInt ((time - startTime) / (endTime - startTime) * playbackRegionsView.getWidth());
}

double ARASampleProjectAudioProcessorEditor::getPlaybackRegionsViewsTimeForX (int x) const
{
    return ((double) x / (double) playbackRegionsView.getWidth()) * (endTime - startTime);
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    if (! isARAEditorView())
    {
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Non ARA Instance. Please re-open as ARA2!", getLocalBounds(), Justification::centred, 1);
    }
}

void ARASampleProjectAudioProcessorEditor::resized()
{
    // calculate visible time range
    if (regionSequenceViews.isEmpty())
    {
        startTime = 0.0;
        endTime = 0.0;
    }
    else
    {
        startTime = std::numeric_limits<double>::max();
        endTime = std::numeric_limits<double>::lowest();
        for (auto v : regionSequenceViews)
        {
            double sequenceStartTime, sequenceEndTime;
            v->getTimeRange (sequenceStartTime, sequenceEndTime);

            startTime = jmin (startTime, sequenceStartTime);
            endTime = jmax (endTime, sequenceEndTime);
        }
    }

    // make sure we can see at least 1 second
    constexpr double minDuration = 1.0;
    double duration = endTime - startTime;
    if (duration < minDuration)
    {
        startTime -= (minDuration - duration) / 2.0;
        endTime = startTime + minDuration;
    }

    // add a second left and right so that regions will not directly hit the end of the view
    constexpr double borderTime = 1.0;
    startTime -= borderTime;
    endTime += borderTime;

    // max zoom 1px : 1sample (this is a naive assumption as audio can be in different sample rate)
    maxPixelsPerSecond = jmax (processor.getSampleRate(), 300.0);

    // min zoom covers entire view range
    minPixelsPerSecond = (getWidth() - kTrackHeaderWidth) / (endTime - startTime);

    // enforce zoom in/out limits, update zoom buttons
    pixelsPerSecond =  jmax (minPixelsPerSecond, jmin (pixelsPerSecond, maxPixelsPerSecond));
    zoomOutButton.setEnabled (pixelsPerSecond > minPixelsPerSecond);
    zoomInButton.setEnabled (pixelsPerSecond < maxPixelsPerSecond);

    // update sizes and positions of all views
    playbackRegionsView.setBounds (0, 0, roundToInt ((endTime - startTime) * pixelsPerSecond), kTrackHeight * regionSequenceViews.size());
    playbackRegionsViewPort.setBounds (kTrackHeaderWidth, kRulersViewHeight, getWidth() - kTrackHeaderWidth, getHeight() - kRulersViewHeight - kStatusBarHeight);

    trackHeadersView.setBounds (0, 0, kTrackHeaderWidth, playbackRegionsView.getHeight());
    trackHeadersViewPort.setBounds (0, kRulersViewHeight, kTrackHeaderWidth, playbackRegionsViewPort.getMaximumVisibleHeight());

    if (rulersView != nullptr)
    {
        rulersView->setBounds (0, 0, playbackRegionsView.getWidth(), kRulersViewHeight);
        rulersViewPort.setBounds (kTrackHeaderWidth, 0, playbackRegionsViewPort.getMaximumVisibleWidth(), kRulersViewHeight);
    }

    int y = 0;
    for (auto v : regionSequenceViews)
    {
        v->setRegionsViewBoundsByYRange (y, kTrackHeight);
        y += kTrackHeight;
    }

    playheadView.setBounds (playbackRegionsView.getBounds());

    zoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
    zoomOutButton.setBounds (zoomInButton.getBounds().translated (-kStatusBarHeight, 0));
    followPlayheadToggleButton.setBounds (0, zoomInButton.getY(), 200, kStatusBarHeight);

    // keep viewport position relative to playhead
    const double newPixelBasedPositionInSeconds = pixelsUntilPlayhead / pixelsPerSecond;
    auto relativeViewportPosition = playbackRegionsViewPort.getViewPosition();
    relativeViewportPosition.setX (roundToInt ((playheadPositionInSeconds - newPixelBasedPositionInSeconds) * pixelsPerSecond));
    playbackRegionsViewPort.setViewPosition (relativeViewportPosition);
    rulersViewPort.setViewPosition (relativeViewportPosition.getX(), 0);
    trackHeadersViewPort.setViewPosition (0, relativeViewportPosition.getY());
}

void ARASampleProjectAudioProcessorEditor::scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    if (scrollBarThatHasMoved == &playbackRegionsViewPort.getHorizontalScrollBar())
        rulersViewPort.setViewPosition (roundToInt (newRangeStart), 0);
    else if (scrollBarThatHasMoved == &playbackRegionsViewPort.getVerticalScrollBar())
        trackHeadersViewPort.setViewPosition (0, roundToInt (newRangeStart));
    else
        jassertfalse;
}

void ARASampleProjectAudioProcessorEditor::rebuildRegionSequenceViews()
{
    regionSequenceViews.clear();

    for (auto regionSequence : getARADocumentController()->getDocument()->getRegionSequences())
    {
        if (! ARA::contains (getARAEditorView()->getHiddenRegionSequences(), regionSequence))
            regionSequenceViews.add (new RegionSequenceView (this, static_cast<ARARegionSequence*> (regionSequence)));
    }

    resized();
}

void ARASampleProjectAudioProcessorEditor::storeRelativePosition()
{
    pixelsUntilPlayhead = roundToInt (pixelsPerSecond * playheadPositionInSeconds - playbackRegionsViewPort.getViewPosition().getX());
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::onNewSelection (const ARA::PlugIn::ViewSelection& /*currentSelection*/)
{
// TODO JUCE_ARA the following was added as workaround for Logic, but it breaks navigating
//               in other hosts while zoomed in - disabled for now.
//    rebuildRegionSequenceViews();
}

void ARASampleProjectAudioProcessorEditor::onHideRegionSequences (std::vector<ARARegionSequence*> const& /*regionSequences*/)
{
    rebuildRegionSequenceViews();
}

void ARASampleProjectAudioProcessorEditor::didEndEditing (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());

    if (regionSequenceViewsAreInvalid)
    {
        rebuildRegionSequenceViews();
        regionSequenceViewsAreInvalid = false;
    }
}

void ARASampleProjectAudioProcessorEditor::didReorderRegionSequencesInDocument (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());

    invalidateRegionSequenceViews();
}

void ARASampleProjectAudioProcessorEditor::getVisibleTimeRange(double &start, double &end)
{
    start = getPlaybackRegionsViewsTimeForX (playbackRegionsViewPort.getViewArea().getX());
    end = getPlaybackRegionsViewsTimeForX (playbackRegionsViewPort.getViewArea().getRight());
}

void ARASampleProjectAudioProcessorEditor::timerCallback()
{
    auto position = static_cast<ARASampleProjectAudioProcessor*> (getAudioProcessor())->getLastKnownPositionInfo();
    if (playheadPositionInSeconds != position.timeInSeconds)
    {
        playheadPositionInSeconds = position.timeInSeconds;

        if (position.isPlaying && followPlayheadToggleButton.getToggleState())
        {
            double visibleStart, visibleEnd;
            getVisibleTimeRange (visibleStart, visibleEnd);
            if (playheadPositionInSeconds < visibleStart || playheadPositionInSeconds > visibleEnd)
                playbackRegionsViewPort.setViewPosition (playbackRegionsViewPort.getViewPosition().withX (getPlaybackRegionsViewsXForTime (playheadPositionInSeconds)));
        };

        playheadView.repaint();
    }
}

//==============================================================================
ARASampleProjectAudioProcessorEditor::PlayheadView::PlayheadView (ARASampleProjectAudioProcessorEditor &editorComponent)
    : editorComponent (editorComponent)
{}

void ARASampleProjectAudioProcessorEditor::PlayheadView::paint (juce::Graphics &g)
{
    int playheadX = editorComponent.getPlaybackRegionsViewsXForTime (editorComponent.getPlayheadPositionInSeconds());
    g.setColour (findColour (ScrollBar::ColourIds::thumbColourId));
    g.fillRect (playheadX - kPlayheadWidth / 2, 0, kPlayheadWidth, getHeight());
}
