#pragma once

#include "juce_ARA_audio_plugin.h"
#include "juce_ARADocumentController.h"
#include "juce_SafeRef.h"

namespace juce
{

class ARAAudioSource : public ARA::PlugIn::AudioSource,
                       ARAAudioSourceUpdateListener
{
public:
    ARAAudioSource (ARA::PlugIn::Document*, ARA::ARAAudioSourceHostRef);
    virtual ~ARAAudioSource();

    AudioFormatReader* newReader();

    void willUpdateAudioSourceProperties(ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> newProperties) noexcept override;

    void didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept override;

    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;

    void didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;

    void doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept override;

    std::unique_ptr<BufferingAudioSource> createBufferingAudioSource (TimeSliceThread& thread, int bufferSize);

private:
    void invalidateReaders();

    class Reader;
    typedef SafeRef<ARAAudioSource> Ref;

    Ref::Ptr ref;

    // Active readers.
    std::vector<Reader*> readers;

   #if JUCE_DEBUG
    bool stateUpdateProperties = false, stateEnableSamplesAccess = false;
   #endif
};

} // namespace juce
