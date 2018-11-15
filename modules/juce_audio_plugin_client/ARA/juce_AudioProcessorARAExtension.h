#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{
    
class ARAPlaybackRenderer;
class ARAEditorRenderer;
class ARAEditorView;

class AudioProcessorARAExtension
{
public:
    const ARA::ARAPlugInExtensionInstance* createARAPlugInExtension (ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles);

    ARAPlaybackRenderer* getARAPlaybackRenderer() const noexcept;
    ARAEditorRenderer* getARAEditorRenderer() const noexcept;
    ARAEditorView* getARAEditorView() const noexcept;

    bool isARAPlaybackRenderer() const noexcept { return getARAPlaybackRenderer() != nullptr; }
    bool isARAEditorRenderer() const noexcept { return getARAEditorRenderer() != nullptr; }
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

private:
    std::unique_ptr<const ARA::PlugIn::PlugInExtension> araPlugInExtension;
};

} // namespace juce
