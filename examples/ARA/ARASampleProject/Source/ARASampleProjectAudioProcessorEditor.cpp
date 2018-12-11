#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARASampleProjectDocumentController.h"

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p),
      horizontalScrollBar (false)
{
    tracksViewPort.setScrollBarsShown (false, false, false, false);
    regionSequencesViewPort.setScrollBarsShown (true, true, false, false);
    regionSequenceListView.setBounds (0, 0, kWidth, kHeight);
    tracksViewPort.setViewedComponent (&regionSequenceListView, false);
    tracksView.setBounds (0, 0, kWidth, kHeight);
    tracksView.addAndMakeVisible (tracksViewPort);

    regionSequencesViewPort.setViewedComponent (&tracksView, false);
    addAndMakeVisible (regionSequencesViewPort);
    addAndMakeVisible (horizontalScrollBar);
    horizontalScrollBar.setColour (ScrollBar::ColourIds::backgroundColourId, Colours::yellow);
    horizontalScrollBar.addListener (this);

    setSize (kWidth, kHeight);
    setResizeLimits (kMinWidth, kMinHeight, 32768, 32768);
    setResizable (true, false);

    if (isARAEditorView())
    {
        getARAEditorView()->addListener (this);

        static_cast<ARADocument*> (getARADocumentController()->getDocument())->addListener (this);

        rebuildView();
    }
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView())
    {
        clearView();

        static_cast<ARADocument*> (getARADocumentController()->getDocument())->removeListener (this);

        getARAEditorView()->removeListener (this);
    }
    horizontalScrollBar.removeListener (this);
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

void ARASampleProjectAudioProcessorEditor::scrollBarMoved (ScrollBar *scrollBarThatHasMoved, double newRangeStart)
{
    auto newRangeStartInt = roundToInt (newRangeStart);
    if (scrollBarThatHasMoved == &horizontalScrollBar)
    {
        tracksViewPort.setViewPosition (newRangeStartInt, tracksViewPort.getViewPositionY());
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

    // set new bounds for all region sequence views
    int width = (int) ((endTime - startTime) * kPixelsPerSecond + 0.5);
    int y = 0;
    for (auto v : regionSequenceViews)
    {
        // child of tracksView
        v->getTrackHeaderView().setBounds (0, y, RegionSequenceView::kTrackHeaderWidth, RegionSequenceView::kHeight);
        // child of regionSequenceListView
        v->setBounds (0, y, width - regionSequencesViewPort.getScrollBarThickness(), RegionSequenceView::kHeight);
        y += RegionSequenceView::kHeight;
    }

    regionSequenceListView.setBounds (0, 0, width, y);
    tracksView.setBounds (0, 0, getWidth() - tracksViewPort.getScrollBarThickness(), y);
    tracksViewPort.setBounds (RegionSequenceView::kTrackHeaderWidth, 0, getWidth() - RegionSequenceView::kTrackHeaderWidth, y);
    regionSequencesViewPort.setBounds (0, 0, getWidth(), getHeight() - tracksViewPort.getScrollBarThickness() - kStatusBarHeight);

    // cache view pos and reset after resizing list view and viewport
    regionSequencesViewPort.setViewPosition (regionSequencesViewPort.getViewPosition());
    tracksViewPort.setViewPosition (tracksViewPort.getViewPosition());

    horizontalScrollBar.setRangeLimits (tracksViewPort.getHorizontalScrollBar().getRangeLimit());
    horizontalScrollBar.setBounds (RegionSequenceView::kTrackHeaderWidth, regionSequencesViewPort.getBottom(), tracksViewPort.getWidth() - regionSequencesViewPort.getScrollBarThickness(), regionSequencesViewPort.getScrollBarThickness());
    horizontalScrollBar.setCurrentRange (tracksViewPort.getHorizontalScrollBar().getCurrentRange());
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
        regionSequenceListView.addAndMakeVisible (sequenceView);
        tracksView.addAndMakeVisible (sequenceView->getTrackHeaderView());
    }

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
