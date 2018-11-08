/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/


#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "PluginARADocumentController.h"
#include "PluginARAEditorView.h"

//==============================================================================
/**
*/
class ARASampleProjectAudioProcessorEditor  : public AudioProcessorEditor
#if JucePlugin_Enable_ARA
     , public AudioProcessorEditorARAExtension
     , public ARASampleProjectEditorView::SelectionListener
#endif
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    void onNewSelection (const ARA::PlugIn::ViewSelection* currentSelection) override;

    void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) ARA_NOEXCEPT override;

private:
    double maxRegionSequenceLength;
    juce::CriticalSection selectionLock;
    juce::OwnedArray <RegionSequenceView> regionSequenceViews;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
