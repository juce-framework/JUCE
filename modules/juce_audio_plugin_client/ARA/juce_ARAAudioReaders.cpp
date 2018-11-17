#include "juce_ARAAudioReaders.h"

namespace juce
{

ARAAudioSourceReader::ARAAudioSourceReader (ARA::PlugIn::AudioSource* audioSource, bool use64BitSamples)
: AudioFormatReader (nullptr, "ARAAudioSourceReader"),
  audioSourceBeingRead (static_cast<ARAAudioSource*> (audioSource))
{
    jassert (audioSourceBeingRead != nullptr);

    bitsPerSample = use64BitSamples ? 64 : 32;
    usesFloatingPointData = true;
    sampleRate = audioSourceBeingRead->getSampleRate();
    numChannels = audioSourceBeingRead->getChannelCount();
    lengthInSamples = audioSourceBeingRead->getSampleCount();
    tmpPtrs.resize (numChannels);

    audioSourceBeingRead->addListener (this);
    if (audioSourceBeingRead->isSampleAccessEnabled())
        recreate();
}

ARAAudioSourceReader::~ARAAudioSourceReader()
{
    if (audioSourceBeingRead)
        audioSourceBeingRead->removeListener (this);

    ScopedWriteLock l (lock);
    invalidate();
}

void ARAAudioSourceReader::willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) noexcept
{
    jassert (audioSourceBeingRead == audioSource);

    // unlocked in didEnableAudioSourceSamplesAccess
    lock.enterWrite();

    // invalidate our reader if sample access is disabled
    if (enable == false)
        invalidate();
}

void ARAAudioSourceReader::didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) noexcept
{
    jassert (audioSourceBeingRead == audioSource);

    // following the invalidation above we can recreate any readers
    // we had before access was disabled

    // recreate our reader if sample access is enabled
    if (enable)
        recreate();

    lock.exitWrite();
}

void ARAAudioSourceReader::willDestroyAudioSource (ARAAudioSource* audioSource) noexcept
{
    jassert (audioSourceBeingRead == audioSource);

    audioSourceBeingRead->removeListener (this);

    ScopedWriteLock scopedLock (lock);
    invalidate();

    audioSourceBeingRead = nullptr;
}

void ARAAudioSourceReader::doUpdateAudioSourceContent (ARAAudioSource* audioSource, const ARA::ARAContentTimeRange* /*range*/, ARA::ARAContentUpdateFlags flags) noexcept
{
    jassert (audioSourceBeingRead == audioSource);

    // don't invalidate if the audio signal is unchanged
    if ((flags & ARA::kARAContentUpdateSignalScopeRemainsUnchanged) != 0)
        return;

    ScopedWriteLock scopedLock (lock);
    invalidate();
}

void ARAAudioSourceReader::recreate()
{
    jassert (araHostReader == nullptr);

    if (audioSourceBeingRead == nullptr)
        return;

    jassert (audioSourceBeingRead->isSampleAccessEnabled());
    araHostReader.reset (new ARA::PlugIn::HostAudioReader (audioSourceBeingRead));
}

void ARAAudioSourceReader::invalidate()
{
//  jassert (lock.isLocked());
    araHostReader.reset();
}

bool ARAAudioSourceReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                        int64 startSampleInFile, int numSamples)
{
    int destSize = (bitsPerSample / 8) * numSamples;
    int bufferOffset = (bitsPerSample / 8) * startOffsetInDestBuffer;

    // If we can't enter the lock or we don't have a reader, zero samples and return false
    if (! lock.tryEnterRead() || (araHostReader == nullptr))
    {
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
        {
            if (destSamples[chan_i] != nullptr)
                zeromem (((uint8_t*) destSamples[chan_i]) + bufferOffset, destSize);
        }
        return false;
    }

    for (int chan_i = 0; chan_i < (int) tmpPtrs.size(); ++chan_i)
    {
        if ((chan_i < numDestChannels) && (destSamples[chan_i] != nullptr))
        {
            tmpPtrs[chan_i] = ((uint8_t*) destSamples[chan_i]) + bufferOffset;
        }
        else
        {
            // When readSamples is not reading all channels,
            // we still need to provide pointers to all channels to the ARA read call.
            // So we'll read the other channels into this dummy buffer.
            static ThreadLocalValue<std::vector<uint8_t>> dummyBuffer;
            if (destSize > (int) dummyBuffer.get().size())
                dummyBuffer.get().resize (destSize);

            tmpPtrs[chan_i] = dummyBuffer.get().data();
        }
    }

    bool success = araHostReader->readAudioSamples (startSampleInFile, numSamples, tmpPtrs.data());
    lock.exitRead();
    return success;
}

