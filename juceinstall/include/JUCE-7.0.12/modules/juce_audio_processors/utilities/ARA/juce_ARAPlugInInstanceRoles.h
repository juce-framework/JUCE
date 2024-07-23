/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

namespace juce
{

//==============================================================================
/** Base class for a renderer fulfilling either the ARAPlaybackRenderer or the ARAEditorRenderer role.

    Instances of either subclass are constructed by the DocumentController.

    @tags{ARA}
*/
class JUCE_API  ARARenderer
{
public:
    enum class AlwaysNonRealtime { no, yes };

    virtual ~ARARenderer() = default;

    /** Initialises the renderer for playback.

        @param sampleRate               The sample rate that will be used for the data that is sent
                                        to the renderer
        @param maximumSamplesPerBlock   The maximum number of samples that will be in the blocks
                                        sent to process() method
        @param numChannels              The number of channels that the process() method will be
                                        expected to handle
        @param precision                This should be the same as the result of getProcessingPrecision()
                                        for the enclosing AudioProcessor
        @param alwaysNonRealtime        yes if this renderer is never used in realtime (e.g. if
                                        providing data for views only)
    */
    virtual void prepareToPlay (double sampleRate,
                                int maximumSamplesPerBlock,
                                int numChannels,
                                AudioProcessor::ProcessingPrecision precision,
                                AlwaysNonRealtime alwaysNonRealtime = AlwaysNonRealtime::no);

    /** Frees render resources allocated in prepareToPlay(). */
    virtual void releaseResources() {}

    /** Resets the internal state variables of the renderer. */
    virtual void reset() {}

    /** Renders the output into the given buffer. Returns true if rendering executed without error,
        false otherwise.

        @param buffer           The output buffer for the rendering. ARAPlaybackRenderers will
                                replace the sample data, while ARAEditorRenderer will add to it.
        @param realtime         Indicates whether the call is executed under real time constraints.
                                The value of this parameter may change from one call to the next,
                                and if the value is yes, the rendering may fail if the required
                                samples cannot be obtained in time.
        @param positionInfo     Current song position, playback state and playback loop location.
                                There should be no need to access the bpm, timeSig and ppqPosition
                                members in any ARA renderer since ARA provides that information with
                                random access in its model graph.

        Returns false if non-ARA fallback rendering is required and true otherwise.
    */
    virtual bool processBlock (AudioBuffer<float>& buffer,
                               AudioProcessor::Realtime realtime,
                               const AudioPlayHead::PositionInfo& positionInfo) noexcept = 0;

    /** Renders the output into the given buffer. Returns true if rendering executed without error,
        false otherwise.

        @param buffer           The output buffer for the rendering. ARAPlaybackRenderers will
                                replace the sample data, while ARAEditorRenderer will add to it.
        @param realtime         Indicates whether the call is executed under real time constraints.
                                The value of this parameter may change from one call to the next,
                                and if the value is yes, the rendering may fail if the required
                                samples cannot be obtained in time.
        @param positionInfo     Current song position, playback state and playback loop location.
                                There should be no need to access the bpm, timeSig and ppqPosition
                                members in any ARA renderer since ARA provides that information with
                                random access in its model graph.

        Returns false if non-ARA fallback rendering is required and true otherwise.
    */
    virtual bool processBlock (AudioBuffer<double>& buffer,
                               AudioProcessor::Realtime realtime,
                               const AudioPlayHead::PositionInfo& positionInfo) noexcept;
};

//==============================================================================
/** Base class for a renderer fulfilling the ARAPlaybackRenderer role as described in the ARA SDK.

    Instances of this class are constructed by the DocumentController. If you are subclassing
    ARAPlaybackRenderer, make sure to call the base class implementation of any overridden function,
    except for processBlock.

    @tags{ARA}
*/
class JUCE_API  ARAPlaybackRenderer  : public ARA::PlugIn::PlaybackRenderer,
                                       public ARARenderer
{
public:
    using ARA::PlugIn::PlaybackRenderer::PlaybackRenderer;

    bool processBlock (AudioBuffer<float>& buffer,
                       AudioProcessor::Realtime realtime,
                       const AudioPlayHead::PositionInfo& positionInfo) noexcept override;

    using ARARenderer::processBlock;

    // Shadowing templated getters to default to JUCE versions of the returned classes
    /** Returns the PlaybackRegions
     *
     * @tparam PlaybackRegion_t
     * @return
     */
    template <typename PlaybackRegion_t = ARAPlaybackRegion>
    std::vector<PlaybackRegion_t*> const& getPlaybackRegions() const noexcept
    {
        return ARA::PlugIn::PlaybackRenderer::getPlaybackRegions<PlaybackRegion_t>();
    }

#if ARA_VALIDATE_API_CALLS
    void addPlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept override;
    void removePlaybackRegion (ARA::ARAPlaybackRegionRef playbackRegionRef) noexcept override;
    AudioProcessorARAExtension* araExtension {};
#endif

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRenderer)
};

