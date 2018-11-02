// a juce::Component to show audio Thumbnail and some info from relevant ARA data..
#pragma once

#include "JuceHeader.h"

class AudioView
: public Component, public juce::ChangeListener
{
public:
    AudioView();
    ~AudioView();
    AudioView (ARA::PlugIn::RegionSequence&);

    void paint (Graphics&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    void setIsSelected (bool value);
    double getStartInSecs();
    double getLengthInSecs();

private:
    String name, order;
    Colour trackColour;
    bool isSelected;
    double startInSecs;

    juce::AudioFormatManager audioFormatManger;
    juce::AudioThumbnailCache audioThumbCache;
    juce::AudioThumbnail audioThumb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioView)
};
