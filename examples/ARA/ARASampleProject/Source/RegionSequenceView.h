#pragma once

#include "JuceHeader.h"

//==============================================================================
/** 
    RegionSequenceView
    JUCE component used to display ARA region sequences in a host document
    along with their name, order index, color, and selection state
*/
class RegionSequenceView: public Component, 
                          public juce::ChangeListener
{
public:
    ~RegionSequenceView();
    RegionSequenceView (ARARegionSequence* sequence);

    void paint (Graphics&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    void setIsSelected (bool value);
    bool getIsSelected() const;
    double getStartInSecs();
    double getLengthInSecs();

    ARA::PlugIn::RegionSequence* getRegionSequence() const { return regionSequence; }

private:
    String name;
    String orderIndex;
    Colour trackColour;
    bool isSelected;
    double startInSecs;

    ARARegionSequence* regionSequence;

    juce::AudioFormatManager audioFormatManger;
    juce::AudioThumbnailCache audioThumbCache;
    juce::AudioThumbnail audioThumb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceView)
};
