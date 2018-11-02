/*
  ==============================================================================

    PluginARAEditorView.cpp
    Created: 2 Nov 2018 3:08:37pm
    Author:  john

  ==============================================================================
*/

#include "PluginARAEditorView.h"

ARASampleProjectEditorView::ARASampleProjectEditorView (ARA::PlugIn::DocumentController* ctrl) noexcept
: ARA::PlugIn::EditorView (ctrl)
{}

void ARASampleProjectEditorView::doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept
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

void ARASampleProjectEditorView::resized()
{
    int i = 0;
    const int width = getParentWidth();
    const int height = 80;
    for (auto v : regionSequenceViews)
    {
        double normalizedStartPos = v->getStartInSecs() / maxRegionSequenceLength;
        double normalizedLength = v->getLengthInSecs() / maxRegionSequenceLength;
        jassert (normalizedStartPos+normalizedLength <= 1.0);
        v->setBounds ((int) (width * normalizedStartPos), height * i, (int) (width * normalizedLength), height);
        i++;
    }
    setBounds (0, 0, width, height * i);
}
