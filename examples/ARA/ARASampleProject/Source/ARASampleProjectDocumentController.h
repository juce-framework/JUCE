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
    ARASampleProjectDocumentController (const ARA::ARADocumentControllerHostInstance* instance) noexcept;

    // thread shared by renderers to read audio source samples ahead-of-time
    TimeSliceThread& getAudioSourceReadingThread() { return audioSourceReadingThread; }

private:
    // Thread used by buffering audio sources to read samples from the host
    TimeSliceThread audioSourceReadingThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectDocumentController)
};
