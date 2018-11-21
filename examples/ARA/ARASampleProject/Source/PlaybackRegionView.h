#pragma once

#include "JuceHeader.h"
#include "RegionSequenceView.h"

class PlaybackRegionView: public Component, 
                          public juce::ChangeListener,
                          public ARADocument::Listener
{
public:
    PlaybackRegionView (ARASampleProjectAudioProcessorEditor* editor, ARAPlaybackRegion* region);
    ~PlaybackRegionView ();

    void paint (Graphics&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    void setIsSelected (bool value);
    bool getIsSelected () const;
    double getStartInSeconds ();
    double getEndInSeconds ();
    double getLengthInSeconds ();
    ARAPlaybackRegion* getPlaybackRegion () const { return playbackRegion; }

    // use this to check if our reader's been invalidated
    void doEndEditing (ARADocument* document) override;

private:
    void recreatePlaybackRegionReader ();

private:
    bool isSelected;
    ARASampleProjectAudioProcessorEditor* editorComponent;
    ARAPlaybackRegion* playbackRegion;
    ARAPlaybackRegionReader* playbackRegionReader;  // careful: "weak" pointer, actual pointer is owned by our audioThumb

    enum { kAudioThumbHashCode = 1 };
    juce::AudioFormatManager audioFormatManger;
    juce::AudioThumbnailCache audioThumbCache;
    juce::AudioThumbnail audioThumb;
};