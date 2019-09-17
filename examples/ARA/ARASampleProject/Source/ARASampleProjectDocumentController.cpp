#include "ARASampleProjectDocumentController.h"

ARASampleProjectDocumentController::ARASampleProjectDocumentController (const ARA::ARADocumentControllerHostInstance* instance) noexcept
    : ARADocumentController (instance),
      audioSourceReadingThread (String (JucePlugin_Name) + " ARA Sample Reading Thread")
{
    audioSourceReadingThread.startThread (7);   // above "default" priority so playback is fluent, but below realtime
}

//==============================================================================
// Hook defined by the ARA SDK to create custom subclass
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController (const ARA::ARADocumentControllerHostInstance* instance) noexcept
{
    return new ARASampleProjectDocumentController (instance);
}
