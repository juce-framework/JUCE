#pragma once

#include "JuceHeader.h"
#include "RegionSequenceViewContainer.h"

//==============================================================================
/**
    PlaybackRegionView
    JUCE component used to display ARA playback regions
    along with their output waveform, name, color, and selection state
*/
class PlaybackRegionView  : public Component,
                            public ChangeListener,
                            private ARAEditorView::Listener,
                            private ARADocument::Listener,
                            private ARAAudioSource::Listener,
                            private ARAAudioModification::Listener,
                            private ARAPlaybackRegion::Listener
{
public:
    PlaybackRegionView (RegionSequenceViewContainer& viewContainer, ARAPlaybackRegion* region);
    ~PlaybackRegionView();

    ARAPlaybackRegion* getPlaybackRegion() const { return playbackRegion; }
    Range<double> getTimeRange() const { return playbackRegion->getTimeRange(); }

    void updateBounds();

    void paint (Graphics&) override;

    void mouseDoubleClick (const MouseEvent& event) override;

    // ChangeListener overrides
    void changeListenerCallback (ChangeBroadcaster*) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARAViewSelection& viewSelection) override;

    // ARADocument::Listener overrides: used to check if our reader has been invalidated
    void didEndEditing (ARADocument* document) override;

    // ARAAudioSource::Listener overrides
    void willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) override;
    void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) override;
    void willUpdateAudioSourceProperties (ARAAudioSource* audioSource, ARAAudioSource::PropertiesPtr newProperties) override;

    // ARAAudioModification::Listener overrides
    void willUpdateAudioModificationProperties (ARAAudioModification* audioModification, ARAAudioModification::PropertiesPtr newProperties) override;

    // ARAPlaybackRegion::Listener overrides
    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) override;
    void didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags) override;

private:
    void destroyPlaybackRegionReader();
    void recreatePlaybackRegionReader();

private:

    // We're subclassing here only to provide a proper default c'tor for our shared ressource
    class SharedAudioThumbnailCache   : public AudioThumbnailCache
    {
        public:
            SharedAudioThumbnailCache()
                : AudioThumbnailCache (20000)
            {}
    };
    SharedResourcePointer<SharedAudioThumbnailCache> sharedAudioThumbnailCache;

    RegionSequenceViewContainer& regionSequenceViewContainer;
    DocumentView& documentView;
    ARAPlaybackRegion* playbackRegion;
    bool isSelected { false };

    AudioThumbnail audioThumb;
    ARAPlaybackRegionReader* playbackRegionReader { nullptr };  // careful: "weak" pointer, actual pointer is owned by our audioThumb

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaybackRegionView)
};
