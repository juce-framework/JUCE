#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "RegionSequenceViewContainer.h"

//==============================================================================
/**
    PlaybackRegionView
    JUCE component used to display ARA playback regions
    along with their output waveform, name, color, and selection state
*/
class PlaybackRegionView  : public juce::Component,
                            public juce::SettableTooltipClient,
                            public juce::ChangeListener,
                            private juce::ARAEditorView::Listener,
                            private juce::ARADocument::Listener,
                            private juce::ARAAudioSource::Listener,
                            private juce::ARAAudioModification::Listener,
                            private juce::ARAPlaybackRegion::Listener
{
public:
    PlaybackRegionView (RegionSequenceViewContainer& viewContainer, juce::ARAPlaybackRegion* region);
    ~PlaybackRegionView() override;

    juce::ARAPlaybackRegion* getPlaybackRegion() const { return playbackRegion; }
    juce::Range<double> getTimeRange() const { return playbackRegion->getTimeRange(); }

    void updateBounds();

    void paint (juce::Graphics&) override;

    void mouseDoubleClick (const juce::MouseEvent& event) override;

    // ChangeListener overrides
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const juce::ARAViewSelection& viewSelection) override;

    // ARADocument::Listener overrides: used to check if our reader has been invalidated
    void didEndEditing (juce::ARADocument* document) override;

    // ARAAudioSource::Listener overrides
    void willEnableAudioSourceSamplesAccess (juce::ARAAudioSource* audioSource, bool enable) override;
    void didEnableAudioSourceSamplesAccess (juce::ARAAudioSource* audioSource, bool enable) override;
    void willUpdateAudioSourceProperties (juce::ARAAudioSource* audioSource, juce::ARAAudioSource::PropertiesPtr newProperties) override;

    // ARAAudioModification::Listener overrides
    void willUpdateAudioModificationProperties (juce::ARAAudioModification* audioModification, juce::ARAAudioModification::PropertiesPtr newProperties) override;

    // ARAPlaybackRegion::Listener overrides
    void willUpdatePlaybackRegionProperties (juce::ARAPlaybackRegion* playbackRegion, juce::ARAPlaybackRegion::PropertiesPtr newProperties) override;
    void didUpdatePlaybackRegionContent (juce::ARAPlaybackRegion* playbackRegion, juce::ARAContentUpdateScopes scopeFlags) override;

private:
    void destroyPlaybackRegionReader();
    void recreatePlaybackRegionReader();

private:

    // We're subclassing here only to provide a proper default c'tor for our shared resource
    class SharedAudioThumbnailCache   : public juce::AudioThumbnailCache
    {
        public:
            SharedAudioThumbnailCache()
                : AudioThumbnailCache (20000)
            {}
    };
    juce::SharedResourcePointer<SharedAudioThumbnailCache> sharedAudioThumbnailCache;

    RegionSequenceViewContainer& regionSequenceViewContainer;
    DocumentView& documentView;
    juce::ARAPlaybackRegion* playbackRegion;
    bool isSelected { false };

    juce::AudioThumbnail audioThumb;
    juce::ARAPlaybackRegionReader* playbackRegionReader { nullptr };  // careful: "weak" pointer, actual pointer is owned by our audioThumb

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaybackRegionView)
};
