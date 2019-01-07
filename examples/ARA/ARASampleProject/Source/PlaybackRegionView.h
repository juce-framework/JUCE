#pragma once

#include "RegionSequenceView.h"

//==============================================================================
/**
    RegionSequenceView
    JUCE component used to display ARA playback regions
    along with their output waveform, name, color, and selection state
*/
class PlaybackRegionView    : public Component,
                              public ChangeListener,
                              private ARAEditorView::Listener,
                              private ARADocument::Listener,
                              private ARAAudioSource::Listener,
                              private ARAPlaybackRegion::Listener
{
public:
    PlaybackRegionView (DocumentView& documentView, ARAPlaybackRegion* region);
    ~PlaybackRegionView();

    ARAPlaybackRegion* getPlaybackRegion() const { return playbackRegion; }
    Range<double> getTimeRange() const { return playbackRegion->getTimeRange(); }

    void paint (Graphics&) override;

    // ChangeListener overrides
    void changeListenerCallback (ChangeBroadcaster*) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;

    // ARADocument::Listener overrides: used to check if our reader has been invalidated
    void didEndEditing (ARADocument* document) override;

    // ARAAudioSource::Listener overrides
    void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) override;

    // ARAPlaybackRegion::Listener overrides
    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) override;
    void didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags) override;

private:
    void recreatePlaybackRegionReader();

private:
    DocumentView& documentView;
    ARAPlaybackRegion* playbackRegion;
    ARAPlaybackRegionReader* playbackRegionReader = nullptr;  // careful: "weak" pointer, actual pointer is owned by our audioThumb
    bool isSelected = false;

    AudioThumbnailCache audioThumbCache;
    AudioThumbnail audioThumb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaybackRegionView)
};
