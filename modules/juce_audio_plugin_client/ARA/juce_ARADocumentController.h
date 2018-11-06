#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARADocumentController : public ARA::PlugIn::DocumentController
{
public:
    ARADocumentController() noexcept {}
    virtual ~ARADocumentController() noexcept {}

    //==============================================================================
    // Override document controller methods here
protected:
    // needed for ARA AudioFormatReaders to be thread-safe and work properly!
    ARA::PlugIn::AudioSource* doCreateAudioSource       (ARA::PlugIn::Document*, ARA::ARAAudioSourceHostRef) noexcept override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess  (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void willUpdateAudioSourceProperties (
        ARA::PlugIn::AudioSource*, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties>) noexcept override;
    void didUpdateAudioSourceProperties     (ARA::PlugIn::AudioSource *audioSource) noexcept override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocumentController)
};

} // namespace juce
