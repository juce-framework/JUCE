#include "RegionSequenceHeaderView.h"

using namespace juce;

//==============================================================================
RegionSequenceHeaderView::RegionSequenceHeaderView (ARAEditorView* view, ARARegionSequence* sequence)
    : editorView (view),
      regionSequence (sequence)
{
    regionSequence->addListener (this);

    editorView->addListener (this);
    onNewSelection (editorView->getViewSelection());
}

RegionSequenceHeaderView::~RegionSequenceHeaderView()
{
    detachFromRegionSequence();
}

void RegionSequenceHeaderView::detachFromRegionSequence()
{
    if (regionSequence == nullptr)
        return;

    regionSequence->removeListener (this);

    editorView->removeListener (this);

    regionSequence = nullptr;
}

//==============================================================================
void RegionSequenceHeaderView::paint (juce::Graphics& g)
{
    if (regionSequence == nullptr)
        return;

    auto rect = getLocalBounds();
    g.setColour (isSelected ? Colours::yellow : Colours::black);
    g.drawRect (rect);
    rect.reduce (1, 1);

    const Colour trackColour = convertOptionalARAColour (regionSequence->getColor());
    g.setColour (trackColour);
    g.fillRect (rect);

    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (Font (12.0f));
    g.drawText (convertOptionalARAString (regionSequence->getName()), rect, Justification::centredLeft);
}

//==============================================================================
void RegionSequenceHeaderView::onNewSelection (const ARAViewSelection& viewSelection)
{
    const bool selected = ARA::contains (viewSelection.getRegionSequences(), regionSequence);
    if (selected != isSelected)
    {
        isSelected = selected;
        repaint();
    }
}

void RegionSequenceHeaderView::didUpdateRegionSequenceProperties (ARARegionSequence* /*regionSequence*/)
{
    repaint();
}

void RegionSequenceHeaderView::willDestroyRegionSequence (ARARegionSequence* /*regionSequence*/)
{
    detachFromRegionSequence();
}