//==============================================================================

ARAPlaybackRegionReader::ARAPlaybackRegionReader (ARAPlaybackRenderer* playbackRenderer, std::vector<ARAPlaybackRegion*> const& playbackRegions)
: AudioFormatReader (nullptr, "ARAAudioSourceReader"),
  playbackRenderer (playbackRenderer)
{
    // TODO JUCE_ARA
    // Make sampleRate, numChannels and use64BitSamples available as c'tor parameters instead
    // of deducing it here. Since regions can start anywhere on the timeline, maybe also define
    // which time range should be considered as "range to be read by this reader".
    bitsPerSample = 32;
    usesFloatingPointData = true;
    numChannels = 1;
    lengthInSamples = 0;
    sampleRate = 0;

    for (auto playbackRegion : playbackRegions)
    {
        ARA::PlugIn::AudioModification* modification = playbackRegion->getAudioModification();
        ARA::PlugIn::AudioSource* source = modification->getAudioSource();

        if (sampleRate == 0.0)
            sampleRate = source->getSampleRate();

        numChannels = std::max (numChannels, (unsigned int) source->getChannelCount());
        lengthInSamples = std::max (lengthInSamples, playbackRegion->getEndInPlaybackSamples (sampleRate));

        playbackRenderer->addPlaybackRegion (playbackRegion);
    }

	if (sampleRate == 0.0)
		sampleRate = 44100;
    playbackRenderer->prepareToPlay(sampleRate, 16*1024);
}

ARAPlaybackRegionReader::~ARAPlaybackRegionReader()
{
    ScopedWriteLock scopedWrite (lock);
    delete playbackRenderer;
}

bool ARAPlaybackRegionReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                           int64 startSampleInFile, int numSamples)
{
    // render our ARA playback regions for this time duration using the ARA playback renderer instance
    if (! lock.tryEnterRead())
    {
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            FloatVectorOperations::clear ((float *) destSamples[chan_i], numSamples);
        return false;
    }

    while (numSamples > 0)
    {
        int numSliceSamples = std::min(numSamples, playbackRenderer->getMaxSamplesPerBlock());
        AudioBuffer<float> buffer ((float **) destSamples, numDestChannels, startOffsetInDestBuffer, numSliceSamples);
        playbackRenderer->processBlock (buffer, startSampleInFile, true);
        numSamples -= numSliceSamples;
        startOffsetInDestBuffer += numSliceSamples;
        startSampleInFile += numSliceSamples;
    }

    lock.exitRead();
    return true;
}

//==============================================================================

ARARegionSequenceReader::ARARegionSequenceReader (ARAPlaybackRenderer* playbackRenderer, ARARegionSequence* regionSequence)
: ARAPlaybackRegionReader (playbackRenderer, reinterpret_cast<std::vector<ARAPlaybackRegion*> const&> (regionSequence->getPlaybackRegions())),
  sequence (regionSequence)
{
    for (auto playbackRegion : sequence->getPlaybackRegions())
        static_cast<ARAPlaybackRegion*> (playbackRegion)->addListener (this);
}

ARARegionSequenceReader::~ARARegionSequenceReader()
{
    for (auto playbackRegion : playbackRenderer->getPlaybackRegions())
        static_cast<ARAPlaybackRegion*> (playbackRegion)->removeListener (this);
}

void ARARegionSequenceReader::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept
{
    if (ARA::contains (playbackRenderer->getPlaybackRegions(), static_cast<ARA::PlugIn::PlaybackRegion*> (playbackRegion)))
    {
        if (newProperties->regionSequenceRef != ARA::PlugIn::toRef (sequence))
        {
            ScopedWriteLock scopedWrite (lock);
            playbackRegion->removeListener (this);
            playbackRenderer->removePlaybackRegion (playbackRegion);
        }
    }
    else if (newProperties->regionSequenceRef == ARA::PlugIn::toRef (sequence))
    {
        ScopedWriteLock scopedWrite (lock);
        playbackRegion->addListener (this);
        playbackRenderer->addPlaybackRegion (playbackRegion);
    }
}

void ARARegionSequenceReader::willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    if (ARA::contains (playbackRenderer->getPlaybackRegions(), static_cast<ARA::PlugIn::PlaybackRegion*> (playbackRegion)))
    {
        ScopedWriteLock scopedWrite (lock);
        playbackRegion->removeListener (this);
        playbackRenderer->removePlaybackRegion (playbackRegion);
    }
}

} // namespace juce
