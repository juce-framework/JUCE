#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

namespace juce
{

// ARARenderer draft begin
// common interface for ARAPlaybackRenderer and ARAEditorRenderer
//==============================================================================
/** Base class for a renderer fulfulling either the ARAPlaybackRenderer or the ARAEditorRenderer role.

    Instances of either subclass are constructed by the ARADocumentController.

    @tags{ARA}
*/
class JUCE_API  ARARenderer
{
public:
    //==============================================================================
    /**
        This structure is passed into an ARARenderer's prepareToPlay() method, and contains
        information about various aspects of the context in which it can expect to be called.
    */
    // ARARenderer draft note: same as juce::dsp::ProcessSpec. we could use that but it'd be the only reference to juce::dsp?
    struct JUCE_API ProcessSpec
    {
        /** The sample rate that will be used for the data that is sent to the renderer. */
        double sampleRate;

        /** The maximum number of samples that will be in the blocks sent to process() method. */
        // ARARenderer draft note: is uint32 in DSP, int in AudioBuffer and AudioProcessor
        int maximumBlockSize;

        /** The number of channels that the process() method will be expected to handle. */
        // ARARenderer draft note: is uint32 in DSP, int in AudioBuffer and AudioProcessor
        int numChannels;

        // ARARenderer draft note: the example has an extra parameter that we can either add here or keep there:
        //             useBufferedAudioSourceReader, which guarantees isNonRealtime is always true.

        // ARARenderer draft note: if adding 64 bit support:
        //bool doublePrecisionProcessing;
    };

    //==============================================================================
    /**
        Contains context information that is passed into an ARARenderer's processBlock() method.
    */
    // ARARenderer draft note: dsp has replacing and non-replacing, audio processor always is replacing
    //             replacing should be sufficient for ARA renderers - editor renderers the are no-ops by default
    // ARARenderer draft note: we keep the buffers external since we need separate virtual methods for rendering
    //             because the outer processor may be switched at runtime between 32 and 64 bt operation.
    //             this however makes this near-empty...
    struct JUCE_API ProcessContext
    {
        /** If set to true, then a processor's process() method is expected to do whatever
            is appropriate for it to be in a bypassed state.
        */
        // ARARenderer draft note: dsp has bypass here, but does this't really make sense for ARA renderers?
        //bool isBypassed;

        bool isNonRealtime;

        // ARARenderer draft note: playback renderers only really need song position and isPlaying,
        //             editor renderers maybe also ppqLoopStart, ppqLoopEnd and isLooping
        //             rather using individual iVars might be cleaner but might also require extra struct conversion..
        //             Also there is an API in VST3 these days to express this but it's not supported in JUCE yet.
        AudioPlayHead::CurrentPositionInfo& positionInfo;
    };

public:
    virtual ~ARARenderer() = default;

    //==============================================================================
    /** Initialises the renderer for playback. */
    // ARARenderer draft note: this is slightly different than the DSP modules: because the ARA renderers are
    //             long-living, we cannot simply discard them when not in use.
    // ARARenderer draft note: what about noexcept - in general for JUCE_ARA, where do we need to insert try/catch guards?
    virtual void prepareToPlay (const ProcessSpec&) {}

    /** Frees render ressources allocated in prepareToPlay(). */
    // ARARenderer draft note: this is slightly different than the DSP modules, see prepareToPlay().
    virtual void releaseResources() {}

    /** Resets the internal state variables of the renderer. */
    // ARARenderer draft note: in theory, playback renderers shouldn't need a reset() since they should be producing
    //             the same sample at each index each time. Practically there often are some filters with state,
    //             requiring at least a few samples warmup before being close (but not identical)
    //             for this, implementing a reset would at least provide deterministic results...
    //             Editor renderers however should implement reset so that host can clear buffers before bouncing.
    virtual void reset() {}

    //==============================================================================
    // ARARenderer draft note: AudioProcessor offers a MidiBuffer& here, which is not useful for playback
    //             renderers but might eventually be useful for editor renderers?
    // ARARenderer draft note: AudioProcessor is not noexcept, but that seems like an oversight?
    virtual bool processBlock (AudioBuffer<float>& buffer, const ProcessContext& context) noexcept = 0;

