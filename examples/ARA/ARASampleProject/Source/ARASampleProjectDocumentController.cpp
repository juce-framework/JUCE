#include "ARASampleProjectDocumentController.h"

ARASampleProjectDocumentController::ARASampleProjectDocumentController() noexcept
    : ARADocumentController(),
      audioSourceReadingThread (String (JucePlugin_Name) + " ARA Sample Reading Thread")
{
    audioSourceReadingThread.startThread();
}

//==============================================================================
// Hook defined by the ARA SDK to create custom subclass
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController() noexcept
{
    return new ARASampleProjectDocumentController();
}
