#include "ARASampleProjectAudioProcessor.h"
#include "ARASampleProjectAudioProcessorEditor.h"

static const int kWidth = 1000;
static const int kHeight = 400;
static const int kRegionSequenceHeight = 80;
static const int kVisibleSeconds = 10;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
: AudioProcessorEditor (&p),
  AudioProcessorEditorARAExtension (&p),
  isViewDirty (false)
{
    // init viewport and region sequence list view
    regionSequenceViewPort.setScrollBarsShown (true, true);
    regionSequenceListView.setBounds (0, 0, kWidth - regionSequenceViewPort.getScrollBarThickness(), kHeight);
    regionSequenceViewPort.setViewedComponent (&regionSequenceListView, false);
    addAndMakeVisible (regionSequenceViewPort);
    
    setSize (kWidth, kHeight);

    // manually invoke the onNewSelection callback to refresh our UI with the current selection
    // TODO JUCE_ARA should we rename the function that recreates the view?
    if (isARAEditorView())
    {
        static_cast<ARADocument*> (getARADocumentController()->getDocument())->addListener (this);
        getARAEditorView()->addSelectionListener (this);

        rebuildView();
        onNewSelection (getARAEditorView()->getViewSelection());
    }
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView ())
    {
        auto document = static_cast<ARADocument*> (getARADocumentController()->getDocument());
        document->removeListener (this);
        getARAEditorView()->removeSelectionListener (this);

        for (auto regionSequence : document->getRegionSequences())
        {
            static_cast<ARARegionSequence*>(regionSequence)->removeListener (this);
            for (auto playbackRegion : regionSequence->getPlaybackRegions())
                static_cast<ARAPlaybackRegion*>(playbackRegion)->removeListener (this);
        }
    }
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
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
        maxRegionSequenceLength = jmax (maxRegionSequenceLength, v->getStartInSecs() + v->getLengthInSecs());

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
    // flag the region as selected if it's a part of the current selection
    for (RegionSequenceView* regionSequenceView : regionSequenceViews)
    {
        bool isSelected = ARA::contains (currentSelection.getRegionSequences(), regionSequenceView->getRegionSequence());
        regionSequenceView->setIsSelected (isSelected);
    }
}

void ARASampleProjectAudioProcessorEditor::rebuildView()
{
    auto& regionSequences = getARAEditorView()->getDocumentController()->getDocument()->getRegionSequences();
    for (int i = 0; i < regionSequences.size(); i++)
    {
        auto regionSequence = static_cast<ARARegionSequence*>(regionSequences[i]);

        // construct the region sequence view if we don't yet have one
        if (regionSequenceViews.size() <= i)
        {
            regionSequenceViews.add (new RegionSequenceView (regionSequence));
        }
        // reconstruct the region sequence view if the sequence order or properties have changed
        else if ((regionSequenceViews[i]->getRegionSequence() != regionSequences[i]) ||
                 (regionSequencesWithPropertyChanges.count (regionSequences[i]) > 0))
        {
            regionSequenceViews.set (i, new RegionSequenceView (regionSequence), true);
        }

        // make the region sequence view visible and keep track of the longest region sequence
        regionSequenceListView.addAndMakeVisible (regionSequenceViews[i]);
    }

    // remove any views for region sequences no longer in the document
    regionSequenceViews.removeLast (regionSequenceViews.size() - (int) regionSequences.size());

    // Clear property change state and resize view
    resized();
}

void ARASampleProjectAudioProcessorEditor::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept
{
    if ((playbackRegion->getStartInPlaybackTime () != newProperties->startInPlaybackTime) ||
        (playbackRegion->getDurationInPlaybackTime () != newProperties->durationInPlaybackTime))
    {
        regionSequencesWithPropertyChanges.insert (playbackRegion->getRegionSequence ());
        isViewDirty = true;
    }
}

void ARASampleProjectAudioProcessorEditor::willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    playbackRegion->removeListener (this);
    regionSequencesWithPropertyChanges.insert (playbackRegion->getRegionSequence ());
    isViewDirty = true;
}

void ARASampleProjectAudioProcessorEditor::didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) noexcept
{
    regionSequencesWithPropertyChanges.insert (regionSequence);
    isViewDirty = true;
}

void ARASampleProjectAudioProcessorEditor::willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* /*playbackRegion*/) noexcept
{
    regionSequencesWithPropertyChanges.insert (regionSequence);
    isViewDirty = true;
}

void ARASampleProjectAudioProcessorEditor::didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* /*playbackRegion*/) noexcept
{
    regionSequencesWithPropertyChanges.insert (regionSequence);
    isViewDirty = true;
}

void ARASampleProjectAudioProcessorEditor::willDestroyRegionSequence (ARARegionSequence* regionSequence) noexcept
{
    regionSequence->removeListener (this);
    isViewDirty = true;
}

void ARASampleProjectAudioProcessorEditor::doEndEditing (ARADocument* document) noexcept
{
    for (auto regionSequence : document->getRegionSequences ())
    {
        static_cast<ARARegionSequence*>(regionSequence)->addListener (this);
        for (auto playbackRegion : regionSequence->getPlaybackRegions ())
            static_cast<ARAPlaybackRegion*>(playbackRegion)->addListener (this);
    }

    if (isViewDirty)
    {
        rebuildView ();
        regionSequencesWithPropertyChanges.clear ();
        isViewDirty = false;
    }
}
