#pragma once

#include "JuceHeader.h"
#include "TrackHeaderView.h"

class DocumentView;
class PlaybackRegionView;

//==============================================================================
/**
    RegionSequenceViewController
    Class used to manage all views associated with an ARA region sequence.
    This includes a track header view containing ARA region sequence data
    and views for all ARA playback regions in the given region sequence.
*/
class RegionSequenceViewController  : private ARARegionSequence::Listener
{
public:
    RegionSequenceViewController (DocumentView& documentView, ARARegionSequence* sequence);
    ~RegionSequenceViewController();

    ARARegionSequence* getRegionSequence() const { return regionSequence; }     // careful: may return nullptr!
    Range<double> getTimeRange() const { return (regionSequence != nullptr) ? regionSequence->getTimeRange() : Range<double>(); }
    bool isEmpty() const { return (regionSequence == nullptr) || regionSequence->getPlaybackRegions().empty(); }

    void setRegionsViewBoundsByYRange (int y, int height);

    // ARARegionSequence::Listener overrides
    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) override;
    void willDestroyRegionSequence (ARARegionSequence* regionSequence) override;
    void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) override;

private:
    void addRegionSequenceViewAndMakeVisible (ARAPlaybackRegion* playbackRegion);
    void detachFromRegionSequence();

private:
    DocumentView& documentView;
    ARARegionSequence* regionSequence;

    TrackHeaderView trackHeaderView;
    OwnedArray<PlaybackRegionView> playbackRegionViews;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceViewController)
};