    // ARARenderer draft note: should we support 64 bit precision? If so, it would need to become a parameter
    //             of ProcessSpec, and we would either require clients to do both or offer a default
    //             implementation of the double case that falls back to float (this might need to
    //             allocate a buffer for conversion during prepareToPlay...?)
//  virtual bool processBlock (AudioBuffer<double>& buffer) = 0;
};
// ARARenderer draft end

//==============================================================================
/** Base class for a renderer fulfulling the ARAPlaybackRenderer role as described in the ARA SDK. 

    Instances of this class are constructed by the ARADocumentController.
    If you are subclassing ARAPlaybackRenderer, make sure to call the base class
    implementations of any overridden function, except for processBlock.

    @tags{ARA}
*/
class JUCE_API  ARAPlaybackRenderer   : public ARA::PlugIn::PlaybackRenderer,
                                        public ARARenderer
{
public:
    using ARA::PlugIn::PlaybackRenderer::PlaybackRenderer;

    // overloading inherited templated getters to default to juce versions of the returned classes
    template <typename DocumentController_t = ARADocumentController>
    DocumentController_t* getDocumentController() const noexcept { return ARA::PlugIn::PlaybackRenderer::getDocumentController<DocumentController_t>(); }
    template <typename PlaybackRegion_t = ARAPlaybackRegion>
    std::vector<PlaybackRegion_t*> const& getPlaybackRegions() const noexcept { return ARA::PlugIn::PlaybackRenderer::getPlaybackRegions<PlaybackRegion_t>(); }

// ARARenderer draft note: is this still needed? it shouldn't...
// TODO JUCE_ARA Should we keep this here for subclasses? There is also didBindToARA(), which
//               can provide this if needed. We could get rid of both renderer subclasses otherwise,
//               in fact we could even go further and hide the instance roles entirely behind
//               AudioProcessor(Editor)ARAExtension, which is maybe a good way reduce complexity?
//    void setAudioProcessor (AudioProcessor* processor) noexcept { audioProcessor = processor; }
//    AudioProcessor* getAudioProcessor() const noexcept { return audioProcessor; }

// TODO JUCE_ARA see definition of these in .cpp
//#if ARA_VALIDATE_API_CALLS
//    void addPlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept override;
//    void removePlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept override;
//#endif

private:
//    AudioProcessor* audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRenderer)
};

//==============================================================================
/** Base class for a renderer fulfulling the ARAEditorRenderer role as described in the ARA SDK. 
    
    Instances of this class are constructed by the ARADocumentController.
    If you are subclassing ARAEditorRenderer, make sure to call the base class
    implementations of any overridden function, except for processBlock.

    @tags{ARA}
*/
class JUCE_API  ARAEditorRenderer     : public ARA::PlugIn::EditorRenderer,
                                        public ARARenderer
{
public:
    using ARA::PlugIn::EditorRenderer::EditorRenderer;

    // overloading inherited templated getters to default to juce versions of the returned classes
    template <typename DocumentController_t = ARADocumentController>
    DocumentController_t* getDocumentController() const noexcept { return ARA::PlugIn::EditorRenderer::getDocumentController<DocumentController_t>(); }
    template <typename PlaybackRegion_t = ARAPlaybackRegion>
    std::vector<PlaybackRegion_t*> const& getPlaybackRegions() const noexcept { return ARA::PlugIn::EditorRenderer::getPlaybackRegions<PlaybackRegion_t>(); }
    template <typename RegionSequence_t = ARARegionSequence>
    std::vector<RegionSequence_t*> const& getRegionSequences() const noexcept { return ARA::PlugIn::EditorRenderer::getRegionSequences<RegionSequence_t>(); }

// ARARenderer draft note: is this still needed? it shouldn't...
//    void setAudioProcessor (AudioProcessor* processor) noexcept { audioProcessor = processor; }
//    AudioProcessor* getAudioProcessor() const noexcept { return audioProcessor; }

    // Per default, editor renderers will just let the signal pass through unaltered.
    // If you're overriding this to implement actual audio preview, remember to test
    // isNonRealtime of the process context - typically preview is limited to realtime!
    // ARARenderer draft note: should we even allow this rendering to happen offline?
    bool processBlock (AudioBuffer<float>&, const ProcessContext&) noexcept override { return true; }

private:
//    AudioProcessor* audioProcessor { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorRenderer)
};

//==============================================================================
/** Base class for a renderer fulfulling the ARAEditorView role as described in the ARA SDK. 
    
    Instances of this class are constructed by the ARADocumentController.
    If you are subclassing ARAEditorView, make sure to call the base class
    implementations of all overridden functions.
    
    @tags{ARA}
*/
class JUCE_API  ARAEditorView     : public ARA::PlugIn::EditorView
{
public:
    using ARA::PlugIn::EditorView::EditorView;

    // overloading inherited templated getters to default to juce versions of the returned classes
    template <typename DocumentController_t = ARADocumentController>
    DocumentController_t* getDocumentController() const noexcept { return ARA::PlugIn::EditorView::getDocumentController<DocumentController_t>(); }
    template <typename RegionSequence_t = ARARegionSequence>
    std::vector<RegionSequence_t*> const& getHiddenRegionSequences() const noexcept { return ARA::PlugIn::EditorView::getHiddenRegionSequences<RegionSequence_t>(); }

    // these must be called by subclass implementations to properly to forward listener notifications
    void doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept override;
    void doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept override;

    class JUCE_API  Listener
    {
    public:
        virtual ~Listener() = default;

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        /** Called when the editor view's selection changes.
            @param viewSelection The current selection state.
        */
        virtual void onNewSelection (const ARA::PlugIn::ViewSelection& viewSelection) {}

        /** Called when region sequences are flagged as hidden in the host UI.
            @param regionSequences A vector containing all hidden region sequences. 
        */
        virtual void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    /** \copydoc ARAListenableModelClass::addListener */
    void addListener (Listener* l);

    /** \copydoc ARAListenableModelClass::removeListener */
    void removeListener (Listener* l);

private:
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorView)
};

}
