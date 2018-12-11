#pragma once

#include "JuceHeader.h"

class ARASampleProjectAudioProcessorEditor;
class PlaybackRegionView;

//==============================================================================
/**
    RegionSequenceView
    JUCE component used to display ARA region sequences in a host document
    along with their name, color, and selection state
*/
class RegionSequenceView    : public Component,
                              private ARAEditorView::Listener,
                              private ARARegionSequence::Listener
{
public:
    RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence);
    ~RegionSequenceView();

    void getTimeRange (double& startTime, double& endTime) const;

    void resized() override;

    Component& getTrackHeaderView();

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;

    // ARARegionSequence::Listener overrides
    void didUpdateRegionSequenceProperties (ARARegionSequence* sequence) override;
    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion) override;
    void willDestroyRegionSequence (ARARegionSequence* sequence) override;

private:
    void detachFromRegionSequence();

public:
    static constexpr int kHeight = 80;
    static constexpr int kTrackHeaderWidth = 120;

private:
    ARASampleProjectAudioProcessorEditor* editorComponent;
    ARARegionSequence* regionSequence;

    class TrackHeaderView : public Component
    {
    public:
        TrackHeaderView (RegionSequenceView& owner);
        void paint (Graphics&) override;
    private:
        RegionSequenceView& owner;
    };

    TrackHeaderView trackHeaderView;
    OwnedArray<PlaybackRegionView> playbackRegionViews;
    bool isSelected = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceView)
};
