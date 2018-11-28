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
        araHostReader.reset (new ARA::PlugIn::HostAudioReader (audioSourceBeingRead));
}

ARAAudioSourceReader::~ARAAudioSourceReader()
{
    invalidate();
}

void ARAAudioSourceReader::invalidate()
{
    ScopedWriteLock scopedLock (lock);

    if (! isValid())
        return;

    araHostReader.reset();

    audioSourceBeingRead->removeListener (this);
    audioSourceBeingRead = nullptr;
}

void ARAAudioSourceReader::willUpdateAudioSourceProperties (ARAAudioSource* audioSource, ARAAudioSource::PropertiesPtr newProperties)
{
    if (audioSource->getSampleCount() != newProperties->sampleCount ||
        audioSource->getSampleRate() != newProperties->sampleRate ||
        audioSource->getChannelCount() != newProperties->channelCount)
    {
        invalidate();
    }
}

void ARAAudioSourceReader::didUpdateAudioSourceContent (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags)
{
    jassert (audioSourceBeingRead == audioSource);

    // don't invalidate if the audio signal is unchanged
    if (scopeFlags.affectSamples())
        invalidate();
}

void ARAAudioSourceReader::willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable)
{
    jassert (audioSourceBeingRead == audioSource);

    // unlocked in didEnableAudioSourceSamplesAccess
    lock.enterWrite();

    // invalidate our reader if sample access is disabled
    if (! enable)
        araHostReader.reset();
}

void ARAAudioSourceReader::didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable)
{
    jassert (audioSourceBeingRead == audioSource);

    // following the invalidation above we can recreate any readers
    // we had before access was disabled

    // recreate our reader if sample access is enabled
    if (enable && isValid())
        araHostReader.reset (new ARA::PlugIn::HostAudioReader (audioSourceBeingRead));

    lock.exitWrite();
}

void ARAAudioSourceReader::willDestroyAudioSource (ARAAudioSource* audioSource)
{
    jassert (audioSourceBeingRead == audioSource);

    invalidate();
}

bool ARAAudioSourceReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                        int64 startSampleInFile, int numSamples)
{
    int destSize = (bitsPerSample / 8) * numSamples;
    int bufferOffset = (bitsPerSample / 8) * startOffsetInDestBuffer;

    // If we're invalid or can't enter the lock or audio source access is currently disabled, zero samples and return false
    bool gotReadlock = isValid() ? lock.tryEnterRead() : false;
    if (! isValid() || ! gotReadlock || (araHostReader == nullptr))
    {
        if (gotReadlock)
            lock.exitRead();

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

ARAPlaybackRegionReader::ARAPlaybackRegionReader (ARAPlaybackRenderer* renderer, std::vector<ARAPlaybackRegion*> const& playbackRegions, bool nonRealtime)
: AudioFormatReader (nullptr, "ARAPlaybackRegionReader"),
  isNonRealtime (nonRealtime),
  playbackRenderer (renderer)
{
    // TODO JUCE_ARA
    // Make sampleRate, numChannels and use64BitSamples available as c'tor parameters instead
    // of deducing it here. Since regions can start anywhere on the timeline, maybe also define
    // which time range should be considered as "range to be read by this reader".
    bitsPerSample = 32;
    usesFloatingPointData = true;
    numChannels = 1;

    if (playbackRegions.size() == 0)
    {
        startInSamples = 0;
        lengthInSamples = 0;
        sampleRate = 44100.0;
    }
    else
    {
        sampleRate = 0.0;
        double regionsStartTime = std::numeric_limits<double>::max();
        double regionsEndTime = std::numeric_limits<double>::min();

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

        startInSamples = (int64) (regionsStartTime * sampleRate + 0.5);
        lengthInSamples = (int64) ((regionsEndTime - regionsStartTime) * sampleRate + 0.5);
    }

    playbackRenderer->prepareToPlay (sampleRate, numChannels, 16*1024, ! isNonRealtime);
}

ARAPlaybackRegionReader::~ARAPlaybackRegionReader()
{
    invalidate();
}

void ARAPlaybackRegionReader::invalidate()
{
    ScopedWriteLock scopedWrite (lock);

    if (! isValid())
        return;

    for (auto playbackRegion : playbackRenderer->getPlaybackRegions())
        static_cast<ARAPlaybackRegion*>(playbackRegion)->removeListener (this);

    playbackRenderer.reset();
}

bool ARAPlaybackRegionReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                           int64 startSampleInFile, int numSamples)
{
    bool success = false;
    bool needClearSamples = true;
    if (lock.tryEnterRead())
    {
        if (isValid())
        {
            success = true;
            needClearSamples = false;
            startSampleInFile += startInSamples;
            while (numSamples > 0)
            {
                int numSliceSamples = jmin(numSamples, playbackRenderer->getMaxSamplesPerBlock());
                AudioBuffer<float> buffer ((float **) destSamples, numDestChannels, startOffsetInDestBuffer, numSliceSamples);
                success &= playbackRenderer->processBlock (buffer, startSampleInFile, true, isNonRealtime);
                numSamples -= numSliceSamples;
                startOffsetInDestBuffer += numSliceSamples;
                startSampleInFile += numSliceSamples;
            }
        }

        lock.exitRead();
    }

    if (needClearSamples)
    {
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            FloatVectorOperations::clear ((float *) destSamples[chan_i], numSamples);
    }

    return success;
}

void ARAPlaybackRegionReader::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties)
{
    jassert (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    // TODO JUCE_ARA most of these tests should be unnecessary now that we're listening to contentChanged...?
    if ((playbackRegion->getStartInAudioModificationTime() != newProperties->startInModificationTime) ||
        (playbackRegion->getDurationInAudioModificationTime() != newProperties->durationInModificationTime) ||
        (playbackRegion->getStartInPlaybackTime() != newProperties->startInPlaybackTime) ||
        (playbackRegion->getDurationInPlaybackTime() != newProperties->durationInPlaybackTime) ||
        (playbackRegion->isTimestretchEnabled() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretch) != 0)) ||
        (playbackRegion->isTimeStretchReflectingTempo() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretchReflectingTempo) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtHead() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtHead) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtTail() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtTail) != 0)))
    {
        invalidate();
    }
}

void ARAPlaybackRegionReader::didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags)
{
    jassert (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    // don't invalidate if the audio signal is unchanged
    if (scopeFlags.affectSamples())
        invalidate();
}

void ARAPlaybackRegionReader::willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion)
{
    jassert (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    invalidate();
}

//==============================================================================

ARARegionSequenceReader::ARARegionSequenceReader (ARAPlaybackRenderer* playbackRenderer, ARARegionSequence* regionSequence, bool nonRealtime)
: ARAPlaybackRegionReader (playbackRenderer, reinterpret_cast<std::vector<ARAPlaybackRegion*> const&> (regionSequence->getPlaybackRegions()), nonRealtime),
  sequence (regionSequence)
{
    sequence->addListener (this);
}

ARARegionSequenceReader::~ARARegionSequenceReader()
{
    if (sequence != nullptr)
        sequence->removeListener (this);
}

void ARARegionSequenceReader::willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion)
{
    jassert (sequence == regionSequence);
    jassert (ARA::contains (sequence->getPlaybackRegions(), playbackRegion));

    invalidate();
}

void ARARegionSequenceReader::didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion)
{
    jassert (sequence == regionSequence);
    jassert (ARA::contains (sequence->getPlaybackRegions(), playbackRegion));

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
