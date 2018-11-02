/*
  ==============================================================================

    PluginARAEditorView.h
    Created: 2 Nov 2018 3:08:37pm
    Author:  john

  ==============================================================================
*/

#pragma once

#include "AudioView.h"

/** Naive Editor class that visualize current ARA Document RegionSequences state */
class ARASampleProjectEditorView : public juce::Component, public ARA::PlugIn::EditorView
{
public:
    ARASampleProjectEditorView (ARA::PlugIn::DocumentController*) noexcept;
    void doNotifySelection (const ARA::PlugIn::ViewSelection*) noexcept override;
    void resized () override;

private:
    double maxRegionSequenceLength;
    juce::CriticalSection selectionLock;
    juce::OwnedArray <AudioView> regionSequenceViews;
};
