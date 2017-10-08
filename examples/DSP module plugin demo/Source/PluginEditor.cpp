/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
DspModulePluginDemoAudioProcessorEditor::DspModulePluginDemoAudioProcessorEditor (DspModulePluginDemoAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      inputVolumeLabel        ({ }, processor.inputVolumeParam->name),
      outputVolumeLabel       ({ }, processor.outputVolumeParam->name),
      lowPassFilterFreqLabel  ({ }, processor.lowPassFilterFreqParam->name),
      highPassFilterFreqLabel ({ }, processor.highPassFilterFreqParam->name),
      stereoLabel({}, processor.stereoParam->name),
      slopeLabel ({ }, processor.slopeParam->name),
      waveshaperLabel({ }, processor.waveshaperParam->name),
      cabinetTypeLabel({ }, processor.cabinetTypeParam->name)
{
    //==============================================================================
    addAndMakeVisible (inputVolumeSlider        = new ParameterSlider (*processor.inputVolumeParam));
    addAndMakeVisible (outputVolumeSlider       = new ParameterSlider (*processor.outputVolumeParam));
    addAndMakeVisible (lowPassFilterFreqSlider  = new ParameterSlider (*processor.lowPassFilterFreqParam));
    addAndMakeVisible (highPassFilterFreqSlider = new ParameterSlider (*processor.highPassFilterFreqParam));

    addAndMakeVisible (inputVolumeLabel);
    inputVolumeLabel.setJustificationType (Justification::centredLeft);
    inputVolumeLabel.attachToComponent (inputVolumeSlider, true);

    addAndMakeVisible (outputVolumeLabel);
    outputVolumeLabel.setJustificationType (Justification::centredLeft);
    outputVolumeLabel.attachToComponent (outputVolumeSlider, true);

    addAndMakeVisible (lowPassFilterFreqLabel);
    lowPassFilterFreqLabel.setJustificationType (Justification::centredLeft);
    lowPassFilterFreqLabel.attachToComponent (lowPassFilterFreqSlider, true);

    addAndMakeVisible (highPassFilterFreqLabel);
    highPassFilterFreqLabel.setJustificationType (Justification::centredLeft);
    highPassFilterFreqLabel.attachToComponent (highPassFilterFreqSlider, true);

    //==============================================================================
    addAndMakeVisible (stereoBox);

    auto i = 1;
    for (auto choice : processor.stereoParam->choices)
        stereoBox.addItem (choice, i++);

    stereoBox.addListener (this);
    stereoBox.setSelectedId (processor.stereoParam->getIndex() + 1);

    addAndMakeVisible (stereoLabel);
    stereoLabel.setJustificationType (Justification::centredLeft);
    stereoLabel.attachToComponent (&stereoBox, true);

    //==============================================================================
    addAndMakeVisible(slopeBox);

    i = 1;
    for (auto choice : processor.slopeParam->choices)
        slopeBox.addItem(choice, i++);

    slopeBox.addListener(this);
    slopeBox.setSelectedId(processor.slopeParam->getIndex() + 1);

    addAndMakeVisible(slopeLabel);
    slopeLabel.setJustificationType(Justification::centredLeft);
    slopeLabel.attachToComponent(&slopeBox, true);

    //==============================================================================
    addAndMakeVisible (waveshaperBox);

    i = 1;
    for (auto choice : processor.waveshaperParam->choices)
        waveshaperBox.addItem (choice, i++);

    waveshaperBox.addListener (this);
    waveshaperBox.setSelectedId (processor.waveshaperParam->getIndex() + 1);

    addAndMakeVisible (waveshaperLabel);
    waveshaperLabel.setJustificationType (Justification::centredLeft);
    waveshaperLabel.attachToComponent (&waveshaperBox, true);

    //==============================================================================
    addAndMakeVisible (cabinetTypeBox);

    i = 1;
    for (auto choice : processor.cabinetTypeParam->choices)
        cabinetTypeBox.addItem (choice, i++);

    cabinetTypeBox.addListener (this);
    cabinetTypeBox.setSelectedId (processor.cabinetTypeParam->getIndex() + 1);

    addAndMakeVisible (cabinetTypeLabel);
    cabinetTypeLabel.setJustificationType (Justification::centredLeft);
    cabinetTypeLabel.attachToComponent (&cabinetTypeBox, true);

    //==============================================================================
    addAndMakeVisible (cabinetSimButton);
    cabinetSimButton.addListener (this);
    cabinetSimButton.setButtonText (processor.cabinetSimParam->name);

    addAndMakeVisible (oversamplingButton);
    oversamplingButton.addListener (this);
    oversamplingButton.setButtonText (processor.oversamplingParam->name);

    //==============================================================================
    setSize (600, 400);
}

DspModulePluginDemoAudioProcessorEditor::~DspModulePluginDemoAudioProcessorEditor()
{
}

//==============================================================================
void DspModulePluginDemoAudioProcessorEditor::paint (Graphics& g)
{
    g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.fillAll();
}

void DspModulePluginDemoAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (10);
    bounds.removeFromTop (10);
    bounds.removeFromLeft (125);

    //==============================================================================
    inputVolumeSlider->setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (5);

    outputVolumeSlider->setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (15);

    highPassFilterFreqSlider->setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (5);

    lowPassFilterFreqSlider->setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (15);

    //==============================================================================
    stereoBox.setBounds (bounds.removeFromTop(30));
    bounds.removeFromTop (5);

    slopeBox.setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (5);

    waveshaperBox.setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (5);

    cabinetTypeBox.setBounds (bounds.removeFromTop (30));
    bounds.removeFromTop (15);

    //==============================================================================
    auto buttonSlice = bounds.removeFromTop (30);
    cabinetSimButton.setSize (200, buttonSlice.getHeight());
    cabinetSimButton.setCentrePosition (buttonSlice.getCentre());
    bounds.removeFromTop(5);

    buttonSlice = bounds.removeFromTop (30);
    oversamplingButton.setSize(200, buttonSlice.getHeight());
    oversamplingButton.setCentrePosition(buttonSlice.getCentre());
}
//==============================================================================
void DspModulePluginDemoAudioProcessorEditor::comboBoxChanged (ComboBox* box)
{
    auto index = box->getSelectedItemIndex();

    if (box == &stereoBox)
    {
        processor.stereoParam->operator= (index);
    }
    else if (box == &slopeBox)
    {
        processor.slopeParam->operator= (index);
    }
    else if (box == &waveshaperBox)
    {
        processor.waveshaperParam->operator= (index);
    }
    else if (box == &cabinetTypeBox)
    {
        processor.cabinetTypeParam->operator= (index);
    }
}

void DspModulePluginDemoAudioProcessorEditor::buttonClicked (Button* button)
{
    if (button == &cabinetSimButton)
    {
        processor.cabinetSimParam->operator= (cabinetSimButton.getToggleState());
    }
    else if (button == &oversamplingButton)
    {
        processor.oversamplingParam->operator= (oversamplingButton.getToggleState());
    }
}
