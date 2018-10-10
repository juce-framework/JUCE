// a juce::Component to show audio Thumbnail and some info from relevant ARA data..
#pragma once

#include "JuceHeader.h"

class AudioView
: public Component, public juce::ChangeListener
{
public:
    AudioView ();
    ~AudioView ();
    AudioView (ARA::PlugIn::RegionSequence&);

    void paint (Graphics&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    void isSelected (bool value);
    double getStartInSecs ();
    double getLengthInSecs ();

private:
    String name, order;
    Colour trackColour;
    bool _isSelected;
    double _startInSecs;

    juce::AudioFormatManager _audioFormatManger;
    juce::AudioThumbnailCache _audioThumbCache;
    juce::AudioThumbnail _audioThumb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioView)
};
