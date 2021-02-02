#include "RegionSequenceHeaderView.h"

//==============================================================================
RegionSequenceHeaderView::RegionSequenceHeaderView (juce::ARAEditorView* view, juce::ARARegionSequence* sequence)
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
    g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
    g.drawRect (rect);
    rect.reduce (1, 1);

    const juce::Colour trackColour = juce::convertOptionalARAColour (regionSequence->getColor());
    g.setColour (trackColour);
    g.fillRect (rect);

    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (juce::Font (12.0f));
    g.drawText (juce::convertOptionalARAString (regionSequence->getName()), rect, juce::Justification::centredLeft);
}

//==============================================================================
void RegionSequenceHeaderView::onNewSelection (const juce::ARAViewSelection& viewSelection)
{
    const bool selected = ARA::contains (viewSelection.getRegionSequences(), regionSequence);
    if (selected != isSelected)
    {
        isSelected = selected;
        repaint();
    }
}

void RegionSequenceHeaderView::didUpdateRegionSequenceProperties (juce::ARARegionSequence* /*regionSequence*/)
{
    repaint();
}

void RegionSequenceHeaderView::willDestroyRegionSequence (juce::ARARegionSequence* /*regionSequence*/)
{
    detachFromRegionSequence();
}
