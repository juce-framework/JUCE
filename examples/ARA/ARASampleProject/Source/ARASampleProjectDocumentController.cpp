#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectPlaybackRenderer.h"

ARASampleProjectDocumentController::ARASampleProjectDocumentController() noexcept
: juce::ARADocumentController()
{
    araAudioSourceReadingThread.reset (new TimeSliceThread (String (JucePlugin_Name) + " ARA Sample Reading Thread"));
    araAudioSourceReadingThread->startThread();
}

BufferingAudioSource* ARASampleProjectDocumentController::createBufferingAudioSourceReader (ARAAudioSource* audioSource, int bufferSize)
{
    return new BufferingAudioSource (new AudioFormatReaderSource (createAudioSourceReader (audioSource), true), *araAudioSourceReadingThread.get(), true, bufferSize, audioSource->getChannelCount());
}

// return an instance of our playback renderer implementation
ARA::PlugIn::PlaybackRenderer* ARASampleProjectDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARASampleProjectPlaybackRenderer (this);
}

//==============================================================================
// This creates new instances of the document controller..
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController() noexcept
{
    return new ARASampleProjectDocumentController();
};
