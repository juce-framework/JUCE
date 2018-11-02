/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#pragma once

#include "AudioView.h"

class ARASampleProjectDocumentController : public juce::ARADocumentController
{
public:
    ARASampleProjectDocumentController() noexcept;
    ARA::PlugIn::EditorView* doCreateEditorView() noexcept override;
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;

private:
    // Thread used by buffering audio sources to read samples from the host
    std::unique_ptr<TimeSliceThread> araAudioSourceReadingThread;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectDocumentController)
};

/** Naive Editor class that visualize current ARA Document RegionSequences state */
class ARASampleProjectEditor : public juce::Component, public ARA::PlugIn::EditorView
{
public:
    ARASampleProjectEditor (ARA::PlugIn::DocumentController*) noexcept;
    void doNotifySelection (const ARA::PlugIn::ViewSelection*) noexcept override;
    void resized () override;

private:
    double maxRegionSequenceLength;
    juce::CriticalSection selectionLock;
    juce::OwnedArray <AudioView> regionSequenceViews;
};
