#pragma once

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
class RegionSequenceViewContainer : private juce::ARARegionSequence::Listener
{
public:
    RegionSequenceViewContainer (DocumentView& documentView, juce::ARARegionSequence* sequence);
    ~RegionSequenceViewContainer() override;

    juce::ARARegionSequence* getRegionSequence() const { return regionSequence; }     // careful: may return nullptr!
    juce::Range<double> getTimeRange() const { return (regionSequence != nullptr) ? regionSequence->getTimeRange() : juce::Range<double>(); }
    bool isEmpty() const { return (regionSequence == nullptr) || regionSequence->getPlaybackRegions().empty(); }

    void setRegionsViewBoundsByYRange (int y, int height);

    // ARARegionSequence::Listener overrides
    void willRemovePlaybackRegionFromRegionSequence (juce::ARARegionSequence* regionSequence, juce::ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (juce::ARARegionSequence* regionSequence, juce::ARAPlaybackRegion* playbackRegion) override;
    void willDestroyRegionSequence (juce::ARARegionSequence* regionSequence) override;
    void willUpdateRegionSequenceProperties (juce::ARARegionSequence* regionSequence, juce::ARARegionSequence::PropertiesPtr newProperties) override;

    DocumentView& getDocumentView() const { return documentView; }
    const RegionSequenceHeaderView& getRegionSequenceHeaderView() const { return regionSequenceHeaderView; }

private:
    void addRegionSequenceViewAndMakeVisible (juce::ARAPlaybackRegion* playbackRegion);
    void detachFromRegionSequence();

private:
    DocumentView& documentView;
    juce::ARARegionSequence* regionSequence;

    RegionSequenceHeaderView regionSequenceHeaderView;
    juce::OwnedArray<PlaybackRegionView> playbackRegionViews;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceViewContainer)
};
