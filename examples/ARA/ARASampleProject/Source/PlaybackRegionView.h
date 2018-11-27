#pragma once

#include "JuceHeader.h"
#include "RegionSequenceView.h"

class PlaybackRegionView: public Component, 
                          public ChangeListener,
                          public ARADocument::Listener,
                          public ARAAudioSource::Listener,
                          public ARAPlaybackRegion::Listener
{
public:
    PlaybackRegionView (ARASampleProjectAudioProcessorEditor* editor, ARAPlaybackRegion* region);
    ~PlaybackRegionView();

    // ChangeListener overrides
    void paint (Graphics&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    // ARADocument::Listener overrides: used to check if our reader has been invalidated
    void doEndEditing (ARADocument* document) override;

    // ARAAudioSource::Listener overrides: used to update when access to the audio source is enabled/disabled
    void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) override;

    // ARAPlaybackRegion::Listener overrides
    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) override;

    ARAPlaybackRegion* getPlaybackRegion() const { return playbackRegion; }

    void setIsSelected (bool value);
    bool getIsSelected () const { return isSelected; }

    double getStartInSeconds() const { return playbackRegion->getStartInPlaybackTime(); }
    double getLengthInSeconds() const { return playbackRegion->getDurationInPlaybackTime(); }
    double getEndInSeconds() const { return playbackRegion->getEndInPlaybackTime(); }

private:
    void recreatePlaybackRegionReader();

private:
    ARASampleProjectAudioProcessorEditor* editorComponent;
    ARAPlaybackRegion* playbackRegion;
    ARAPlaybackRegionReader* playbackRegionReader;  // careful: "weak" pointer, actual pointer is owned by our audioThumb
    bool isSelected;

    juce::AudioFormatManager audioFormatManger;
    juce::AudioThumbnailCache audioThumbCache;
    juce::AudioThumbnail audioThumb;
};
