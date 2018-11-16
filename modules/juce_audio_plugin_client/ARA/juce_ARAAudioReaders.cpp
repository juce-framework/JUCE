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
    // If we can't enter the lock or we don't have a reader, zero samples and return false
    if (! lock.tryEnterRead() || (araHostReader == nullptr))
    {
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            FloatVectorOperations::clear ((float *) destSamples[chan_i], numSamples);
        return false;
    }

    for (int chan_i = 0; chan_i < (int) tmpPtrs.size (); ++chan_i)
        if (chan_i < numDestChannels && destSamples[chan_i] != nullptr)
            tmpPtrs[chan_i] = (void*) (destSamples[chan_i] + startOffsetInDestBuffer);
        else
        {
            if (numSamples > (int) dummyBuffer.size ())
                dummyBuffer.resize ((bitsPerSample / 8) * numSamples);
            tmpPtrs[chan_i] = (void*) dummyBuffer.data ();
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
    // deal with single and double precision floats
    bitsPerSample = 32;
    usesFloatingPointData = true;
    numChannels = 0;
    lengthInSamples = 0;
    sampleRate = 0;

    for (auto playbackRegion : playbackRegions)
    {
        ARA::PlugIn::AudioModification* modification = playbackRegion->getAudioModification();
        ARA::PlugIn::AudioSource* source = modification->getAudioSource();

        if (sampleRate == 0.0)
            sampleRate = source->getSampleRate();

        if (sampleRate != source->getSampleRate())
        {
            // Skip regions with mis-matching sample-rates!
            continue;
        }

        numChannels = std::max (numChannels, (unsigned int) source->getChannelCount());
        lengthInSamples = std::max (lengthInSamples, playbackRegion->getEndInPlaybackSamples (sampleRate));

        playbackRenderer->addPlaybackRegion (playbackRegion);
    }
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

    AudioBuffer<float> buffer ((float **) destSamples, numDestChannels, startOffsetInDestBuffer, numSamples);
    playbackRenderer->renderSamples (buffer, sampleRate, startSampleInFile, true);
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
