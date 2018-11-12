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
#include "RegionSequenceView.h"

//==============================================================================
/** 
    Editor class for ARA sample project
    This class manages the UI we use to display region sequences in the 
    ARA document as well as their current selection state
*/
class ARASampleProjectAudioProcessorEditor: public AudioProcessorEditor
#if JucePlugin_Enable_ARA
     , public AudioProcessorEditorARAExtension               // Provides access to the ARA EditorView instance
     , public ARASampleProjectEditorView::SelectionListener  // Receives ARA selection notifications
     , public ARARegionSequenceUpdateListener                // Receives ARA region sequence update notifications
#endif
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // ARASampleProjectEditorView overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection* currentSelection) override;

    // ARARegionSequenceUpdateListener overrides
    void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) ARA_NOEXCEPT override;

private:

    // we'll be displaying all region sequences in the document in a scrollable view
    Viewport regionSequenceViewPort;
    Component regionSequenceListView;

    double maxRegionSequenceLength;
    juce::CriticalSection selectionLock;
    juce::OwnedArray <RegionSequenceView> regionSequenceViews;

    std::set<ARA::PlugIn::RegionSequence*> regionSequencesWithPropertyChanges;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
