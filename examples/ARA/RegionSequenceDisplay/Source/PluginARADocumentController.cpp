/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#include "PluginARADocumentController.h"
#include "PluginARAPlaybackRenderer.h"

ARASampleProjectDocumentController::ARASampleProjectDocumentController() noexcept
: juce::ARADocumentController ()
{
    araAudioSourceReadingThread.reset (new TimeSliceThread (String (JucePlugin_Name) + " ARA Sample Reading Thread"));
    araAudioSourceReadingThread->startThread();
}

ARA::PlugIn::EditorView* ARASampleProjectDocumentController::doCreateEditorView() noexcept
{
    return new ARASampleProjectEditor (this);
}

ARA::PlugIn::PlaybackRenderer* ARASampleProjectDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARASampleProjectPlaybackRenderer (this, *araAudioSourceReadingThread.get (), (1 << 16));
}

//==============================================================================
// This creates new instances of the document controller..
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController() noexcept
{
    return new ARASampleProjectDocumentController();
};


ARASampleProjectEditor::ARASampleProjectEditor (ARA::PlugIn::DocumentController* ctrl) noexcept
: ARA::PlugIn::EditorView (ctrl)
{
}

void ARASampleProjectEditor::doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept
{
    const ScopedLock lock (selectionLock);

    removeAllChildren();
    maxRegionSequenceLength = 0.0;
    regionSequenceViews.clear();
    for (auto regionSequence : getDocumentController()->getDocument()->getRegionSequences())
    {
        auto regSeqView = new AudioView (*regionSequence);
        // shows all RegionSequences, highlight ones in current selection.
        for (auto selectedRegionSequence : currentSelection->getRegionSequences())
        {
            if (regionSequence == selectedRegionSequence)
            {
                regSeqView->setIsSelected (true);
                break;
            }
        }
        addAndMakeVisible (regSeqView);
        regionSequenceViews.add (regSeqView);
        maxRegionSequenceLength = std::max (maxRegionSequenceLength, regSeqView->getStartInSecs() + regSeqView->getLengthInSecs());
    }
    resized();
}

void ARASampleProjectEditor::resized()
{
    int i = 0;
    const int width = getParentWidth();
    const int height = 80;
    for (auto v : regionSequenceViews)
    {
        double normalizedStartPos = v->getStartInSecs() / maxRegionSequenceLength;
        double normalizedLength = v->getLengthInSecs() / maxRegionSequenceLength;
        jassert(normalizedStartPos+normalizedLength <= 1.0);
        v->setBounds ((int) (width * normalizedStartPos), height * i, (int) (width * normalizedLength), height);
        i++;
    }
    setBounds (0, 0, width, height * i);
}
