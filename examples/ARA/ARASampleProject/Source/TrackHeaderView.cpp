#include "TrackHeaderView.h"

//==============================================================================
TrackHeaderView::TrackHeaderView (ARAEditorView* view, ARARegionSequence* sequence)
    : editorView (view),
      regionSequence (sequence)
{
    regionSequence->addListener (this);

    editorView->addListener (this);
    onNewSelection (editorView->getViewSelection());
}

TrackHeaderView::~TrackHeaderView()
{
    detachFromRegionSequence();
}

void TrackHeaderView::detachFromRegionSequence()
{
    if (regionSequence == nullptr)
        return;

    regionSequence->removeListener (this);

    editorView->removeListener (this);

    regionSequence = nullptr;
}

//==============================================================================
void TrackHeaderView::paint (juce::Graphics& g)
{
    if (regionSequence == nullptr)
        return;

    Colour trackColour;
    if (auto& colour = regionSequence->getColor())
        trackColour = Colour::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);

    // draw region sequence header
    g.setColour (trackColour);
    g.fillRect (getLocalBounds());

    // draw selection state as a yellow border around the header
    g.setColour (isSelected ? Colours::yellow : Colours::black);
    g.drawRect (getLocalBounds());

    if (auto& name = regionSequence->getName())
    {
        g.setColour (trackColour.contrasting (1.0f));
        g.setFont (Font (12.0f));
        g.drawText (convertARAString (name), getLocalBounds(), Justification::centredLeft);
    }
}

//==============================================================================
void TrackHeaderView::onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection)
{
    jassert (regionSequence != nullptr);

    bool isOurRegionSequenceSelected = ARA::contains (currentSelection.getRegionSequences(), regionSequence);
    if (isOurRegionSequenceSelected != isSelected)
    {
        isSelected = isOurRegionSequenceSelected;
        repaint();
    }
}

void TrackHeaderView::didUpdateRegionSequenceProperties (ARARegionSequence* sequence)
{
    jassert (regionSequence == sequence);

    repaint();
}

void TrackHeaderView::willDestroyRegionSequence (ARARegionSequence* sequence)
{
    jassert (regionSequence == sequence);

    detachFromRegionSequence();
}
