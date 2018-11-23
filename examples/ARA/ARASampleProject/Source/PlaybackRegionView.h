#pragma once

#include "JuceHeader.h"
#include "RegionSequenceView.h"

class PlaybackRegionView: public Component, 
                          public juce::ChangeListener,
                          public ARADocument::Listener
{
public:
    PlaybackRegionView (ARASampleProjectAudioProcessorEditor* editor, ARAPlaybackRegion* region);
    ~PlaybackRegionView();

    void paint (Graphics&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    ARAPlaybackRegion* getPlaybackRegion() const { return playbackRegion; }

    double getStartInSeconds() const;
    double getLengthInSeconds() const;
    double getEndInSeconds() const;

    void setIsSelected (bool value);

    // use this to check if our reader's been invalidated
    void doEndEditing (ARADocument* document) override;

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
