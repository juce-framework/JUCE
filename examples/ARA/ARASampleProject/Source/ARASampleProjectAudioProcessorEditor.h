#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"
#include "ARASampleProjectDocumentController.h"
#include "RegionSequenceView.h"

#if ! JucePlugin_Enable_ARA
    #error "bad project configuration, JucePlugin_Enable_ARA is required for compiling this class"
#endif

//==============================================================================
/** 
    Editor class for ARA sample project
    This class manages the UI we use to display region sequences in the 
    ARA document as well as their current selection state
*/
class ARASampleProjectAudioProcessorEditor: public AudioProcessorEditor,
                                            public AudioProcessorEditorARAExtension,    // Provides access to the ARA EditorView instance
                                            public ARAEditorView::Listener,             // Receives ARA selection notifications
                                            public ARARegionSequence::Listener,         // Receives ARA region sequence update notifications
                                            public ARAPlaybackRegion::Listener,         // Receives ARA playback region update notifications
                                            public ARADocumentController::Listener      // Receives ARA document controller update notifications
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // ARASampleProjectEditorView overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;

    // ARARegionSequence::Listener overrides
    virtual void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept override;
    virtual void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept override;

    // ARARegionSequence::Listener overrides
    void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) noexcept override;
    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) noexcept override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) noexcept override;
    void willDestroyRegionSequence (ARARegionSequence* regionSequence) noexcept override;

    // ARADocumentController::Listener overrides
    void doEndEditing (ARADocumentController* documentController) noexcept override;

private:
    void rebuildView ();

private:

    // we'll be displaying all region sequences in the document in a scrollable view
    Viewport regionSequenceViewPort;
    Component regionSequenceListView;

    juce::OwnedArray <RegionSequenceView> regionSequenceViews;

    bool isViewDirty;
    std::set<ARA::PlugIn::RegionSequence*> regionSequencesWithPropertyChanges;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