//==============================================================================
/** Base class for a renderer fulfilling the ARAEditorRenderer role as described in the ARA SDK.

    Instances of this class are constructed by the DocumentController. If you are subclassing
    ARAEditorRenderer, make sure to call the base class implementation of any overridden function,
    except for processBlock.

    @tags{ARA}
*/
class JUCE_API  ARAEditorRenderer  : public ARA::PlugIn::EditorRenderer,
                                     public ARARenderer
{
public:
    using ARA::PlugIn::EditorRenderer::EditorRenderer;

    // Shadowing templated getters to default to JUCE versions of the returned classes
    template <typename PlaybackRegion_t = ARAPlaybackRegion>
    std::vector<PlaybackRegion_t*> const& getPlaybackRegions() const noexcept
    {
        return ARA::PlugIn::EditorRenderer::getPlaybackRegions<PlaybackRegion_t>();
    }

    template <typename RegionSequence_t = ARARegionSequence>
    std::vector<RegionSequence_t*> const& getRegionSequences() const noexcept
    {
        return ARA::PlugIn::EditorRenderer::getRegionSequences<RegionSequence_t>();
    }

    // By default, editor renderers will just let the signal pass through unaltered.
    // If you're overriding this to implement actual audio preview, remember to check
    // isNonRealtime of the process context - typically preview is limited to realtime.
    bool processBlock (AudioBuffer<float>& buffer,
                       AudioProcessor::Realtime isNonRealtime,
                       const AudioPlayHead::PositionInfo& positionInfo) noexcept override;

    using ARARenderer::processBlock;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorRenderer)
};

//==============================================================================
/** Base class for fulfilling the ARAEditorView role as described in the ARA SDK.

    Instances of this class are constructed by the DocumentController. If you are subclassing
    ARAEditorView, make sure to call the base class implementation of overridden functions.

    @tags{ARA}
*/
class JUCE_API  ARAEditorView  : public ARA::PlugIn::EditorView
{
public:
    using ARA::PlugIn::EditorView::EditorView;

    // Shadowing templated getters to default to JUCE versions of the returned classes
    template <typename RegionSequence_t = ARARegionSequence>
    const std::vector<RegionSequence_t*>& getHiddenRegionSequences() const noexcept
    {
        return ARA::PlugIn::EditorView::getHiddenRegionSequences<RegionSequence_t>();
    }

    // Base class implementation must be called if overridden
    void doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept override;

    // Base class implementation must be called if overridden
    void doNotifyHideRegionSequences (const std::vector<ARA::PlugIn::RegionSequence*>& regionSequences) noexcept override;

    /** A base class for listeners that want to know about changes to an ARAEditorView object.

        Use ARAEditorView::addListener() to register your listener with an ARAEditorView.
    */
    class JUCE_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when the editor view's selection changes.
            @param viewSelection The current selection state
        */
        virtual void onNewSelection (const ARAViewSelection& viewSelection);

        /** Called when region sequences are flagged as hidden in the host UI.
            @param regionSequences A vector containing all hidden region sequences.
        */
        virtual void onHideRegionSequences (const std::vector<ARARegionSequence*>& regionSequences);
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
