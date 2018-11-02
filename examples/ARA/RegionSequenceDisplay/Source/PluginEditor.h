/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "AudioView.h"
#include "PluginARADocumentController.h"
#include "PluginARAEditorView.h"

//==============================================================================
/**
*/
class ArasampleProjectAudioProcessorEditor  : public AudioProcessorEditor
#if JucePlugin_Enable_ARA
     , public AudioProcessorEditorARAExtension
#endif
{
public:
    ArasampleProjectAudioProcessorEditor (ArasampleProjectAudioProcessor&);
    ~ArasampleProjectAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ArasampleProjectAudioProcessor& processor;
    ARASampleProjectEditorView* editor {nullptr};

    std::vector <AudioView*> dummyView;
    Viewport tracksViewport;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArasampleProjectAudioProcessorEditor)
};
