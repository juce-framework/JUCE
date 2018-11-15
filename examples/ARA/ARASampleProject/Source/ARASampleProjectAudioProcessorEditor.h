#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"
#include "ARASampleProjectDocumentController.h"
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
     , public ARAEditorView::Listener                        // Receives ARA selection notifications
     , public ARARegionSequence::Listener                    // Receives ARA region sequence update notifications
#endif
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // ARASampleProjectEditorView overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;

    // ARAPlaybackRegion::Listener overrides
    void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) noexcept override;
    void willDestroyRegionSequence (ARARegionSequence* regionSequence) noexcept override;

private:

    // we'll be displaying all region sequences in the document in a scrollable view
    Viewport regionSequenceViewPort;
    Component regionSequenceListView;

    double maxRegionSequenceLength;
    juce::OwnedArray <RegionSequenceView> regionSequenceViews;

    std::set<ARA::PlugIn::RegionSequence*> regionSequencesWithPropertyChanges;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
