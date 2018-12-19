#pragma once

#include "JuceHeader.h"

namespace juce
{

//==============================================================================
// shared base class for ARAPlaybackRenderer and ARAEditorRenderer, not to be used directly
class ARARendererBase
{
public:
    bool isPrepared() const noexcept            { return prepared; }
    double getSampleRate() const noexcept       { return sampleRate; }
    int getNumChannels() const noexcept         { return numChannels; }
    int getMaxSamplesPerBlock() const noexcept  { return maxSamplesPerBlock; }

protected:
    void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock);
    void releaseResources();

private:
    double sampleRate = 44100.0;
    int numChannels = 1;
    int maxSamplesPerBlock = 1024;
    bool prepared = false;
};

//==============================================================================
class ARAPlaybackRenderer     : public ARA::PlugIn::PlaybackRenderer,
                                public ARARendererBase
{
public:
    using ARA::PlugIn::PlaybackRenderer::PlaybackRenderer;

    bool isPreparedForRealtime() const noexcept { return preparedForRealtime; }

    // If you are subclassing ARAPlaybackRenderer, make sure to call the base class
    // implementations of any overridden function, except for processBlock().
    virtual void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock, bool mayBeRealtime);
    virtual bool processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack, bool isNonRealtime);
    virtual void releaseResources();

    // only to be called if using a playback renderer created internally, i.e. not by the host.
    void addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;

private:
    bool preparedForRealtime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRenderer)
};

//==============================================================================
class ARAEditorRenderer   : public ARA::PlugIn::EditorRenderer,
                            public ARARendererBase
{
public:
    using ARA::PlugIn::EditorRenderer::EditorRenderer;

    // If you are subclassing ARAEditorRenderer, make sure to call the base class
    // implementations of any overridden function, except for processBlock().
    virtual void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock);
    virtual bool processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack);
    virtual void releaseResources();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorRenderer)
};

//==============================================================================
class ARAEditorView  : public ARA::PlugIn::EditorView
{
public:
    using ARA::PlugIn::EditorView::EditorView;

    // If you are subclassing ARAEditorView, make sure to call the base class
    // implementations of all overridden functions.
    void doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept override;
    void doNotifyHideRegionSequences (std::vector<ARA::PlugIn::RegionSequence*> const& regionSequences) noexcept override;

    class Listener
    {
    public:
        virtual ~Listener() = default;

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) {}
        virtual void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

private:
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAEditorView)
};

}
