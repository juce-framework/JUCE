/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static const int kWidth = 1000;
static const int kHeight = 400;

//==============================================================================
ArasampleProjectAudioProcessorEditor::ArasampleProjectAudioProcessorEditor (ArasampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    _tracksViewport.setScrollBarsShown(true, true);
    if (auto e = getARAEditorView())
    {
        editor = static_cast<ARASampleProjectEditor*>(e);
        editor->setBounds(0, 0, kWidth - _tracksViewport.getScrollBarThickness(), kHeight);
        _tracksViewport.setViewedComponent (editor, false);
    }
    addAndMakeVisible (_tracksViewport);
    setSize (kWidth, kHeight);
}

ArasampleProjectAudioProcessorEditor::~ArasampleProjectAudioProcessorEditor()
{
}

//==============================================================================
void ArasampleProjectAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    if (!getARAEditorView())
    {
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Non ARA Instance. Please re-open as ARA2!", getLocalBounds(), Justification::centred, 1);
    }
}

void ArasampleProjectAudioProcessorEditor::resized()
{
    _tracksViewport.setBounds (0, 0, getWidth(), getHeight());
}
