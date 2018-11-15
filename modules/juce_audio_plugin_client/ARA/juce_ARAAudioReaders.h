#pragma once

#include "juce_ARAAudioSource.h"

namespace juce
{
    
class ARAAudioSourceReader : public AudioFormatReader,
                             ARAAudioSource::Listener
{
public:
    ARAAudioSourceReader (ARA::PlugIn::AudioSource* audioSource, bool use64BitSamples = false);
    virtual ~ARAAudioSourceReader();

    bool readSamples (
        int** destSamples,
        int numDestChannels,
        int startOffsetInDestBuffer,
        int64 startSampleInFile,
        int numSamples) override;

    // TODO JUCE_ARA
    // do we need to handle property updates?
    // any other invalidation hooks? 
    void willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) noexcept override;
    void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) noexcept override;
    void willDestroyAudioSource (ARAAudioSource* audioSource) noexcept override;
    void doUpdateAudioSourceContent (ARAAudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept override;

private:
    void recreate();
    void invalidate();

private:
    ARAAudioSource* audioSourceBeingRead;
    std::unique_ptr<ARA::PlugIn::HostAudioReader> araHostReader;

    // per reader locks means we can create readers while others are reading
    ReadWriteLock lock;

    std::vector<void*> tmpPtrs;

    // When readSamples is not reading all channels,
    // we still need to provide pointers to all channels to the ARA read call.
    // So we'll read the other channels into this dummy buffer.
    std::vector<float> dummyBuffer;
};

//==============================================================================

class ARAPlaybackRegionReader : public AudioFormatReader
{
public:
    ARAPlaybackRegionReader (ARAPlaybackRenderer* playbackRenderer, std::vector<ARAPlaybackRegion*> const& playbackRegions);
    virtual ~ARAPlaybackRegionReader();

    bool readSamples (
        int** destSamples,
        int numDestChannels,
        int startOffsetInDestBuffer,
        int64 startSampleInFile,
        int numSamples) override;

protected:
    ARAPlaybackRenderer* playbackRenderer;
    ReadWriteLock lock;
};

//==============================================================================

class ARARegionSequenceReader : public ARAPlaybackRegionReader,
                                ARAPlaybackRegion::Listener
{
public:
    ARARegionSequenceReader (ARAPlaybackRenderer* playbackRenderer, ARARegionSequence* regionSequence);
    virtual ~ARARegionSequenceReader();

    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept override;
    void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept override;

private:
    ARARegionSequence* sequence;
};

} // namespace juce
