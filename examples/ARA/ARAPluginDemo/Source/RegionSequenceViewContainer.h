#pragma once

#include "JuceHeader.h"
#include "RegionSequenceHeaderView.h"

class DocumentView;
class PlaybackRegionView;

//==============================================================================
/**
    RegionSequenceViewContainer
    Class used to manage all views associated with an ARA region sequence.
    This includes a header view containing ARA region sequence data
    and views for all ARA playback regions in the given region sequence.
*/
class RegionSequenceViewContainer : private ARARegionSequence::Listener
{
public:
    RegionSequenceViewContainer (DocumentView& documentView, ARARegionSequence* sequence);
    ~RegionSequenceViewContainer();

    ARARegionSequence* getRegionSequence() const { return regionSequence; }     // careful: may return nullptr!
    Range<double> getTimeRange() const { return (regionSequence != nullptr) ? regionSequence->getTimeRange() : Range<double>(); }
    bool isEmpty() const { return (regionSequence == nullptr) || regionSequence->getPlaybackRegions().empty(); }

    void setRegionsViewBoundsByYRange (int y, int height);

    // ARARegionSequence::Listener overrides
    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) override;
    void willDestroyRegionSequence (ARARegionSequence* regionSequence) override;
    void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) override;

    DocumentView& getDocumentView() const { return documentView; }
    const RegionSequenceHeaderView& getRegionSequenceHeaderView() const { return regionSequenceHeaderView; }

private:
    void addRegionSequenceViewAndMakeVisible (ARAPlaybackRegion* playbackRegion);
    void detachFromRegionSequence();

private:
    DocumentView& documentView;
    ARARegionSequence* regionSequence;

    RegionSequenceHeaderView regionSequenceHeaderView;
    OwnedArray<PlaybackRegionView> playbackRegionViews;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceViewContainer)
};
