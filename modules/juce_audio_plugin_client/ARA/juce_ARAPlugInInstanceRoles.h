#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

namespace juce
{

//==============================================================================
/** Base class for a renderer fulfulling either the ARAPlaybackRenderer or the ARAEditorRenderer role.

    Instances of either subclass are constructed by the ARADocumentController.

    @tags{ARA}
*/
class JUCE_API  ARARenderer
{
public:
    virtual ~ARARenderer() = default;

    /** Initialises the renderer for playback.
        @param sampleRate               The sample rate that will be used for the data that is sent to the renderer.
        @param maximumSamplesPerBlock   The maximum number of samples that will be in the blocks sent to process() method.
        @param numChannels              The number of channels that the process() method will be expected to handle.
        @param alwaysNonRealtime        True if this renderer is never used in realtime (e.g. if providing data for views only).
    */
    // TODO JUCE_ARA if adding 64 bit support, we need to add bool doublePrecisionProcessing here.
    virtual void prepareToPlay (double sampleRate, int maximumSamplesPerBlock, int numChannels, bool alwaysNonRealtime = false) { ignoreUnused (sampleRate, maximumSamplesPerBlock, numChannels, alwaysNonRealtime); }

    /** Frees render resources allocated in prepareToPlay(). */
    virtual void releaseResources() {}

    /** Resets the internal state variables of the renderer. */
    virtual void reset() {}

    /** Renders the output into the given buffer. Returns true if rendering executed without error, false otherwise.
        @param buffer           The output buffer for the rendering. ARAPlaybackRenderers will replace the sample data,
                                while ARAEditorRenderer will add to it.
        @param isNonRealtime    Indicate whether the call is executed under real time constraints.
                                This may toggle upon each call, and if true might cause the rendering to fail
                                if required data such as audio source samples could not be obtained in time.
        @param positionInfo     Current song position, playback state and playback loop location.
                                There should be no need to access the bpm, timeSig and ppqPosition members in any
                                ARA renderer since ARA provides that information with random access in its model graph.
    */
    // TODO JUCE_ARA There is an API in VST3 now to skip costly calculations for those members of positionInfo
    //               which we do not need in ARA, but this is not yet supported in JUCE...
    virtual bool processBlock (AudioBuffer<float>& buffer, bool isNonRealtime, const AudioPlayHead::CurrentPositionInfo& positionInfo) noexcept = 0;
    
    // TODO JUCE_ARA: should we support 64 bit precision? If so, we should require clients to do both
    //                variants or offer a default implementation of for double that falls back to float
    //                (this might need to allocate a buffer for conversion during prepareToPlay...)?
//  virtual bool processBlock (AudioBuffer<double>& buffer, bool isNonRealtime, const AudioPlayHead::CurrentPositionInfo& positionInfo) = 0;
};

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

#if ARA_VALIDATE_API_CALLS
    void addPlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept override;
    void removePlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept override;
    AudioProcessorARAExtension* araExtension {};
#endif

private:
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

    // Per default, editor renderers will just let the signal pass through unaltered.
    // If you're overriding this to implement actual audio preview, remember to test
    // isNonRealtime of the process context - typically preview is limited to realtime!
    bool processBlock (AudioBuffer<float>& /*buffer*/, bool /*isNonRealtime*/, const AudioPlayHead::CurrentPositionInfo& /*positionInfo*/) noexcept override { return true; }

private:
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
