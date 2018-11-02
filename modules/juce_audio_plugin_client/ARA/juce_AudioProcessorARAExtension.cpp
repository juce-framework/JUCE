#pragma once

#include "juce_AudioProcessorARAExtension.h"

const ARA::ARAPlugInExtensionInstance* AudioProcessorARAExtension::createARAPlugInExtension (ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    ARA::PlugIn::DocumentController* documentController = reinterpret_cast<ARA::PlugIn::DocumentController*> (documentControllerRef);
    ARA_VALIDATE_API_ARGUMENT (documentControllerRef, ARA::PlugIn::DocumentController::isValidDocumentController (documentController));

    // verify this is only called once
    if (araPlugInExtension != nullptr)
    {
        ARA_VALIDATE_API_STATE (false && "binding already established");
        return nullptr;
    }

    araPlugInExtension.reset (documentController->createPlugInExtensionWithRoles (knownRoles, assignedRoles));
    return araPlugInExtension->getInstance();
}

ARA::PlugIn::PlaybackRenderer* AudioProcessorARAExtension::getARAPlaybackRenderer() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getPlaybackRenderer() : nullptr;
}

ARA::PlugIn::EditorRenderer* AudioProcessorARAExtension::getARAEditorRenderer() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getEditorRenderer() : nullptr;
}

ARA::PlugIn::EditorView* AudioProcessorARAExtension::getARAEditorView() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getEditorView() : nullptr;
}
