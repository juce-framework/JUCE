/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

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
