#include "juce_ARAAudioReaders.h"

namespace juce
{
    
ARAAudioSourceReader::ARAAudioSourceReader (ARAAudioSource* audioSource, bool use64BitSamples)
: AudioFormatReader (nullptr, "ARAAudioSourceReader"),
  audioSourceBeingRead (audioSource)
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
    if (! enable)
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
    bool gotReadlock = lock.tryEnterRead();
    if (! gotReadlock || (araHostReader == nullptr))
    {
        if (gotReadlock)
            lock.exitRead ();

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

    if (!success)
    {
        ScopedWriteLock scopedLock (lock);
        invalidate();
    }

    return success;
}

//==============================================================================

ARAPlaybackRegionReader::ARAPlaybackRegionReader (ARAPlaybackRenderer* renderer, std::vector<ARAPlaybackRegion*> const& playbackRegions)
: AudioFormatReader (nullptr, "ARAPlaybackRegionReader")
{
    playbackRenderer.reset (renderer);

    // TODO JUCE_ARA
    // Make sampleRate, numChannels and use64BitSamples available as c'tor parameters instead
    // of deducing it here. Since regions can start anywhere on the timeline, maybe also define
    // which time range should be considered as "range to be read by this reader".
    bitsPerSample = 32;
    usesFloatingPointData = true;
    numChannels = 1;

    if (playbackRegions.size() == 0)
    {
        lengthInSamples = 0;
        sampleRate = 44100.0;
        regionsStartTime = 0.0;
        regionsEndTime = 0.0;
    }
    else
    {
        sampleRate = 0.0;
        regionsStartTime = std::numeric_limits<double>::max();
        regionsEndTime = std::numeric_limits<double>::min();

        for (auto playbackRegion : playbackRegions)
        {
            ARA::PlugIn::AudioModification* modification = playbackRegion->getAudioModification();
            ARA::PlugIn::AudioSource* source = modification->getAudioSource();

            if (sampleRate == 0.0)
                sampleRate = source->getSampleRate();

            numChannels = jmax (numChannels, (unsigned int) source->getChannelCount());

            regionsStartTime = jmin (regionsStartTime, playbackRegion->getStartInPlaybackTime());
            regionsEndTime = jmax (regionsEndTime, playbackRegion->getEndInPlaybackTime());

            playbackRenderer->addPlaybackRegion (playbackRegion);
            playbackRegion->addListener (this);
        }

        lengthInSamples = (int64)((regionsEndTime - regionsStartTime) * sampleRate + 0.5);
    }

    playbackRenderer->prepareToPlay (sampleRate, numChannels, 16*1024);
}

ARAPlaybackRegionReader::~ARAPlaybackRegionReader()
{
    invalidate();
}

void ARAPlaybackRegionReader::invalidate()
{
    if (isValid())
    {
        ScopedWriteLock scopedWrite (lock);
        for (auto playbackRegion : playbackRenderer->getPlaybackRegions ())
            static_cast<ARAPlaybackRegion*>(playbackRegion)->removeListener (this);

        playbackRenderer.reset();
    }
}

bool ARAPlaybackRegionReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                           int64 startSampleInFile, int numSamples)
{
    bool success = false;
    if (lock.tryEnterRead())
    {
        if (isValid())
        {
            success = true;
            while (numSamples > 0 && success) // TODO JUCE_ARA should we check success here?
            {
                int numSliceSamples = jmin(numSamples, playbackRenderer->getMaxSamplesPerBlock());
                AudioBuffer<float> buffer ((float **) destSamples, numDestChannels, startOffsetInDestBuffer, numSliceSamples);
                success = playbackRenderer->processBlock (buffer, startSampleInFile, true);
                numSamples -= numSliceSamples;
                startOffsetInDestBuffer += numSliceSamples;
                startSampleInFile += numSliceSamples;
            }
        }

        lock.exitRead();
    }

    if (! success)
    {
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            FloatVectorOperations::clear ((float *) destSamples[chan_i], numSamples);

        invalidate();
    }
    return success;
}

void ARAPlaybackRegionReader::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties)
{
    if ((playbackRegion->getStartInAudioModificationTime () != newProperties->startInModificationTime) ||
        (playbackRegion->getDurationInAudioModificationTime () != newProperties->durationInModificationTime) ||
        (playbackRegion->getStartInPlaybackTime () != newProperties->startInPlaybackTime) ||
        (playbackRegion->getDurationInPlaybackTime () != newProperties->durationInPlaybackTime) ||

        (playbackRegion->isTimestretchEnabled () != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretch) != 0)) ||
        (playbackRegion->isTimeStretchReflectingTempo () != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretchReflectingTempo) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtHead () != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtHead) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtTail () != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtTail) != 0)))
    {
        invalidate();
    }
}

void ARAPlaybackRegionReader::willDestroyPlaybackRegion (ARAPlaybackRegion* /*playbackRegion*/) noexcept
{
    invalidate();
}

//==============================================================================

ARARegionSequenceReader::ARARegionSequenceReader (ARAPlaybackRenderer* playbackRenderer, ARARegionSequence* regionSequence)
: ARAPlaybackRegionReader (playbackRenderer, reinterpret_cast<std::vector<ARAPlaybackRegion*> const&> (regionSequence->getPlaybackRegions ())),
  sequence (regionSequence)
{
    sequence->addListener (this);
}

ARARegionSequenceReader::~ARARegionSequenceReader()
{
    if (sequence != nullptr)
        sequence->removeListener (this);

    invalidate();
}

void ARARegionSequenceReader::willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion)
{
    jassert (sequence == regionSequence);
    jassert (ARA::contains (sequence->getPlaybackRegions (), playbackRegion));

    invalidate();
}

void ARARegionSequenceReader::didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion)
{
    jassert (sequence == regionSequence);
    jassert (ARA::contains (sequence->getPlaybackRegions (), playbackRegion));

    invalidate();
}

void ARARegionSequenceReader::willDestroyRegionSequence (ARARegionSequence* regionSequence)
{
    jassert (sequence == regionSequence);

    invalidate();

    sequence->removeListener (this);
    sequence = nullptr;
}

} // namespace juce
