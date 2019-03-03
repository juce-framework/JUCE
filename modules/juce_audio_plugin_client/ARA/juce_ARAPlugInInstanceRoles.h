#pragma once

#include "JuceHeader.h"

namespace juce
{

/** Shared base class for ARAPlaybackRenderer and ARAEditorRenderer, not to be used directly
*/
class ARARendererBase
{
public:
    /** Returns true if prepareToPlay has been called after construction or releasing resources. */
    bool isPrepared() const noexcept            { return prepared; }

    /** Returns the renderer sample rate as configured in prepareToPlay (or the default value of 44100.0). */
    double getSampleRate() const noexcept       { return sampleRate; }

    /** Returns the number of channels as configured in prepareToPlay (or the default value of 1). */
    int getNumChannels() const noexcept         { return numChannels; }

    /** Returns max samples per block as configured in prepareToPlay (or the default value of 1024). */
    int getMaxSamplesPerBlock() const noexcept  { return maxSamplesPerBlock; }

protected:
    /** Internal implementation helper, do not call directly. */
    void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock);

    /** Internal implementation helper, do not call directly. */
    void releaseResources();

private:
    double sampleRate = 44100.0;
    int numChannels = 1;
    int maxSamplesPerBlock = 1024;
    bool prepared = false;
};

//==============================================================================
/** Base class for a renderer fulfulling the ARAPlaybackRenderer role as described in the ARA SDK. 

    Instances of this class are constructed by the DocumentController. 
    If you are subclassing ARAPlaybackRenderer, make sure to call the base class
    implementations of any overridden function, except for processBlock.

    @tags{ARA}
*/
class ARAPlaybackRenderer     : public ARA::PlugIn::PlaybackRenderer,
                                public ARARendererBase
{
public:
    using ARA::PlugIn::PlaybackRenderer::PlaybackRenderer;

    /** Returns true if prepareToPlay has been called with \p (mayBeRealTime==true) */
    bool isPreparedForRealtime() const noexcept { return preparedForRealtime; }

    /** Must be called before any rendering is done via processBlock,
        to allow for allocating any ressources needed while processing.

        @param newSampleRate The desired sample rate.
        @param newNumChannels The desired channel count.
        @param newMaxSamplesPerBlock The desired max number of samples per block.
        @param mayBeRealtime Whether or not the renderer should be prepared to output samples in real time.

        Overrides of this method must call the inherited implementation!
    */
    virtual void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock, bool mayBeRealtime);

    /** Render a buffer of playback output samples, replacing the input samples.
        This default implementation just clears the provided buffer,
        so typically this method will be overridden to do something more useful.

        Generally this will be called from your plugin's @see AudioProcessor implementation,
        but playback renderers can be used to calculate the output of playback regions in any context
        provided they are configured and used properly - see for example ARAPlaybackRegionReader.

        @param buffer The buffer that the renderer will output samples in to.
        @param timeInSamples The current playback time, in samples.
        @param isPlayingBack Whether or not the host playhead is rolling.
        @param isNonRealtime Whether or not we're rendering in a non-realitme context.
    */
    virtual bool processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack, bool isNonRealtime);

    /** Called when no longer processing blocks, allowing the render to clean up unnecessary resources.

        Overrides of this method must call the inherited implementation!
    */
    virtual void releaseResources();

    /** Add a playback region to the renderer
        
        Only to be called if using a playback renderer created internally, i.e. not by the host.
    
        @param playbackRegion The playback region to add to the renderer instance. 
    */
    void addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;

    /** Remove a playback region from the renderer

        Only to be called if using a playback renderer created internally, i.e. not by the host.

        @param playbackRegion The playback region to remove from the renderer instance.
    */
    void removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;

private:
    bool preparedForRealtime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRenderer)
};

//==============================================================================
/** Base class for a renderer fulfulling the ARAEditorRenderer role as described in the ARA SDK. 
    
    Instances of this class are constructed by the DocumentController. 
    If you are subclassing ARAEditorRenderer, make sure to call the base class
    implementations of any overridden function, except for processBlock.

    @tags{ARA}
*/
class ARAEditorRenderer   : public ARA::PlugIn::EditorRenderer,
                            public ARARendererBase
{
public:
    using ARA::PlugIn::EditorRenderer::EditorRenderer;

    /** Must be called before any rendering is done via processBlock,
        to allow for allocating any ressources needed while processing.

        @param newSampleRate The desired sample rate.
        @param newNumChannels The desired channel count.
        @param newMaxSamplesPerBlock The desired max number of samples per block.

        Overrides of this method must call the inherited implementation!
    */
    virtual void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock);

    /** Render a buffer of editor signal output samples, adding to the input samples.
        This default implementation does nothing, i.e. just forwards the input.

        This will be called from your plugin's @see AudioProcessor implementation,
        and is typically only called for realtime playback (not offline bounces).

        @param buffer The buffer that the renderer will output samples in to.
        @param timeInSamples The current playback time, in samples.
        @param isPlayingBack Whether or not the host playhead is rolling.
    */
    virtual bool processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack);

    /** Called when no longer processing blocks, allowing the render to clean up unnecessary resources.

        Overrides of this method must call the inherited implementation!
    */
    virtual void releaseResources();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorRenderer)
};

//==============================================================================
/** Base class for a renderer fulfulling the ARAEditorView role as described in the ARA SDK. 
    
    Instances of this class are constructed by the DocumentController. 
    If you are subclassing ARAEditorView, make sure to call the base class
    implementations of all overridden functions.
    
    @tags{ARA}
*/
class ARAEditorView  : public ARA::PlugIn::EditorView
{
public:
    using ARA::PlugIn::EditorView::EditorView;

    // these must be called by subclass implementations to properly to forward listener notifications
    void doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept override;
    void doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept override;

    class Listener
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
