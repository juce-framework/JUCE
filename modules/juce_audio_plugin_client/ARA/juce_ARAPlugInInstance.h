#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{
    class ARAPlugInInstance
    {
    public:
        const ARA::ARAPlugInExtensionInstance* createARAPlugInExtension(ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles);

        ARA::PlugIn::PlaybackRenderer* getARAPlaybackRenderer() const noexcept;
        ARA::PlugIn::EditorRenderer* getARAEditorRenderer() const noexcept;
        ARA::PlugIn::EditorView* getARAEditorView() const noexcept;

        bool isARAPlaybackRenderer() const noexcept { return getARAPlaybackRenderer() != nullptr; }
        bool isARAEditorRenderer() const noexcept { return getARAEditorRenderer() != nullptr; }
        bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

    private:
        std::unique_ptr<const ARA::PlugIn::PlugInExtension> araPlugInExtension;
    };
    
    //==============================================================================

    class ARAPlugInEditor
    {
    public:
        ARAPlugInEditor(ARAPlugInInstance* araProcessor);

        ARA::PlugIn::EditorView* getARAEditorView() const noexcept;

        bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }
    
    private:
        ARAPlugInInstance* processor;
    };
}
