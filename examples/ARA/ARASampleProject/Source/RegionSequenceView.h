#pragma once

#include "JuceHeader.h"

class ARASampleProjectAudioProcessorEditor;
class PlaybackRegionView;

//==============================================================================
/** 
    RegionSequenceView
    JUCE component used to display ARA region sequences in a host document
    along with their name, order index, color, and selection state
*/
class RegionSequenceView: public Component, 
                          public ARAEditorView::Listener,
                          public ARARegionSequence::Listener
{
public:
    RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence);
    ~RegionSequenceView();

    void paint (Graphics&) override;
    void resized() override;

    // ARAEditorView::Listener overrides: used to track selection
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;

    // ARARegionSequence::Listener overrides
    void didUpdateRegionSequenceProperties (ARARegionSequence* sequence) override;
    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion) override;
    void willDestroyRegionSequence (ARARegionSequence* sequence) override;

    ARARegionSequence* getRegionSequence() const { return regionSequence; }
    
    OwnedArray<PlaybackRegionView>& getPlaybackRegionViews() { return playbackRegionViews; }

    void getTimeRange (double& startTimeInSeconds, double& endTimeInSeconds) const;

private:
    bool isSelected;
    void detachFromRegionSequence();

private:
    ARASampleProjectAudioProcessorEditor* editorComponent;
    ARARegionSequence* regionSequence;
    OwnedArray<PlaybackRegionView> playbackRegionViews;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceView)
};
