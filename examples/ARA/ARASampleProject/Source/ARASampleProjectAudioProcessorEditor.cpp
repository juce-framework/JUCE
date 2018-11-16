#include "ARASampleProjectAudioProcessor.h"
#include "ARASampleProjectAudioProcessorEditor.h"

static const int kWidth = 1000;
static const int kHeight = 400;
static const int kRegionSequenceHeight = 80;
static const int kVisibleSeconds = 10;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p)
#if JucePlugin_Enable_ARA
    , AudioProcessorEditorARAExtension (&p)
#endif
{
    // init viewport and region sequence list view
    regionSequenceViewPort.setScrollBarsShown (true, true);
    regionSequenceListView.setBounds (0, 0, kWidth - regionSequenceViewPort.getScrollBarThickness(), kHeight);
    regionSequenceViewPort.setViewedComponent (&regionSequenceListView, false);
    addAndMakeVisible (regionSequenceViewPort);
    
    setSize (kWidth, kHeight);

    // manually invoke the onNewSelection callback to refresh our UI with the current selection
    // TODO JUCE_ARA should we rename the function that recreates the view?
    getARAEditorView()->addSelectionListener (this);
    onNewSelection (getARAEditorView()->getViewSelection());
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    getARAEditorView()->removeSelectionListener (this);
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    if (isARAEditorView() == false)
    {
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Non ARA Instance. Please re-open as ARA2!", getLocalBounds(), Justification::centred, 1);
    }
}

void ARASampleProjectAudioProcessorEditor::resized()
{
    int i = 0;
    const int width = getWidth();

    // compute region sequence view bounds in terms of kVisibleSeconds and kRegionSequenceHeight
    for (auto v : regionSequenceViews)
    {
        double normalizedStartPos = v->getStartInSecs() / kVisibleSeconds;
        double normalizedLength = v->getLengthInSecs() / kVisibleSeconds;
        v->setBounds ((int) (width * normalizedStartPos), kRegionSequenceHeight * i, (int) (width * normalizedLength), kRegionSequenceHeight);
        i++;
    }

    // normalized width = view width in terms of kVisibleSeconds
    // size this to ensure we can see one second beyond the longest region sequnce
    const double normalizedWidth = (maxRegionSequenceLength + 1) / kVisibleSeconds;
    regionSequenceListView.setBounds (0, 0, (int) (normalizedWidth * width), kRegionSequenceHeight * i);
    regionSequenceViewPort.setBounds (0, 0, getWidth(), getHeight());
}

// rebuild our region sequence views and display selection state
void ARASampleProjectAudioProcessorEditor::onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection)
{
    // determine the length in seconds of the longest ARA region sequence
    maxRegionSequenceLength = 0.0;

    auto& regionSequences = getARAEditorView()->getDocumentController()->getDocument()->getRegionSequences();
    for (int i = 0; i < regionSequences.size(); i++)
    {
        ARARegionSequence* regionSequence = static_cast<ARARegionSequence*>(regionSequences[i]);

        // construct the region sequence view if we don't yet have one
        if (regionSequenceViews.size() <= i)
        {
            regionSequenceViews.add (new RegionSequenceView (regionSequence));
        }
        // reconstruct the region sequence view if the sequence order or properties have changed
        else if (( regionSequenceViews[i]->getRegionSequence() != regionSequences[i]) ||
                 ( regionSequencesWithPropertyChanges.count (regionSequences[i]) > 0 ))
        {
            regionSequenceViews.set (i, new RegionSequenceView (regionSequence), true);
        }

        // flag the region as selected if it's a part of the current selection, 
        // or not selected if we have no selection
        bool isSelected = ARA::contains (currentSelection.getRegionSequences(), regionSequences[i]);
        regionSequenceViews[i]->setIsSelected (isSelected);

        // make the region sequence view visible and keep track of the longest region sequence
        regionSequenceListView.addAndMakeVisible (regionSequenceViews[i]);
        maxRegionSequenceLength = std::max (maxRegionSequenceLength, regionSequenceViews[i]->getStartInSecs() + regionSequenceViews[i]->getLengthInSecs());
    }

    // remove any views for region sequences no longer in the document
    regionSequenceViews.removeLast (regionSequenceViews.size() - (int) regionSequences.size());

    // Clear property change state and resize view
    regionSequencesWithPropertyChanges.clear();
    resized();
}

void ARASampleProjectAudioProcessorEditor::didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) noexcept
{
    // manually invoke onNewSelection here to redraw the region sequence views
    regionSequencesWithPropertyChanges.insert (regionSequence);
    onNewSelection (getARAEditorView()->getViewSelection());
}

void ARASampleProjectAudioProcessorEditor::willDestroyRegionSequence (ARARegionSequence* /*regionSequence*/) noexcept
{
    onNewSelection (getARAEditorView()->getViewSelection());
}
