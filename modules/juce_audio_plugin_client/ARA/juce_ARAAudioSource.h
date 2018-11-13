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

class ARAAudioSourceReader : public AudioFormatReader,
                             ARAAudioSourceUpdateListener
{
public:
    ARAAudioSourceReader (ARA::PlugIn::AudioSource* audioSource, bool use64BitSamples = false);
    ~ARAAudioSourceReader ();

    void recreate ();
    void invalidate ();

    bool readSamples (
        int** destSamples,
        int numDestChannels,
        int startOffsetInDestBuffer,
        int64 startSampleInFile,
        int numSamples) override;

    // TODO JUCE_ARA
    // do we need to handle property updates?
    // any other invalidation hooks? 
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept override;
    void doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept override;

private:
    std::vector<void*> tmpPtrs;

    // per reader locks means we can create readers while others are reading
    ReadWriteLock lock;

    // When readSamples is not reading all channels,
    // we still need to provide pointers to all channels to the ARA read call.
    // So we'll read the other channels into this dummy buffer.
    std::vector<float> dummyBuffer;
    ARA::PlugIn::AudioSource* audioSourceBeingRead; // TODO JUCE_ARA make this const
    std::unique_ptr<ARA::PlugIn::HostAudioReader> araHostReader;
};
} // namespace juce
