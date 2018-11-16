#pragma once

#include "JuceHeader.h"

namespace juce
{
    
//==============================================================================
class ARAPlaybackRenderer : public ARA::PlugIn::PlaybackRenderer
{
public:
    ARAPlaybackRenderer (ARADocumentController* documentController);

    virtual void prepareToPlay (double sampleRate, int maxSamplesPerBlock);
    virtual void processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack);
    virtual void releaseResources();

    double getSampleRate() const noexcept       { return sampleRate; }
    int getMaxSamplesPerBlock() const noexcept  { return maxSamplesPerBlock; }

    void addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;

private:
    double sampleRate;
    int maxSamplesPerBlock;

public:
   #if ! JUCE_DISABLE_ASSERTIONS
    bool isPreparedToPlay;
   #endif
};

//==============================================================================
class ARAEditorRenderer: public ARA::PlugIn::EditorRenderer
{
public:
    ARAEditorRenderer (ARADocumentController* documentController);

    void addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void addRegionSequence (ARARegionSequence* regionSequence) noexcept;
    void removeRegionSequence (ARARegionSequence* regionSequence) noexcept;

    // TODO JUCE_ARA 
    // what should this look like?
    // virtual void renderSamples (AudioBuffer<float>& buffer, ARA::ARASampleRate sampleRate);
};

//==============================================================================
class ARAEditorView : public ARA::PlugIn::EditorView
{
public:
    ARAEditorView (ARA::PlugIn::DocumentController* documentController) noexcept;

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
