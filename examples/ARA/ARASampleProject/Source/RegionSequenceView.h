#pragma once

#include "JuceHeader.h"
#include "TrackHeaderView.h"

class DocumentView;
class PlaybackRegionView;

//==============================================================================
/**
    RegionSequenceView
    JUCE component used to display all ARA playback regions in a region sequences
*/
// TODO JUCE_ARA this is no longer a view, but rather some container/controller for all views
//               associated with a given region sequence - must rename accordingly.
class RegionSequenceView  : private ARARegionSequence::Listener
{
public:
    RegionSequenceView (DocumentView& documentView, ARARegionSequence* sequence);
    ~RegionSequenceView();

    ARARegionSequence* getRegionSequence() const { return regionSequence; }     // careful: may return nullptr!
    Range<double> getTimeRange() const { return (regionSequence != nullptr) ? regionSequence->getTimeRange() : Range<double>(); }
    bool isEmpty() const { return (regionSequence == nullptr) || regionSequence->getPlaybackRegions().empty(); }

    void setRegionsViewBoundsByYRange (int y, int height);

    // ARARegionSequence::Listener overrides
    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion) override;
    void willDestroyRegionSequence (ARARegionSequence* sequence) override;
    void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) override;

private:
    void addRegionSequenceViewAndMakeVisible (ARAPlaybackRegion* playbackRegion);
    void detachFromRegionSequence();

private:
    DocumentView& documentView;
    ARARegionSequence* regionSequence;

    TrackHeaderView trackHeaderView;
    OwnedArray<PlaybackRegionView> playbackRegionViews;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceView)
};
