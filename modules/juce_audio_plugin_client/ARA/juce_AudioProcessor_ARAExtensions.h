#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

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
    roles as defined by the ARA SDK. Hosts can assign any subset of roles to each plugin instance.

    @tags{ARA}
*/
class JUCE_API  AudioProcessorARAExtension  : public ARA::PlugIn::PlugInExtension
{
public:
    using ARA::PlugIn::PlugInExtension::PlugInExtension;

    // overloading inherited templated getters to default to juce versions of the returned classes
    template <typename DocumentController_t = ARADocumentController>
    DocumentController_t* getDocumentController() const noexcept { return ARA::PlugIn::PlugInExtension::getDocumentController<DocumentController_t>(); }
    template <typename PlaybackRenderer_t = ARAPlaybackRenderer>
    PlaybackRenderer_t* getPlaybackRenderer() const noexcept { return ARA::PlugIn::PlugInExtension::getPlaybackRenderer<PlaybackRenderer_t>(); }
    template <typename EditorRenderer_t = ARAEditorRenderer>
    EditorRenderer_t* getEditorRenderer() const noexcept { return ARA::PlugIn::PlugInExtension::getEditorRenderer<EditorRenderer_t>(); }
    template <typename EditorView_t = ARAEditorView>
    EditorView_t* getEditorView() const noexcept { return ARA::PlugIn::PlugInExtension::getEditorView<EditorView_t>(); }

    /** Returns true if plugin instance fulfills the ARAPlaybackRenderer role. */
    bool isPlaybackRenderer() const noexcept { return ARA::PlugIn::PlugInExtension::getPlaybackRenderer() != nullptr; }

    /** Returns true if plugin instance fulfills the ARAEditorRenderer role. */
    bool isEditorRenderer() const noexcept { return ARA::PlugIn::PlugInExtension::getEditorRenderer() != nullptr; }

    /** Returns true if plugin instance fulfills the ARAEditorView role. */
    bool isEditorView() const noexcept { return ARA::PlugIn::PlugInExtension::getEditorView() != nullptr; }

protected:
    /** Optional hook for derived classes to perform any additional initialization that may be needed.
        If overriding this, make sure you call the base class implementation from your override. */
    virtual void didBindToARA() noexcept override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorARAExtension)
};

//==============================================================================
/** Extension class meant to be subclassed by the plugin's implementation of @see AudioProcessorEditor. 

    Subclassing AudioProcessorARAExtension allows access to the ARAEditorView instance role
    as described by the ARA SDK. 

    @tags{ARA}
*/
class JUCE_API  AudioProcessorEditorARAExtension
{
public:
    AudioProcessorEditorARAExtension (AudioProcessor* audioProcessor);
    virtual ~AudioProcessorEditorARAExtension();

    /** \copydoc AudioProcessorARAExtension::getARAEditorView */
    template<typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept { return (this->araProcessorExtension != nullptr) ? this->araProcessorExtension->getEditorView<EditorView_t>() : nullptr; }
    
    /** \copydoc AudioProcessorARAExtension::isARAEditorView */
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

private:
    AudioProcessorARAExtension* araProcessorExtension;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorEditorARAExtension)
};

} // namespace juce
