#pragma once

#include "juce_ARA_audio_plugin.h"
#include "juce_audio_processors/juce_audio_processors.h"

namespace juce
{
    class ARAAudioProcessor : public juce::AudioProcessor
    {
    public:
        ARAAudioProcessor() : AudioProcessor() {}
        ARAAudioProcessor (const BusesProperties& ioLayouts) : AudioProcessor(ioLayouts) {}
        ARAAudioProcessor (const std::initializer_list<const short[2]>& channelLayoutList) : AudioProcessor(channelLayoutList) {}

        virtual ~ARAAudioProcessor() {}

        const ARA::ARAPlugInExtensionInstance* createARAPlugInExtension(ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles);

        ARA::PlugIn::PlaybackRenderer* getARAPlaybackRenderer() const;
        ARA::PlugIn::EditorRenderer* getARAEditorRenderer() const;
        ARA::PlugIn::EditorView* getARAEditorView() const;

        bool isARAPlaybackRenderer() const { return getARAPlaybackRenderer() != nullptr; }
        bool isARAEditorRenderer() const { return getARAEditorRenderer() != nullptr; }
        bool isARAEditorView() const { return getARAEditorView() != nullptr; }

    private:
        std::unique_ptr<const ARA::PlugIn::PlugInExtension> araPlugInExtension;
    };
    
    //==============================================================================

    class ARAAudioProcessorEditor : public AudioProcessorEditor
    {
    public:
        /** Creates an editor for the specified processor. */
        ARAAudioProcessorEditor  (AudioProcessor& processor) noexcept : AudioProcessorEditor(processor) {}

        /** Creates an editor for the specified processor. */
        ARAAudioProcessorEditor  (AudioProcessor* processor) noexcept : AudioProcessorEditor(processor) {}

        ARA::PlugIn::EditorView* getARAEditorView() const;

        bool isARAEditorView() const { return getARAEditorView() != nullptr; }
    };
}
