#pragma once

#include "juce_ARAPlugInInstance.h"

const ARA::ARAPlugInExtensionInstance* ARAPlugInInstance::createARAPlugInExtension (ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    ARA::PlugIn::DocumentController* documentController = reinterpret_cast<ARA::PlugIn::DocumentController*> (documentControllerRef);
    ARA_VALIDATE_API_ARGUMENT (documentControllerRef, ARA::PlugIn::DocumentController::isValidDocumentController (documentController));

    // verify this is only called once
    if (araPlugInExtension != nullptr)
    {
        ARA_VALIDATE_API_STATE (false && "binding already established");
        return nullptr;
    }

    araPlugInExtension.reset(documentController->createPlugInExtensionWithRoles (knownRoles, assignedRoles));
    return araPlugInExtension->getInstance ();
}

ARA::PlugIn::PlaybackRenderer* ARAPlugInInstance::getARAPlaybackRenderer() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getPlaybackRenderer() : nullptr;
}

ARA::PlugIn::EditorRenderer* ARAPlugInInstance::getARAEditorRenderer() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getEditorRenderer() : nullptr;
}

ARA::PlugIn::EditorView* ARAPlugInInstance::getARAEditorView() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getEditorView() : nullptr;
}

//==============================================================================

ARAPlugInEditor::ARAPlugInEditor(ARAPlugInInstance* araAudioProcessor)
: processor(araAudioProcessor)
{}

ARA::PlugIn::EditorView* ARAPlugInEditor::getARAEditorView() const noexcept
{
    return processor->getARAEditorView();
}
