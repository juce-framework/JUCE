#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class AudioProcessor;
class ARAPlaybackRenderer;
class ARAEditorRenderer;
class ARAEditorView;
class ARADocumentController;

class AudioProcessorARAExtension
{
public:
    AudioProcessorARAExtension() = default;

    const ARA::ARAPlugInExtensionInstance* bindToARA (ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles);

    bool isBoundToARA() const noexcept { return araPlugInExtension != nullptr; }

    template<typename PlaybackRenderer_t = ARAPlaybackRenderer>
    PlaybackRenderer_t* getARAPlaybackRenderer() const noexcept { return araPlugInExtension ? static_cast<PlaybackRenderer_t*> (araPlugInExtension->getPlaybackRenderer()) : nullptr; }
    template<typename EditorRenderer_t = ARAEditorRenderer>
    EditorRenderer_t* getARAEditorRenderer() const noexcept { return araPlugInExtension ? static_cast<EditorRenderer_t*> (araPlugInExtension->getEditorRenderer()) : nullptr; }
    template<typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept { return araPlugInExtension ? static_cast<EditorView_t*> (araPlugInExtension->getEditorView()) : nullptr; }

    bool isARAPlaybackRenderer() const noexcept { return getARAPlaybackRenderer() != nullptr; }
    bool isARAEditorRenderer() const noexcept { return getARAEditorRenderer() != nullptr; }
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

    template<typename DocumentController_t = ARADocumentController>
    DocumentController_t* getARADocumentController() const noexcept { return araPlugInExtension ? static_cast<DocumentController_t*> (araPlugInExtension->getDocumentController()) : nullptr; }

private:
    std::unique_ptr<const ARA::PlugIn::PlugInExtension> araPlugInExtension;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorARAExtension)
};

class AudioProcessorEditorARAExtension
{
public:
    AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor);
    virtual ~AudioProcessorEditorARAExtension();

    template<typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept { return araProcessorExtension ? araProcessorExtension->getARAEditorView<EditorView_t>() : nullptr; }

    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

    template<typename DocumentController_t = ARADocumentController>
    DocumentController_t* getARADocumentController() const noexcept { return araProcessorExtension ? araProcessorExtension->getARADocumentController<DocumentController_t>() : nullptr; }

private:
    AudioProcessorARAExtension* araProcessorExtension;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorEditorARAExtension)
};

} // namespace juce
