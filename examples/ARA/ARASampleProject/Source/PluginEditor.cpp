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
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p)
#if JucePlugin_Enable_ARA
     , AudioProcessorEditorARAExtension(&p)
#endif
{
    tracksViewport.setScrollBarsShown (true, true);
    if (auto e = getARAEditorView())
    {
        editor = static_cast<ARASampleProjectEditorView*> (e);
        editor->setBounds (0, 0, kWidth - tracksViewport.getScrollBarThickness(), kHeight);
        tracksViewport.setViewedComponent (editor, false);
    }
    addAndMakeVisible (tracksViewport);
    setSize (kWidth, kHeight);
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::paint (Graphics& g)
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

void ARASampleProjectAudioProcessorEditor::resized()
{
    tracksViewport.setBounds (0, 0, getWidth(), getHeight());
}
