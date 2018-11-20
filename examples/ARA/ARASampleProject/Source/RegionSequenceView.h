#pragma once

#include "JuceHeader.h"

class ARASampleProjectAudioProcessorEditor;

//==============================================================================
/** 
    RegionSequenceView
    JUCE component used to display ARA region sequences in a host document
    along with their name, order index, color, and selection state
*/
class RegionSequenceView: public Component, 
                          public juce::ChangeListener,
                          public ARADocument::Listener,               // Receives ARA region sequence update notifications
                          public ARARegionSequence::Listener,         // Receives ARA region sequence update notifications
                          public ARAPlaybackRegion::Listener         // Receives ARA playback region update notifications
{
public:
    ~RegionSequenceView();
    RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence);

    void paint (Graphics&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    void setIsSelected (bool value);
    bool getIsSelected() const;
    double getStartInSecs();
    double getLengthInSecs();

    ARARegionSequence* getRegionSequence() const { return regionSequence; }

    void doEndEditing (ARADocument* document) override;

    void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) override;
    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) override;

    // ARAPlaybackRegion::Listener overrides
    virtual void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) override;
    virtual void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) override;

private:
    String name;
    String orderIndex;
    Colour trackColour;
    bool isSelected;
    double startInSecs;

    ARASampleProjectAudioProcessorEditor* editorComponent;
    ARARegionSequence* regionSequence;

    enum { kAudioThumbHashCode = 1 };
    juce::AudioFormatManager audioFormatManger;
    juce::AudioThumbnailCache audioThumbCache;
    juce::AudioThumbnail audioThumb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceView)
};
