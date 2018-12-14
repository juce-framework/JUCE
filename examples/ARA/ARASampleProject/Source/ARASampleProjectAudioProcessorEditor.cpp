#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARASampleProjectDocumentController.h"
#include "RegionSequenceView.h"

constexpr int kTrackHeaderWidth = 120;
constexpr int kTrackHeight = 80;
constexpr int kStatusBarHeight = 20;
constexpr int kMinWidth = 3 * kTrackHeaderWidth;
constexpr int kWidth = 1000;
constexpr int kMinHeight = 1 * kTrackHeight;
constexpr int kHeight = 5 * kTrackHeight + kStatusBarHeight;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p),
      playheadView (*this),
      araSampleProcessor (p)
{
    setSize (kWidth, kHeight);
    setResizeLimits (kMinWidth, kMinHeight, 32768, 32768);
    setResizable (true, false);

    playheadView.setAlwaysOnTop (true);
    playbackRegionsView.addAndMakeVisible (playheadView);

    playbackRegionsViewPort.setScrollBarsShown (true, true, false, false);
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

        static_cast<ARADocument*> (getARADocumentController()->getDocument())->addListener (this);
    }

    rebuildView();
    startTimerHz (60);
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView())
    {
        clearView();

        static_cast<ARADocument*> (getARADocumentController()->getDocument())->removeListener (this);

        getARAEditorView()->removeListener (this);
    }

    playbackRegionsViewPort.getVerticalScrollBar().removeListener (this);
}

void ARASampleProjectAudioProcessorEditor::storeRelativePosition()
{
    pixelsUntilPlayhead = roundToInt (pixelsPerSecond * playheadPositionInSeconds - playbackRegionsViewPort.getViewArea().getX());
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
    // max zoom 1px : 1sample (this is naive assumption as audio can be in different samplerate)
    maxPixelsPerSecond = jmax (processor.getSampleRate(), 300.0);

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

    // enforce zoom in/out limits
    minPixelsPerSecond = (playbackRegionsViewPort.getWidth() - playbackRegionsViewPort.getScrollBarThickness()) / (endTime - startTime);
    pixelsPerSecond =  jmax (minPixelsPerSecond, jmin (pixelsPerSecond, maxPixelsPerSecond));
    zoomOutButton.setEnabled (pixelsPerSecond > minPixelsPerSecond);
    zoomInButton.setEnabled (pixelsPerSecond < maxPixelsPerSecond);

    // set new bounds for all views associated with each region sequence
    int width = roundToInt ((endTime - startTime) * pixelsPerSecond);
    int y = 0;
    for (auto v : regionSequenceViews)
    {
        v->setRegionsViewBounds(kTrackHeaderWidth, y, width - trackHeadersViewPort.getScrollBarThickness(), kTrackHeight);
        y += kTrackHeight;
    }

    playbackRegionsView.setBounds (0, 0, width, y);
    playbackRegionsViewPort.setBounds (kTrackHeaderWidth, 0, getWidth() - kTrackHeaderWidth, getHeight() - kStatusBarHeight);

    playheadView.setBounds (playbackRegionsView.getBounds());

    trackHeadersView.setBounds (0, 0, kTrackHeaderWidth, y);
    trackHeadersViewPort.setBounds (0, 0, kTrackHeaderWidth, playbackRegionsViewPort.getHeight() - playbackRegionsViewPort.getScrollBarThickness());

    // keeps viewport position relative to playhead
    const auto newPixelBasedPositionInSeconds = pixelsUntilPlayhead / pixelsPerSecond;
    auto relativeViewportPosition = playbackRegionsViewPort.getViewPosition();
    relativeViewportPosition.setX (roundToInt ((playheadPositionInSeconds - newPixelBasedPositionInSeconds) * pixelsPerSecond));
    playbackRegionsViewPort.setViewPosition (relativeViewportPosition);
    trackHeadersViewPort.setViewPosition (0, relativeViewportPosition.getY());

    zoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
    zoomOutButton.setBounds (zoomInButton.getBounds().translated (-kStatusBarHeight, 0));
    followPlayheadToggleButton.setBounds (0, zoomInButton.getY(), 200, kStatusBarHeight);
}

void ARASampleProjectAudioProcessorEditor::scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    jassert (scrollBarThatHasMoved == &playbackRegionsViewPort.getVerticalScrollBar());
    trackHeadersViewPort.setViewPosition (0, roundToInt (newRangeStart));
}

void ARASampleProjectAudioProcessorEditor::rebuildView()
{
    clearView();

    for (auto regionSequence : getARADocumentController()->getDocument()->getRegionSequences())
    {
        if (ARA::contains (getARAEditorView()->getHiddenRegionSequences(), regionSequence))
            continue;

        auto sequenceView = new RegionSequenceView (this, static_cast<ARARegionSequence*> (regionSequence));
        regionSequenceViews.add (sequenceView);
    }

    // for demo purposes each rebuild resets zoom to show all document
    pixelsPerSecond = (endTime - startTime) / getWidth();

    resized();
}

void ARASampleProjectAudioProcessorEditor::clearView()
{
    regionSequenceViews.clear();
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::onNewSelection (const ARA::PlugIn::ViewSelection& /*currentSelection*/)
{
    rebuildView();
}

void ARASampleProjectAudioProcessorEditor::onHideRegionSequences (std::vector<ARARegionSequence*> const& /*regionSequences*/)
{
    rebuildView();
}

void ARASampleProjectAudioProcessorEditor::didEndEditing (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());

    if (isViewDirty)
    {
        rebuildView();
        isViewDirty = false;
    }
}

void ARASampleProjectAudioProcessorEditor::didReorderRegionSequencesInDocument (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());

    setDirty();
}

void ARASampleProjectAudioProcessorEditor::getVisibleTimeRange(double &start, double &end)
{
    start = playbackRegionsViewPort.getViewArea().getX() / pixelsPerSecond;
    end = playbackRegionsViewPort.getViewArea().getRight() / pixelsPerSecond;
}

//==============================================================================
ARASampleProjectAudioProcessorEditor::PlayheadView::PlayheadView(ARASampleProjectAudioProcessorEditor &owner)
    : owner(owner)
{}

void ARASampleProjectAudioProcessorEditor::PlayheadView::paint(juce::Graphics &g)
{
    int playheadX = roundToInt (owner.getPlayheadPositionInSeconds() * owner.getPixelsPerSeconds());
    g.setColour (findColour (ScrollBar::ColourIds::thumbColourId));
    g.fillRect(playheadX - kPlayheadWidth, 0, kPlayheadWidth, getHeight());
}

void ARASampleProjectAudioProcessorEditor::timerCallback()
{
    auto position = araSampleProcessor.getLastKnownPositionInfo();
    if (position.isPlaying)
    {
        playheadPositionInSeconds = position.timeInSeconds;
        if (followPlayheadToggleButton.getToggleState())
        {
            double visibleStart, visibleEnd;
            getVisibleTimeRange (visibleStart, visibleEnd);
            if (playheadPositionInSeconds < visibleStart || playheadPositionInSeconds > visibleEnd)
                playbackRegionsViewPort.setViewPosition(playbackRegionsViewPort.getViewPosition().withX (playheadPositionInSeconds * pixelsPerSecond));
        };
        playheadView.repaint();
    }
}
