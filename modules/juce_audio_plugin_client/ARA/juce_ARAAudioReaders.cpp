#include "juce_ARAAudioReaders.h"

namespace juce
{

ARAAudioSourceReader::ARAAudioSourceReader (ARAAudioSource* audioSource)
    : AudioFormatReader (nullptr, "ARAAudioSourceReader"),
      audioSourceBeingRead (audioSource)
{
    jassert (audioSourceBeingRead != nullptr);

    bitsPerSample = 32;
    usesFloatingPointData = true;
    sampleRate = audioSourceBeingRead->getSampleRate();
    numChannels = (unsigned int) audioSourceBeingRead->getChannelCount();
    lengthInSamples = audioSourceBeingRead->getSampleCount();
    tmpPtrs.resize (numChannels);

    audioSourceBeingRead->addListener (this);
    if (audioSourceBeingRead->isSampleAccessEnabled())
        hostReader.reset (new ARA::PlugIn::HostAudioReader (audioSourceBeingRead));
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

    hostReader.reset();

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

void ARAAudioSourceReader::doUpdateAudioSourceContent (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags)
{
    jassert (audioSourceBeingRead == audioSource);

    // don't invalidate if the audio signal is unchanged
    if (scopeFlags.affectSamples())
        invalidate();
}

void ARAAudioSourceReader::willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable)
{
    jassert (audioSourceBeingRead == audioSource);

    // invalidate our reader if sample access is disabled
    if (! enable)
    {
        ScopedWriteLock scopedLock (lock);
        hostReader.reset();
    }
}

void ARAAudioSourceReader::didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable)
{
    jassert (audioSourceBeingRead == audioSource);

    // recreate our reader if sample access is enabled
    if (enable && isValid())
    {
        ScopedWriteLock scopedLock (lock);
        hostReader.reset (new ARA::PlugIn::HostAudioReader (audioSourceBeingRead));
    }
}

void ARAAudioSourceReader::willDestroyAudioSource (ARAAudioSource* audioSource)
{
    jassert (audioSourceBeingRead == audioSource);

    invalidate();
}

bool ARAAudioSourceReader::readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                        int64 startSampleInFile, int numSamples)
{
    const auto destSize = (bitsPerSample / 8) * (size_t) numSamples;
    const auto bufferOffset = (int) (bitsPerSample / 8) * startOffsetInDestBuffer;

    // If we're invalid or can't enter the lock or audio source access is currently disabled, zero samples and return false
    bool gotReadlock = isValid() ? lock.tryEnterRead() : false;
    if (! isValid() || ! gotReadlock || (hostReader == nullptr))
    {
        if (gotReadlock)
            lock.exitRead();

        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            if (destSamples[chan_i] != nullptr)
                zeromem (((uint8_t*) destSamples[chan_i]) + bufferOffset, destSize);

        return false;
    }

    for (size_t chan_i = 0; chan_i < tmpPtrs.size(); ++chan_i)
    {
        if ((chan_i < (size_t) numDestChannels) && (destSamples[chan_i] != nullptr))
        {
            tmpPtrs[chan_i] = ((uint8_t*) destSamples[chan_i]) + bufferOffset;
        }
        else
        {
            // When readSamples is not reading all channels,
            // we still need to provide pointers to all channels to the ARA read call.
            // So we'll read the other channels into this dummy buffer.
            static ThreadLocalValue<std::vector<uint8_t>> dummyBuffer;
            if (destSize > dummyBuffer.get().size())
                dummyBuffer.get().resize (destSize);

            tmpPtrs[chan_i] = dummyBuffer.get().data();
        }
    }

    bool success = hostReader->readAudioSamples (startSampleInFile, numSamples, tmpPtrs.data());

    lock.exitRead();

    return success;
}

//==============================================================================

ARAPlaybackRegionReader::ARAPlaybackRegionReader (ARAPlaybackRegion* playbackRegion)
    : ARAPlaybackRegionReader (playbackRegion->getAudioModification()->getAudioSource()->getSampleRate(),
                               playbackRegion->getAudioModification()->getAudioSource()->getChannelCount(),
                               { playbackRegion })
{}

