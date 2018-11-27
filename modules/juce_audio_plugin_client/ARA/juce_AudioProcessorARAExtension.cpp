#include "juce_AudioProcessorARAExtension.h"

namespace juce
{

const ARA::ARAPlugInExtensionInstance* AudioProcessorARAExtension::bindToARA (ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    ARA::PlugIn::DocumentController* documentController = reinterpret_cast<ARA::PlugIn::DocumentController*> (documentControllerRef);
    ARA_VALIDATE_API_ARGUMENT (documentControllerRef, ARA::PlugIn::DocumentController::isValidDocumentController (documentController));

    // verify this is only called once
    if (isBoundToARA())
    {
        ARA_VALIDATE_API_STATE (false && "binding already established");
        return nullptr;
    }

    araPlugInExtension.reset (documentController->createPlugInExtensionWithRoles (knownRoles, assignedRoles));
    return araPlugInExtension->getInstance();
}

ARAPlaybackRenderer* AudioProcessorARAExtension::getARAPlaybackRenderer() const noexcept
{
    return araPlugInExtension ? static_cast<ARAPlaybackRenderer*> (araPlugInExtension->getPlaybackRenderer()) : nullptr;
}

ARAEditorRenderer* AudioProcessorARAExtension::getARAEditorRenderer() const noexcept
{
    return araPlugInExtension ? static_cast<ARAEditorRenderer*> (araPlugInExtension->getEditorRenderer()) : nullptr;
}

ARAEditorView* AudioProcessorARAExtension::getARAEditorView() const noexcept
{
    return araPlugInExtension ? static_cast<ARAEditorView*> (araPlugInExtension->getEditorView()) : nullptr;
}

ARADocumentController* AudioProcessorARAExtension::getARADocumentController() const noexcept
{
    ARA::PlugIn::DocumentController* documentController = nullptr;
    if (isARAPlaybackRenderer())
        documentController = getARAPlaybackRenderer()->getDocumentController();
    else if (isARAEditorRenderer())
        documentController = getARAEditorRenderer()->getDocumentController();
    else if (isARAEditorView())
        documentController = getARAEditorView()->getDocumentController();
    return static_cast<ARADocumentController*> (documentController);
}

} // namespace juce
