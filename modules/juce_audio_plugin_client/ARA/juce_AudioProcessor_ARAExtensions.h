#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class AudioProcessor;
class ARAPlaybackRenderer;
class ARAEditorRenderer;
class ARAEditorView;
class ARADocumentController;
//==============================================================================
/** Extension class meant to be subclassed by the plugin's implementation of @see AudioProcessor. 

    Subclassing AudioProcessorARAExtension allows access to the three possible plugin instance
    roles as defined by the ARA SDK. Note that a given plugin instance isn't required to fulfill
    more than one roles - the isX functions should be used before using getX directly. 

    @tags{ARA}
*/
class AudioProcessorARAExtension
{
public:
    AudioProcessorARAExtension() = default;

    /** Called by the ARA Companion SDK code to bind the plugin instance to an ARA document. */
    const ARA::ARAPlugInExtensionInstance* bindToARA (ARA::ARADocumentControllerRef documentControllerRef, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles);

    /** Returns true if this plugin instance is bound to an ARA document. */
    bool isBoundToARA() const noexcept { return araPlugInExtension != nullptr; }

    /** Return the ARAPlaybackRenderer instance, if it exists. */
    template<typename PlaybackRenderer_t = ARAPlaybackRenderer>
    PlaybackRenderer_t* getARAPlaybackRenderer() const noexcept { return araPlugInExtension ? static_cast<PlaybackRenderer_t*> (araPlugInExtension->getPlaybackRenderer()) : nullptr; }
    /** Return the ARAEditorRenderer instance, if it exists. */
    template<typename EditorRenderer_t = ARAEditorRenderer>
    EditorRenderer_t* getARAEditorRenderer() const noexcept { return araPlugInExtension ? static_cast<EditorRenderer_t*> (araPlugInExtension->getEditorRenderer()) : nullptr; }
    /** Return the ARAEditorView instance, if it exists. */
    template<typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept { return araPlugInExtension ? static_cast<EditorView_t*> (araPlugInExtension->getEditorView()) : nullptr; }

    /** Returns true if plugin instance fulfills the ARAPlaybackRenderer role. */
    bool isARAPlaybackRenderer() const noexcept { return getARAPlaybackRenderer() != nullptr; }
    /** Returns true if plugin instance fulfills the ARAEditorRenderer role. */
    bool isARAEditorRenderer() const noexcept { return getARAEditorRenderer() != nullptr; }
    /** Returns true if plugin instance fulfills the ARAEditorView role. */
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

    /** Returns the ARA document controller (provided the plugin instance is bound to ARA). */
    template<typename DocumentController_t = ARADocumentController>
    DocumentController_t* getARADocumentController() const noexcept { return araPlugInExtension ? static_cast<DocumentController_t*> (araPlugInExtension->getDocumentController()) : nullptr; }

private:
    std::unique_ptr<const ARA::PlugIn::PlugInExtension> araPlugInExtension;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorARAExtension)
};
//==============================================================================
/** Extension class meant to be subclassed by the plugin's implementation of @see AudioProcessorEditor. 

    Subclassing AudioProcessorARAExtension allows access to the ARAEditorView instance role
    as described by the ARA SDK. 

    @tags{ARA}
*/
class AudioProcessorEditorARAExtension
{
public:
    AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor);
    virtual ~AudioProcessorEditorARAExtension();

    /** \copydoc AudioProcessorARAExtension::getARAEditorView */
    template<typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept { return araProcessorExtension ? araProcessorExtension->getARAEditorView<EditorView_t>() : nullptr; }
    
    /** \copydoc AudioProcessorARAExtension::isARAEditorView */
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

    /** \copydoc AudioProcessorARAExtension::getARADocumentController */
    template<typename DocumentController_t = ARADocumentController>
    DocumentController_t* getARADocumentController() const noexcept { return araProcessorExtension ? araProcessorExtension->getARADocumentController<DocumentController_t>() : nullptr; }

private:
    AudioProcessorARAExtension* araProcessorExtension;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorEditorARAExtension)
};

} // namespace juce
