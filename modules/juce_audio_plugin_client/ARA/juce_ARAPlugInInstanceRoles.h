#pragma once

#include "JuceHeader.h"

namespace juce
{

//==============================================================================
// shared base class for ARAPlaybackRenderer and ARAEditorRenderer, not to be used directly
template <typename ARARendererType, void (ARARendererType::*setRenderingFunc) (bool), bool clearProcessBuffer>
class ARARendererBase : public ARARendererType
{
public:
    using ARARendererType::ARARendererType;

    virtual void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock, bool /*mayBeRealtime*/)
    {
        sampleRate = newSampleRate;
        numChannels = newNumChannels;
        maxSamplesPerBlock = newMaxSamplesPerBlock;

        if (setRenderingFunc)
            (this->*setRenderingFunc) (true);
        prepared = true;
    }

    virtual bool processBlock (AudioBuffer<float>& buffer, int64 /*timeInSamples*/, bool /*isPlayingBack*/, bool /*isNonRealtime*/)
    {
        jassert (buffer.getNumSamples() <= getMaxSamplesPerBlock());
        if (clearProcessBuffer)
            buffer.clear();
        return true;
    }

    virtual void releaseResources()
    {
        prepared = false;
        if (setRenderingFunc)
            (this->*setRenderingFunc) (false);
    }

    bool isPrepared() const noexcept            { return prepared; }

    double getSampleRate() const noexcept       { return sampleRate; }
    int getNumChannels() const noexcept         { return numChannels; }
    int getMaxSamplesPerBlock() const noexcept  { return maxSamplesPerBlock; }

private:
    double sampleRate = 44100.0;
    int numChannels = 1;
    int maxSamplesPerBlock = 1024;
    bool prepared = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARARendererBase)
};

//==============================================================================
using ARAPlaybackRendererBase = ARARendererBase<ARA::PlugIn::PlaybackRenderer, &ARA::PlugIn::PlaybackRenderer::setRendering, true>;

class ARAPlaybackRenderer : public ARAPlaybackRendererBase
{
public:
    using ARAPlaybackRendererBase::ARAPlaybackRendererBase;

    // If you are subclassing ARAPlaybackRenderer, make sure to call the base class
    // implementations of any overridden function, except for processBlock().

    // only to be called if using a playback renderer created internally, i.e. not by the host.
    void addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;

private:

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRenderer)
};

//==============================================================================
using ARAEditorRendererBase = ARARendererBase<ARA::PlugIn::EditorRenderer, nullptr, false>;
class ARAEditorRenderer : public ARAEditorRendererBase
{
public:
    using ARAEditorRendererBase::ARAEditorRendererBase;

    // If you are subclassing ARAEditorRenderer, make sure to call the base class
    // implementations of any overridden function, except for processBlock().

private:

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorRenderer)
};

//==============================================================================
class ARAEditorView : public ARA::PlugIn::EditorView
{
public:
    ARAEditorView (ARA::PlugIn::DocumentController* documentController) noexcept;

    // If you are subclassing ARAEditorView, make sure to call the base class
    // implementations of all overridden functions.
    void doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept override;
    void doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept override;

    class Listener
    {
    public:
        virtual ~Listener() {}

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) {}
        virtual void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

private:
    std::vector<Listener*> listeners;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorView)
};

}
