#pragma once

#include "JuceHeader.h"

namespace juce
{

// juce ARA document controller implementation
class ARADocumentController: public ARA::PlugIn::DocumentController
{
public:
    ARADocumentController() noexcept {}
    virtual ~ARADocumentController() noexcept {}

    // TODO JUCE_ARA
    // should the time slice thread be a part of this document controller class?
    AudioFormatReader* createAudioSourceReader (ARAAudioSource* audioSource);
    BufferingAudioSource* createBufferingAudioSourceReader (ARAAudioSource* audioSource, TimeSliceThread& thread, int bufferSize);
    AudioFormatReader* createPlaybackRegionReader (std::vector<ARAPlaybackRegion*> playbackRegions);
    AudioFormatReader* createRegionSequenceReader (ARARegionSequence* regionSequence);

    //==============================================================================
    // Override document controller methods here
protected:
    // MusicalContext callbacks
    ARA::PlugIn::MusicalContext* doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept override;
    void willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARA::PlugIn::MusicalContext::PropertiesPtr newProperties) noexcept override;
    void didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;
    void doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept override;
    void willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept override;

    // RegionSequence callbacks
    ARA::PlugIn::RegionSequence* doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept override;
    void willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::RegionSequence::PropertiesPtr newProperties) noexcept override;
    void didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept override;
    void willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    // AudioSource callbacks
    ARA::PlugIn::AudioSource* doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept override;
    void willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioSource::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void doDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept override;
    void willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept override;

    // AudioModification callbacks
    ARA::PlugIn::AudioModification* doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef) noexcept override;
    void willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::AudioModification::PropertiesPtr newProperties) noexcept override;
    void didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept override;
    void doDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept override;
    void willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept override;

    // TODO JUCE_ARA 
    // Do we need to override this? The default ARPlug implementation may be sufficient, 
    // but we may want to copy the listener list to the cloned modification
    //ARA::PlugIn::AudioModification* doCloneAudioModification (ARA::PlugIn::AudioModification* src, ARA::ARAAudioModificationHostRef hostRef) noexcept override;

    // PlaybackRegion callbacks
    ARA::PlugIn::PlaybackRegion* doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept override;
    void willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PlaybackRegion::PropertiesPtr newProperties) noexcept override;
    void didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

    // PlugIn instance role creation
    // these can be overridden with custom types so long as 
    // they inherit from our juce::ARA instance role classes
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer () noexcept override;
    ARA::PlugIn::EditorRenderer* doCreateEditorRenderer () noexcept override;
    ARA::PlugIn::EditorView* doCreateEditorView () noexcept override;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentController)
};

} // namespace juce
