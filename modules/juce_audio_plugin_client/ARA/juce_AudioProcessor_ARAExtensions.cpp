#include "juce_AudioProcessor_ARAExtensions.h"

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

// currently unused and disabled
//  if (isARAPlaybackRenderer())
//      getARAPlaybackRenderer()->setAudioProcessor (dynamic_cast<AudioProcessor*> (this));
//  if (isARAEditorRenderer())
//      getARAEditorRenderer()->setAudioProcessor (dynamic_cast<AudioProcessor*> (this));

    didBindToARA();

    return araPlugInExtension->getInstance();
}

//==============================================================================

AudioProcessorEditorARAExtension::AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor)
    : araProcessorExtension (dynamic_cast<AudioProcessorARAExtension*> (audioProcessor))
{
    if (isARAEditorView())
        getARAEditorView()->setEditorOpen (true);
}

AudioProcessorEditorARAExtension::~AudioProcessorEditorARAExtension()
{
    if (isARAEditorView())
        getARAEditorView()->setEditorOpen (false);
}

} // namespace juce
