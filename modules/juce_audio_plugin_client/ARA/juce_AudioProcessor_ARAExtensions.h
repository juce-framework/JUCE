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
    AudioProcessorARAExtension() = default;

    /** Query whether last call to processBlock() was successful.
        TODO JUCE_ARA AudioProcessor::processBlock() should rather return a bool
    */
    virtual bool didProcessBlockSucceed() const noexcept = 0;

    /** Return the ARAPlaybackRenderer instance, if plugin instance fulfills the ARAPlaybackRenderer role. */
    template<typename PlaybackRenderer_t = ARAPlaybackRenderer>
    PlaybackRenderer_t* getARAPlaybackRenderer() const noexcept { return static_cast<PlaybackRenderer_t*> (this->getPlaybackRenderer()); }

    /** Return the ARAEditorRenderer instance, if plugin instance fulfills the ARAEditorRenderer role. */
    template<typename EditorRenderer_t = ARAEditorRenderer>
    EditorRenderer_t* getARAEditorRenderer() const noexcept { return static_cast<EditorRenderer_t*> (this->getEditorRenderer()); }

    /** Return the ARAEditorView instance, if plugin instance fulfills the ARAEditorView role. */
    template<typename EditorView_t = ARAEditorView>
    EditorView_t* getARAEditorView() const noexcept { return static_cast<EditorView_t*> (this->getEditorView()); }

    /** Returns true if plugin instance fulfills the ARAPlaybackRenderer role. */
    bool isARAPlaybackRenderer() const noexcept { return getARAPlaybackRenderer() != nullptr; }

    /** Returns true if plugin instance fulfills the ARAEditorRenderer role. */
    bool isARAEditorRenderer() const noexcept { return getARAEditorRenderer() != nullptr; }

    /** Returns true if plugin instance fulfills the ARAEditorView role. */
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

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
    EditorView_t* getARAEditorView() const noexcept { return this->araProcessorExtension != nullptr ? this->araProcessorExtension->getARAEditorView<EditorView_t>() : nullptr; }
    
    /** \copydoc AudioProcessorARAExtension::isARAEditorView */
    bool isARAEditorView() const noexcept { return getARAEditorView() != nullptr; }

private:
    AudioProcessorARAExtension* araProcessorExtension;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorEditorARAExtension)
};

} // namespace juce
