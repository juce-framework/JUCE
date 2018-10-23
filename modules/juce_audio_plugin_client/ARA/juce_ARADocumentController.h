#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    ARADocumentController() ARA_NOEXCEPT {}
    virtual ~ARADocumentController() ARA_NOEXCEPT {}

    //==============================================================================
    // Override document controller methods here
protected:
    // needed for ARA AudioFormatReaders to be thread-safe and work properly!
    ARA::PlugIn::AudioSource* doCreateAudioSource       (ARA::PlugIn::Document*, ARA::ARAAudioSourceHostRef) ARA_NOEXCEPT override;
    ARA::PlugIn::RegionSequence* doCreateRegionSequence (ARA::PlugIn::Document*, ARA::ARARegionSequenceHostRef) ARA_NOEXCEPT override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) ARA_NOEXCEPT override;
    void didEnableAudioSourceSamplesAccess  (ARA::PlugIn::AudioSource* audioSource, bool enable) ARA_NOEXCEPT override;
    void willUpdateAudioSourceProperties (
        ARA::PlugIn::AudioSource*, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties>) ARA_NOEXCEPT override;
    void didUpdateAudioSourceProperties     (ARA::PlugIn::AudioSource *audioSource) ARA_NOEXCEPT override;
    void willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) ARA_NOEXCEPT override;
    void didUpdatePlaybackRegionProperties  (ARA::PlugIn::PlaybackRegion* playbackRegion) ARA_NOEXCEPT override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentController)
};

} // namespace juce
