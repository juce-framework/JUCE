#pragma once

#include "JuceHeader.h"

class ARASampleProjectAudioProcessorEditor;
class TrackHeaderView;
class PlaybackRegionView;

//==============================================================================
/**
    RegionSequenceView
    JUCE component used to display all ARA playback regions in a region sequences
*/
class RegionSequenceView    : public Component,
                              private ARARegionSequence::Listener
{
public:
    RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence);
    ~RegionSequenceView();

    void getTimeRange (double& startTime, double& endTime) const;

    Component& getTrackHeaderView();

    void resized() override;

    // ARARegionSequence::Listener overrides
    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion) override;
    void willDestroyRegionSequence (ARARegionSequence* sequence) override;

private:
    void detachFromRegionSequence();

private:
    ARASampleProjectAudioProcessorEditor* editorComponent;
    ARARegionSequence* regionSequence;

    std::unique_ptr<TrackHeaderView> trackHeaderView;
    OwnedArray<PlaybackRegionView> playbackRegionViews;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceView)
};
