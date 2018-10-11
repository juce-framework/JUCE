#pragma once

#include "ARAUtils.h"

namespace juce
{

class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    ARADocumentController();
    ~ARADocumentController();

    //==============================================================================
    // Override document controller methods here
protected:
    // needed for ARA AudioFormatReaders to be thread-safe and work properly!
    ARA::PlugIn::AudioSource* doCreateAudioSource       (ARA::PlugIn::Document*, ARA::ARAAudioSourceHostRef) override;
    ARA::PlugIn::RegionSequence* doCreateRegionSequence (ARA::PlugIn::Document*, ARA::ARARegionSequenceHostRef) override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) override;
    void didEnableAudioSourceSamplesAccess  (ARA::PlugIn::AudioSource* audioSource, bool enable) override;
    void willUpdateAudioSourceProperties (
        ARA::PlugIn::AudioSource*, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties>) override;
    void didUpdateAudioSourceProperties     (ARA::PlugIn::AudioSource *audioSource) override;
    void willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) override;
    void didUpdatePlaybackRegionProperties  (ARA::PlugIn::PlaybackRegion* playbackRegion) override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentController)
};

} // namespace juce