ARAPlaybackRegionReader::ARAPlaybackRegionReader (double rate, int numChans,
                                                  std::vector<ARAPlaybackRegion*> const& playbackRegions)
    : AudioFormatReader (nullptr, "ARAPlaybackRegionReader")
{
    // we're only providing the minimal set of meaningful values, since the ARA renderer
    // should only look at the time position and the playing state, and read any related
    // tempo or bar signature information from the ARA model directly (MusicalContext)
    positionInfo.resetToDefault();
    positionInfo.isPlaying = true;

    sampleRate = rate;
    numChannels = (unsigned int) numChans;
    bitsPerSample = 32;
    usesFloatingPointData = true;

    ARADocumentController* documentController = (! playbackRegions.empty()) ? playbackRegions.front()->getDocumentController() : nullptr;
    playbackRenderer.reset (documentController ? static_cast<ARAPlaybackRenderer*> (documentController->doCreatePlaybackRenderer()) : nullptr);
    if (playbackRenderer)
    {
        double regionsStartTime = std::numeric_limits<double>::max();
        double regionsEndTime = std::numeric_limits<double>::lowest();

        for (const auto& playbackRegion : playbackRegions)
        {
            jassert (playbackRegion->getDocumentController() == documentController);
            auto playbackRegionTimeRange = playbackRegion->getTimeRange (true);
            regionsStartTime = jmin (regionsStartTime, playbackRegionTimeRange.getStart());
            regionsEndTime = jmax (regionsEndTime, playbackRegionTimeRange.getEnd());

            playbackRenderer->addPlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
            playbackRegion->addListener (this);
        }

        startInSamples = (int64) (regionsStartTime * sampleRate + 0.5);
        lengthInSamples = (int64) ((regionsEndTime - regionsStartTime) * sampleRate + 0.5);

        playbackRenderer->prepareToPlay (rate, maximumBlockSize, numChans, true);
    }
    else
    {
        startInSamples = 0;
        lengthInSamples = 0;
    }
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

    for (auto& playbackRegion : playbackRenderer->getPlaybackRegions())
        playbackRegion->removeListener (this);

    playbackRenderer->releaseResources();
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
            positionInfo.timeInSamples = startSampleInFile + startInSamples;
            while (numSamples > 0)
            {
                int numSliceSamples = jmin (numSamples, maximumBlockSize);
                AudioBuffer<float> buffer ((float **) destSamples, numDestChannels, startOffsetInDestBuffer, numSliceSamples);
                positionInfo.timeInSeconds = static_cast<double> (positionInfo.timeInSamples) / sampleRate;
                success &= playbackRenderer->processBlock (buffer, true, positionInfo);
                numSamples -= numSliceSamples;
                startOffsetInDestBuffer += numSliceSamples;
                positionInfo.timeInSamples += numSliceSamples;
            }
        }

        lock.exitRead();
    }

    if (needClearSamples)
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            FloatVectorOperations::clear ((float *) destSamples[chan_i], numSamples);

    return success;
}

void ARAPlaybackRegionReader::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties)
{
    jassert (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    if ((playbackRegion->getStartInAudioModificationTime() != newProperties->startInModificationTime) ||
        (playbackRegion->getDurationInAudioModificationTime() != newProperties->durationInModificationTime) ||
        (playbackRegion->getStartInPlaybackTime() != newProperties->startInPlaybackTime) ||
        (playbackRegion->getDurationInPlaybackTime() != newProperties->durationInPlaybackTime)||
        (playbackRegion->isTimestretchEnabled() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretch) != 0)) ||
        (playbackRegion->isTimeStretchReflectingTempo() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretchReflectingTempo) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtHead() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtHead) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtTail() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtTail) != 0)))
        invalidate();
}

void ARAPlaybackRegionReader::didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags)
{
    jassert (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    // invalidate if the audio signal is changed
    if (scopeFlags.affectSamples())
        invalidate();
}

void ARAPlaybackRegionReader::willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion)
{
    jassert (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    invalidate();
}

} // namespace juce
