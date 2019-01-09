#pragma once

#include "JuceHeader.h"

//==============================================================================
/**
    ARA DocumentController class for ARA sample project
    This is our plug-in's document controller implementation, which will
    be the central point of communication between the ARA host and our plug-in
*/
class ARASampleProjectDocumentController    : public ARADocumentController
{
public:
    ARASampleProjectDocumentController() noexcept;

    TimeSliceThread& getAudioSourceReadingThread() { return audioSourceReadingThread; }
    ValueTree&       getGlobalEditorSettings() { return globalEditorSettings; }
protected:
    // ARA class creation overrides
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;

private:
    // Thread used by buffering audio sources to read samples from the host
    TimeSliceThread audioSourceReadingThread;

    // Keeps global settings that should be preserved across all EditorViews.
    ValueTree globalEditorSettings;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectDocumentController)
};
