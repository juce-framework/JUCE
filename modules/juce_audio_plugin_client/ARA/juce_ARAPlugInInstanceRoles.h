#pragma once

#include "JuceHeader.h"

namespace juce
{
    
//==============================================================================
class ARAPlaybackRenderer : public ARA::PlugIn::PlaybackRenderer
{
public:
    ARAPlaybackRenderer (ARADocumentController* documentController);

    virtual void renderSamples (AudioBuffer<float>& buffer, ARA::ARASampleRate sampleRate, ARA::ARASamplePosition samplePosition, bool isPlayingBack);
    
    void addPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void removePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
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

    /** Listener class that can be used to get selection notification updates */
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
