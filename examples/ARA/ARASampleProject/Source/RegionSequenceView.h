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
                          public ARADocument::Listener,
                          public ARARegionSequence::Listener
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

private:
    void recreateRegionSequneceReader();

private:
    bool isSelected;
    double startInSecs;

    ARASampleProjectAudioProcessorEditor* editorComponent;
    ARARegionSequence* regionSequence;
    ARARegionSequenceReader* regionSequenceReader;

    enum { kAudioThumbHashCode = 1 };
    juce::AudioFormatManager audioFormatManger;
    juce::AudioThumbnailCache audioThumbCache;
    juce::AudioThumbnail audioThumb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceView)
};
