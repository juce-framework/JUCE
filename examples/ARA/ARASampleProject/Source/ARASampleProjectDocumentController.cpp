#include "ARASampleProjectDocumentController.h"

//==============================================================================
// Hook defined by the ARA SDK to create custom subclass
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController (const ARA::ARADocumentControllerHostInstance* instance) noexcept
{
    return new ARASampleProjectDocumentController (instance);
}
