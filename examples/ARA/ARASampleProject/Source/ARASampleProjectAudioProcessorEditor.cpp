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

    playbackRegionsViewPort.getHorizontalScrollBar().removeListener (this);
    playbackRegionsViewPort.getVerticalScrollBar().removeListener (this);
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
    playbackRegionsViewPort.setBounds (kTrackHeaderWidth, kRulersViewHeight, getWidth() - kTrackHeaderWidth, getHeight() - kRulersViewHeight - kStatusBarHeight);

    playheadView.setBounds (playbackRegionsView.getBounds());

    if (rulersView != nullptr)
    {
        rulersView->setBounds (0, 0, width, kRulersViewHeight);
        rulersViewPort.setBounds (kTrackHeaderWidth, 0, playbackRegionsViewPort.getWidth() - playbackRegionsViewPort.getScrollBarThickness(), kRulersViewHeight);
    }

    trackHeadersView.setBounds (0, 0, kTrackHeaderWidth, y);
    trackHeadersViewPort.setBounds (0, kRulersViewHeight, kTrackHeaderWidth, playbackRegionsViewPort.getHeight() - playbackRegionsViewPort.getScrollBarThickness());

    // keeps viewport position relative to playhead
    const auto newPixelBasedPositionInSeconds = pixelsUntilPlayhead / pixelsPerSecond;
    auto relativeViewportPosition = playbackRegionsViewPort.getViewPosition();
    relativeViewportPosition.setX (roundToInt ((playheadPositionInSeconds - newPixelBasedPositionInSeconds) * pixelsPerSecond));
    playbackRegionsViewPort.setViewPosition (relativeViewportPosition);
    rulersViewPort.setViewPosition (relativeViewportPosition.getX(), 0);
    trackHeadersViewPort.setViewPosition (0, relativeViewportPosition.getY());

    zoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
    zoomOutButton.setBounds (zoomInButton.getBounds().translated (-kStatusBarHeight, 0));
    followPlayheadToggleButton.setBounds (0, zoomInButton.getY(), 200, kStatusBarHeight);
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

void ARASampleProjectAudioProcessorEditor::storeRelativePosition()
{
    pixelsUntilPlayhead = roundToInt (pixelsPerSecond * playheadPositionInSeconds - playbackRegionsViewPort.getViewArea().getX());
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
    auto position = static_cast<ARASampleProjectAudioProcessor*> (getAudioProcessor())->getLastKnownPositionInfo();
    if (playheadPositionInSeconds != position.timeInSeconds)
    {
        playheadPositionInSeconds = position.timeInSeconds;

        if (position.isPlaying && followPlayheadToggleButton.getToggleState())
        {
            double visibleStart, visibleEnd;
            getVisibleTimeRange (visibleStart, visibleEnd);
            if (playheadPositionInSeconds < visibleStart || playheadPositionInSeconds > visibleEnd)
                playbackRegionsViewPort.setViewPosition(playbackRegionsViewPort.getViewPosition().withX (playheadPositionInSeconds * pixelsPerSecond));
        };

        playheadView.repaint();
    }
}
