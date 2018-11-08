/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginARAEditorView.h"

static const int kWidth = 1000;
static const int kHeight = 400;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p)
#if JucePlugin_Enable_ARA
    , AudioProcessorEditorARAExtension (&p)
    , ARASampleProjectEditorView::SelectionListener (getARAEditorView ())
#endif
{
    setBounds (0, 0, kWidth, kHeight);
    setSize (kWidth, kHeight);

    // manually invoke the onNewSelection callback to refresh our UI with the current selection
    // TODO should we rename the function that recreates the view?
    onNewSelection (getMostRecentSelection ());
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel ().findColour (ResizableWindow::backgroundColourId));
    if (isARAEditorView() == false)
    {
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Non ARA Instance. Please re-open as ARA2!", getLocalBounds (), Justification::centred, 1);
    }
}

void ARASampleProjectAudioProcessorEditor::resized ()
{
    int i = 0;
    const int width = getWidth ();
    const int height = (int) (double (getHeight ()) / regionSequenceViews.size ());
    for (auto v : regionSequenceViews)
    {
        double normalizedStartPos = v->getStartInSecs () / maxRegionSequenceLength;
        double normalizedLength = v->getLengthInSecs () / maxRegionSequenceLength;
        jassert (normalizedStartPos + normalizedLength <= 1.0);
        v->setBounds ((int) (width * normalizedStartPos), height * i, (int) (width * normalizedLength), height);
        i++;
    }
}

// shows all RegionSequences, highlight ones in current selection.
void ARASampleProjectAudioProcessorEditor::onNewSelection (const ARA::PlugIn::ViewSelection* currentSelection)
{
    const ScopedLock lock (selectionLock);

    maxRegionSequenceLength = 0.0;

    auto& regionSequences = getARAEditorView ()->getDocumentController ()->getDocument ()->getRegionSequences ();
    for (int i = 0; i < regionSequences.size (); i++)
    {
        ARA::PlugIn::RegionSequence* regionSequence = regionSequences[i];
        if (regionSequenceViews.size () <= i)
        {
            // construct the region sequence view if we don't yet have one
            regionSequenceViews.add (new RegionSequenceView (*regionSequence ));
        }
        else if (regionSequenceViews[i]->getRegionSequence () != regionSequence )
        {
            // reconstruct the region sequence view if the sequence order has changed
            regionSequenceViews.set (i, new RegionSequenceView (*regionSequence ), true);
        }

        // flag the region as selected if it's a part of the current selection, 
        // or not selected if we have no selection
        auto& selectedRegionSequences = currentSelection->getRegionSequences ();
        bool selectionState = selectedRegionSequences.end () != std::find (selectedRegionSequences.begin (), selectedRegionSequences.end (), regionSequence);
        regionSequenceViews[i]->setIsSelected (selectionState);

        // add region sequence view component and keep track of the longest region sequence
        addAndMakeVisible (regionSequenceViews[i]);
        maxRegionSequenceLength = std::max (maxRegionSequenceLength, regionSequenceViews[i]->getStartInSecs () + regionSequenceViews[i]->getLengthInSecs ());
    }

    // remove any views for region sequences no longer in the document
    regionSequenceViews.removeLast (regionSequenceViews.size () - (int) regionSequences.size ());

    resized ();
}
