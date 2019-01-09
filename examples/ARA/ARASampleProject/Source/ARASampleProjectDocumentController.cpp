#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectPlaybackRenderer.h"

ARASampleProjectDocumentController::ARASampleProjectDocumentController() noexcept
    : ARADocumentController(),
      audioSourceReadingThread (String (JucePlugin_Name) + " ARA Sample Reading Thread"),
      globalEditorSettings (Identifier (JucePlugin_Name "_GlobalEditorSettings"))
{
    audioSourceReadingThread.startThread();
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
