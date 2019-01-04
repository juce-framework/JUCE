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
constexpr double kMinSecondDuration = 1.0;
constexpr double kMinBorderSeconds = 1.0;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p),
      playbackRegionsViewPort (*this),
      playheadView (*this),
      visibleRange (-kMinBorderSeconds, kMinSecondDuration + kMinBorderSeconds)
{
    // TODO JUCE_ARA hotfix for Unicode chord symbols, see https://forum.juce.com/t/embedding-unicode-string-literals-in-your-cpp-files/12600/7
    getLookAndFeel().setDefaultSansSerifTypefaceName("Arial Unicode MS");

    setSize (kWidth, kHeight);
    setResizeLimits (kMinWidth, kMinHeight, 32768, 32768);
    setResizable (true, false);

    playheadView.setAlwaysOnTop (true);
    playbackRegionsView.addAndMakeVisible (playheadView);

    playbackRegionsViewPort.setScrollBarsShown (true, true, false, false);
    playbackRegionsViewPort.setViewedComponent (&playbackRegionsView, false);
    addAndMakeVisible (playbackRegionsViewPort);

    trackHeadersViewPort.setScrollBarsShown (false, false, false, false);
    trackHeadersViewPort.setViewedComponent (&trackHeadersView, false);
    addAndMakeVisible (trackHeadersViewPort);

    zoomInButton.setButtonText("+");
    zoomOutButton.setButtonText("-");
    constexpr double zoomStepFactor = 1.5;
    zoomInButton.onClick = [this, zoomStepFactor]
    {
        pixelsPerSecond *= zoomStepFactor;
        resized();
    };
    zoomOutButton.onClick = [this, zoomStepFactor]
    {
        pixelsPerSecond /= zoomStepFactor;
        resized();
    };
    addAndMakeVisible (zoomInButton);
    addAndMakeVisible (zoomOutButton);

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

    rebuildRegionSequenceViews();
    startTimerHz (60);
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView())
    {
        getARADocumentController()->getDocument<ARADocument>()->removeListener (this);
        getARAEditorView()->removeListener (this);
    }
}

//==============================================================================
int ARASampleProjectAudioProcessorEditor::getPlaybackRegionsViewsXForTime (double time) const
{
    return roundToInt ((time - visibleRange.getStart()) / visibleRange.getLength() * playbackRegionsView.getWidth());
}

double ARASampleProjectAudioProcessorEditor::getPlaybackRegionsViewsTimeForX (int x) const
{
    return visibleRange.getStart() + ((double) x / (double) playbackRegionsView.getWidth()) * visibleRange.getLength();
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
    zoomOutButton.setEnabled (pixelsPerSecond > minPixelsPerSecond);
    zoomInButton.setEnabled (pixelsPerSecond < maxPixelsPerSecond);

    // update sizes and positions of all views
    playbackRegionsViewPort.setBounds (kTrackHeaderWidth, kRulersViewHeight, getWidth() - kTrackHeaderWidth, getHeight() - kRulersViewHeight - kStatusBarHeight);
    playbackRegionsView.setBounds (0, 0, roundToInt (visibleRange.getLength() * pixelsPerSecond), jmax (kTrackHeight * regionSequenceViews.size(), playbackRegionsViewPort.getHeight() - playbackRegionsViewPort.getScrollBarThickness()));
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
        v->setRegionsViewBoundsByYRange (y, kTrackHeight);
        y += kTrackHeight;
    }

    playheadView.setBounds (playbackRegionsView.getBounds());

    zoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
    zoomOutButton.setBounds (zoomInButton.getBounds().translated (-kStatusBarHeight, 0));
    followPlayheadToggleButton.setBounds (0, zoomInButton.getY(), 200, kStatusBarHeight);

    // keep viewport position relative to playhead
    // TODO JUCE_ARA if playhead is not visible in new position, we should rather keep the
    //               left or right border stable, depending on which side the playhead is.
    auto relativeViewportPosition = playbackRegionsViewPort.getViewPosition();
    relativeViewportPosition.setX (getPlaybackRegionsViewsXForTime (playheadTimePosition) - previousPlayheadX);
    playbackRegionsViewPort.setViewPosition (relativeViewportPosition);
    rulersViewPort.setViewPosition (relativeViewportPosition.getX(), 0);
}

void ARASampleProjectAudioProcessorEditor::rebuildRegionSequenceViews()
{
    regionSequenceViews.clear();

    for (auto regionSequence : getARADocumentController()->getDocument()->getRegionSequences<ARARegionSequence>())
    {
        if (! ARA::contains (getARAEditorView()->getHiddenRegionSequences(), regionSequence))
            regionSequenceViews.add (new RegionSequenceView (this, regionSequence));
    }
    regionSequenceViewsAreInvalid = false;

    resized();
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::onHideRegionSequences (std::vector<ARARegionSequence*> const& /*regionSequences*/)
{
    rebuildRegionSequenceViews();
}

void ARASampleProjectAudioProcessorEditor::didEndEditing (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());

    if (regionSequenceViewsAreInvalid)
        rebuildRegionSequenceViews();
}

void ARASampleProjectAudioProcessorEditor::didReorderRegionSequencesInDocument (ARADocument* document)
{
    jassert (document == getARADocumentController()->getDocument());

    invalidateRegionSequenceViews();
}

Range<double> ARASampleProjectAudioProcessorEditor::getVisibleTimeRange() const
{
    const double start = getPlaybackRegionsViewsTimeForX (playbackRegionsViewPort.getViewArea().getX());
    const double end = getPlaybackRegionsViewsTimeForX (playbackRegionsViewPort.getViewArea().getRight());
    return { start, end };
}

void ARASampleProjectAudioProcessorEditor::timerCallback()
{
    auto position = static_cast<ARASampleProjectAudioProcessor*> (getAudioProcessor())->getLastKnownPositionInfo();
    if (playheadTimePosition != position.timeInSeconds)
    {
        playheadTimePosition = position.timeInSeconds;

        if (followPlayheadToggleButton.getToggleState())
        {
            if (playheadTimePosition < visibleRange.getStart() || playheadTimePosition > visibleRange.getEnd())
                playbackRegionsViewPort.setViewPosition (playbackRegionsViewPort.getViewPosition().withX (getPlaybackRegionsViewsXForTime (playheadTimePosition)));
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
    static constexpr int kPlayheadWidth = 1;
    const int playheadX = editorComponent.getPlaybackRegionsViewsXForTime (editorComponent.getPlayheadTimePosition());
    g.setColour (findColour (ScrollBar::ColourIds::thumbColourId));
    g.fillRect (playheadX - kPlayheadWidth / 2, 0, kPlayheadWidth, getHeight());
}

//==============================================================================
// see https://forum.juce.com/t/viewport-scrollbarmoved-mousewheelmoved/20226
void ARASampleProjectAudioProcessorEditor::ScrollMasterViewPort::visibleAreaChanged (const Rectangle<int>& newVisibleArea)
{
    Viewport::visibleAreaChanged (newVisibleArea);

    editorComponent.getRulersViewPort().setViewPosition (newVisibleArea.getX(), 0);
    editorComponent.getTrackHeadersViewPort().setViewPosition (0, newVisibleArea.getY());
}
