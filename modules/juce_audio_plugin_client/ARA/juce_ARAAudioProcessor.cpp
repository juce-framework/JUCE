#pragma once

#include "juce_ARAAudioProcessor.h"

const ARA::ARAPlugInExtensionInstance* ARAAudioProcessor::createARAPlugInExtension (ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles)
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

ARA::PlugIn::PlaybackRenderer* ARAAudioProcessor::getARAPlaybackRenderer() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getPlaybackRenderer() : nullptr;
}

ARA::PlugIn::EditorRenderer* ARAAudioProcessor::getARAEditorRenderer() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getEditorRenderer() : nullptr;
}

ARA::PlugIn::EditorView* ARAAudioProcessor::getARAEditorView() const noexcept
{
    return araPlugInExtension ? araPlugInExtension->getEditorView() : nullptr;
}

//==============================================================================

ARA::PlugIn::EditorView* ARAAudioProcessorEditor::getARAEditorView() const noexcept
{
    return static_cast<ARAAudioProcessor*> (&processor)->getARAEditorView();
}
