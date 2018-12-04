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

    ARAPlaybackRenderer* getARAPlaybackRenderer() const noexcept;
    ARAEditorRenderer* getARAEditorRenderer() const noexcept;
    ARAEditorView* getARAEditorView() const noexcept;

    bool isARAPlaybackRenderer() const noexcept { return getARAPlaybackRenderer() != nullptr; }
    bool isARAEditorRenderer() const noexcept { return getARAEditorRenderer() != nullptr; }
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

    ARADocumentController* getARADocumentController() const noexcept;

private:
    std::unique_ptr<const ARA::PlugIn::PlugInExtension> araPlugInExtension;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorARAExtension)
};

class AudioProcessorEditorARAExtension
{
public:
    AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor);
    virtual ~AudioProcessorEditorARAExtension();

    ARAEditorView* getARAEditorView() const noexcept;

    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

    ARADocumentController* getARADocumentController() const noexcept;

private:
    AudioProcessorARAExtension* araProcessorExtension;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorEditorARAExtension)
};

} // namespace juce
