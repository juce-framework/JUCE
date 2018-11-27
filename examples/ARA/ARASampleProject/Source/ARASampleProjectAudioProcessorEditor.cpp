#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARASampleProjectDocumentController.h"

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
: AudioProcessorEditor (&p),
  AudioProcessorEditorARAExtension (&p),
  isViewDirty (false)
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
        getARAEditorView ()->addListener (this);

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

        getARAEditorView ()->removeListener (this);
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
    double maxRegionSequenceLength = 0.0;
    int i = 0;
    const int width = getWidth();

    // compute region sequence view bounds in terms of kVisibleSeconds and kRegionSequenceHeight
    // and determine the length in seconds of the longest ARA region sequence
    for (auto v : regionSequenceViews)
    {
        double startInSeconds, endInSeconds;
        v->getTimeRange (startInSeconds, endInSeconds);

        double normalizedEnd = endInSeconds / kVisibleSeconds;
        v->setBounds (0, kRegionSequenceHeight * i, kTrackHeaderWidth + (int) (width * normalizedEnd), kRegionSequenceHeight);

        maxRegionSequenceLength = jmax (maxRegionSequenceLength, endInSeconds);
        i++;
    }

    // normalized width = view width in terms of kVisibleSeconds
    // size this to ensure we can see one second beyond the longest region sequnce
    const double normalizedWidth = (maxRegionSequenceLength + 1) / kVisibleSeconds;
    regionSequenceListView.setBounds (0, 0, kTrackHeaderWidth + (int) (normalizedWidth * width), kRegionSequenceHeight * i);
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
        sequenceView->getRegionSequence()->addListener (this);
        regionSequenceViews.add (sequenceView);
        regionSequenceListView.addAndMakeVisible (sequenceView);
    }

    resized();
}

void ARASampleProjectAudioProcessorEditor::clearView()
{
    for (auto v : regionSequenceViews)
        v->getRegionSequence()->removeListener (this);

    regionSequenceViews.clear();
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences)
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

void ARASampleProjectAudioProcessorEditor::willDestroyRegionSequence (ARARegionSequence* regionSequence)
{
    for (int i = 0; i < regionSequenceViews.size(); i++)
    {
        if (regionSequenceViews[i]->getRegionSequence() == regionSequence)
        {
            regionSequenceViews.remove (i, true);
            return;
        }
    }
}
