/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#include "PluginARADocumentController.h"

//==============================================================================

ARA::PlugIn::EditorView *ARASampleProjectDocumentController::doCreateEditorView()
{
    return new ARASampleProjectEditor (this);
}

//==============================================================================
// This creates new instances of the document controller..
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController ()
{
    return new ARASampleProjectDocumentController();
};


ARASampleProjectEditor::ARASampleProjectEditor (ARA::PlugIn::DocumentController* ctrl)
: ARA::PlugIn::EditorView (ctrl)
{
}

void ARASampleProjectEditor::doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection)
{
    const ScopedLock lock (selectionLock);

    removeAllChildren();
    _maxRegionSequenceLength = 0.0;
    _regionSequenceViews.clear();
    for (auto regionSequence : getDocumentController()->getDocument()->getRegionSequences())
    {
        auto regSeqView = new AudioView (*regionSequence);
        // shows all RegionSequences, highlight ones in current selection.
        for (auto selectedRegionSequence : currentSelection->getRegionSequences())
        {
            if (regionSequence == selectedRegionSequence)
            {
                regSeqView->isSelected (true);
                break;
            }
        }
        addAndMakeVisible (regSeqView);
        _regionSequenceViews.add (regSeqView);
        _maxRegionSequenceLength = std::max (_maxRegionSequenceLength, regSeqView->getStartInSecs() + regSeqView->getLengthInSecs());
    }
    resized();
}

void ARASampleProjectEditor::resized()
{
    int i = 0;
    const int width = getParentWidth();
    const int height = 80;
    for (auto v : _regionSequenceViews)
    {
        double normalizedStartPos = v->getStartInSecs() / _maxRegionSequenceLength;
        double normalizedLength = v->getLengthInSecs() / _maxRegionSequenceLength;
        jassert(normalizedStartPos+normalizedLength <= 1.0);
        v->setBounds (width * normalizedStartPos, height * i, width * normalizedLength, height);
        i++;
    }
    setBounds (0, 0, width, height * i);
}
