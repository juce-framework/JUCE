#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectPlaybackRenderer.h"

ARASampleProjectDocumentController::ARASampleProjectDocumentController() noexcept
: ARADocumentController()
{
    araAudioSourceReadingThread.reset (new TimeSliceThread (String (JucePlugin_Name) + " ARA Sample Reading Thread"));
    araAudioSourceReadingThread->startThread();
}

BufferingAudioReader* ARASampleProjectDocumentController::createBufferingAudioSourceReader (ARAAudioSource* audioSource, int bufferSize)
{
    return new BufferingAudioReader (createAudioSourceReader (audioSource), *araAudioSourceReadingThread.get(), bufferSize);
}

ARA::PlugIn::PlaybackRenderer* ARASampleProjectDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARASampleProjectPlaybackRenderer (this);
}

//==============================================================================
// Hook defined by the ARA SDK to create custom subclass
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController() noexcept
{
    return new ARASampleProjectDocumentController();
};
