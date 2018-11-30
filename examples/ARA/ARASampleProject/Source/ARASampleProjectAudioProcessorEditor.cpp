#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARASampleProjectDocumentController.h"

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
: AudioProcessorEditor (&p),
  AudioProcessorEditorARAExtension (&p)
{
    regionSequenceViewPort.setScrollBarsShown (true, true);
    regionSequenceListView.setBounds (0, 0, kWidth - regionSequenceViewPort.getScrollBarThickness(), kHeight);
    regionSequenceViewPort.setViewedComponent (&regionSequenceListView, false);
    addAndMakeVisible (regionSequenceViewPort);

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
    // compute region sequence view bounds in terms of kVisibleSeconds and kRegionSequenceHeight
    // by finding the range of start and end times for all of our region sequence views
    minRegionSequenceStartTime = std::numeric_limits<double>::max();
    double maxRegionSequenceEndTime = std::numeric_limits<double>::lowest();
    for (auto v : regionSequenceViews)
    {
        double startInSeconds, endInSeconds;
        v->getTimeRange (startInSeconds, endInSeconds);

        maxRegionSequenceEndTime = jmax (maxRegionSequenceEndTime, endInSeconds);
        minRegionSequenceStartTime = jmin (minRegionSequenceStartTime, startInSeconds);
    }

    // offset the start time by our "pad" value (converting pixels to seconds)
    const double secondsPerPixel = double (kVisibleSeconds) / getWidth();
    minRegionSequenceStartTime -= (secondsPerPixel * kRegionSequenceDurationPadPixels);
    
    // place each region sequence view such that all views start at minRegionSequenceStartTime
    // and extend to cover their full duration (including room for the track header width)
    int i = 0;
    for (auto v : regionSequenceViews)
    {
        double startInSeconds, endInSeconds;
        v->getTimeRange (startInSeconds, endInSeconds);
        double normalizedDuration = (endInSeconds - minRegionSequenceStartTime) / kVisibleSeconds;
        v->setBounds (0, kRegionSequenceHeight * i, kTrackHeaderWidth + (int) (getWidth() * normalizedDuration), kRegionSequenceHeight);
        i++;
    }

    // size our region sequence list view to fit all of our region sequences + our "pad" value
    double totalRegionSequenceDuration = maxRegionSequenceEndTime - minRegionSequenceStartTime;
    const double normalizedWidth = (totalRegionSequenceDuration) / kVisibleSeconds;
    regionSequenceListView.setBounds (0, 0, (int) (normalizedWidth * getWidth()) + kTrackHeaderWidth + kRegionSequenceDurationPadPixels, kRegionSequenceHeight * i);

    regionSequenceViewPort.setBounds (0, 0, getWidth(), getHeight());
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
    }

    resized();
}

void ARASampleProjectAudioProcessorEditor::clearView()
{
    regionSequenceViews.clear();
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::onHideRegionSequences (std::vector<ARARegionSequence*> const& /*regionSequences*/)
{
    rebuildView();
}

void ARASampleProjectAudioProcessorEditor::doEndEditing (ARADocument* document)
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
