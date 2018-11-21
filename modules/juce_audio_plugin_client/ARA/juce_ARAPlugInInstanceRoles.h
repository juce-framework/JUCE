#pragma once

#include "JuceHeader.h"

namespace juce
{

//==============================================================================
class ARAPlaybackRenderer : public ARA::PlugIn::PlaybackRenderer
{
public:
    ARAPlaybackRenderer (ARADocumentController* documentController);

    // If you are subclassing ARAPlaybackRenderer, make sure to call the base class
    // implementations of any overridden function, except for processBlock().
    virtual void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock);
    virtual bool processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack);
    virtual void releaseResources();

    double getSampleRate() const noexcept       { return sampleRate; }
    int getNumChannels() const noexcept  { return numChannels; }
    int getMaxSamplesPerBlock() const noexcept  { return maxSamplesPerBlock; }

    // only to be called if using a playback renderer created internally, i.e. not by the host.
    void addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;

private:
    double sampleRate;
    int numChannels;
    int maxSamplesPerBlock;
};

//==============================================================================
class ARAEditorRenderer: public ARA::PlugIn::EditorRenderer
{
public:
    ARAEditorRenderer (ARADocumentController* documentController);

    // TODO JUCE_ARA
    // what should this look like?
    // virtual void processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack);
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

    void addSelectionListener (Listener* l);
    void removeSelectionListener (Listener* l);

private:

    std::vector<Listener*> listeners;
};

}
