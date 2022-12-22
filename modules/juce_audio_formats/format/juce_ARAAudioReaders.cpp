/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

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

void ARAAudioSourceReader::willUpdateAudioSourceProperties (ARAAudioSource* audioSource,
                                                            ARAAudioSource::PropertiesPtr newProperties)
{
    if (audioSource->getSampleCount() != newProperties->sampleCount
        || audioSource->getSampleRate() != newProperties->sampleRate
        || audioSource->getChannelCount() != newProperties->channelCount)
    {
        invalidate();
    }
}

void ARAAudioSourceReader::doUpdateAudioSourceContent (ARAAudioSource* audioSource,
                                                       ARAContentUpdateScopes scopeFlags)
{
    jassertquiet (audioSourceBeingRead == audioSource);

    // Don't invalidate if the audio signal is unchanged
    if (scopeFlags.affectSamples())
        invalidate();
}

void ARAAudioSourceReader::willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable)
{
    jassertquiet (audioSourceBeingRead == audioSource);

    // Invalidate our reader if sample access is disabled
    if (! enable)
    {
        ScopedWriteLock scopedLock (lock);
        hostReader.reset();
    }
}

void ARAAudioSourceReader::didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable)
{
    jassertquiet (audioSourceBeingRead == audioSource);

    // Recreate our reader if sample access is enabled
    if (enable && isValid())
    {
        ScopedWriteLock scopedLock (lock);
        hostReader.reset (new ARA::PlugIn::HostAudioReader (audioSourceBeingRead));
    }
}

void ARAAudioSourceReader::willDestroyAudioSource (ARAAudioSource* audioSource)
{
    jassertquiet (audioSourceBeingRead == audioSource);

    invalidate();
}

bool ARAAudioSourceReader::readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                        int64 startSampleInFile, int numSamples)
{
    const auto destSize = (bitsPerSample / 8) * (size_t) numSamples;
    const auto bufferOffset = (int) (bitsPerSample / 8) * startOffsetInDestBuffer;

    if (isValid())
    {
        const ScopedTryReadLock readLock (lock);

        if (readLock.isLocked() && hostReader != nullptr)
        {
            for (size_t i = 0; i < tmpPtrs.size(); ++i)
            {
                if ((i < (size_t) numDestChannels) && (destSamples[i] != nullptr))
                {
                    tmpPtrs[i] = ((uint8_t*) destSamples[i]) + bufferOffset;
                }
                else
                {
                    // We need to provide destination pointers for all channels in the ARA read call, even if
                    // readSamples is not reading all of them. Hence we use this dummy buffer to pad the read
                    // destination area.
                    static thread_local std::vector<uint8_t> dummyBuffer;

                    if (destSize > dummyBuffer.size())
                        dummyBuffer.resize (destSize);

                    tmpPtrs[i] = dummyBuffer.data();
                }
            }

            return hostReader->readAudioSamples (startSampleInFile, numSamples, tmpPtrs.data());
        }
    }

    for (int i = 0; i < numDestChannels; ++i)
        if (destSamples[i] != nullptr)
            zeromem (((uint8_t*) destSamples[i]) + bufferOffset, destSize);

    return false;
}

//==============================================================================
ARAPlaybackRegionReader::ARAPlaybackRegionReader (ARAPlaybackRegion* playbackRegion)
    : ARAPlaybackRegionReader (playbackRegion->getAudioModification()->getAudioSource()->getSampleRate(),
                               playbackRegion->getAudioModification()->getAudioSource()->getChannelCount(),
                               { playbackRegion })
{}

ARAPlaybackRegionReader::ARAPlaybackRegionReader (double rate, int numChans,
                                                  const std::vector<ARAPlaybackRegion*>& playbackRegions)
    : AudioFormatReader (nullptr, "ARAPlaybackRegionReader")
{
    // We're only providing the minimal set of meaningful values, since the ARA renderer should only
    // look at the time position and the playing state, and read any related tempo or bar signature
    // information from the ARA model directly (MusicalContext).
    positionInfo.setIsPlaying (true);

    sampleRate = rate;
    numChannels = (unsigned int) numChans;
    bitsPerSample = 32;
    usesFloatingPointData = true;

    auto* documentController = (! playbackRegions.empty())
                                   ? playbackRegions.front()->getDocumentController<ARADocumentController>()
                                   : nullptr;

    playbackRenderer.reset (documentController ? static_cast<ARAPlaybackRenderer*> (documentController->doCreatePlaybackRenderer())
                                               : nullptr);

    if (playbackRenderer != nullptr)
    {
        double regionsStartTime = std::numeric_limits<double>::max();
        double regionsEndTime = std::numeric_limits<double>::lowest();

        for (const auto& playbackRegion : playbackRegions)
        {
            jassert (playbackRegion->getDocumentController() == documentController);
            auto playbackRegionTimeRange = playbackRegion->getTimeRange (ARAPlaybackRegion::IncludeHeadAndTail::yes);
            regionsStartTime = jmin (regionsStartTime, playbackRegionTimeRange.getStart());
            regionsEndTime = jmax (regionsEndTime, playbackRegionTimeRange.getEnd());

            playbackRenderer->addPlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
            playbackRegion->addListener (this);
        }

        startInSamples = (int64) (regionsStartTime * sampleRate + 0.5);
        lengthInSamples = (int64) ((regionsEndTime - regionsStartTime) * sampleRate + 0.5);

        playbackRenderer->prepareToPlay (rate,
                                         maximumBlockSize,
                                         numChans,
                                         AudioProcessor::ProcessingPrecision::singlePrecision,
                                         ARARenderer::AlwaysNonRealtime::yes);
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

bool ARAPlaybackRegionReader::readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                                           int64 startSampleInFile, int numSamples)
{
    bool success = false;
    bool needClearSamples = true;

    const ScopedTryReadLock readLock (lock);

    if (readLock.isLocked())
    {
        if (isValid())
        {
            success = true;
            needClearSamples = false;
            positionInfo.setTimeInSamples (startSampleInFile + startInSamples);

            while (numSamples > 0)
            {
                const int numSliceSamples = jmin (numSamples, maximumBlockSize);
                AudioBuffer<float> buffer ((float **) destSamples, numDestChannels, startOffsetInDestBuffer, numSliceSamples);
                positionInfo.setTimeInSeconds (static_cast<double> (*positionInfo.getTimeInSamples()) / sampleRate);
                success &= playbackRenderer->processBlock (buffer, AudioProcessor::Realtime::no, positionInfo);
                numSamples -= numSliceSamples;
                startOffsetInDestBuffer += numSliceSamples;
                positionInfo.setTimeInSamples (*positionInfo.getTimeInSamples() + numSliceSamples);
            }
        }
    }

    if (needClearSamples)
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            FloatVectorOperations::clear ((float *) destSamples[chan_i], numSamples);

    return success;
}

void ARAPlaybackRegionReader::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties)
{
    jassert (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    if ((playbackRegion->getStartInAudioModificationTime() != newProperties->startInModificationTime)
        || (playbackRegion->getDurationInAudioModificationTime() != newProperties->durationInModificationTime)
        || (playbackRegion->getStartInPlaybackTime() != newProperties->startInPlaybackTime)
        || (playbackRegion->getDurationInPlaybackTime() != newProperties->durationInPlaybackTime)
        || (playbackRegion->isTimestretchEnabled() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretch) != 0))
        || (playbackRegion->isTimeStretchReflectingTempo() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretchReflectingTempo) != 0))
        || (playbackRegion->hasContentBasedFadeAtHead() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtHead) != 0))
        || (playbackRegion->hasContentBasedFadeAtTail() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtTail) != 0)))
    {
        invalidate();
    }
}

void ARAPlaybackRegionReader::didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion,
                                                              ARAContentUpdateScopes scopeFlags)
{
    jassertquiet (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    // Invalidate if the audio signal is changed
    if (scopeFlags.affectSamples())
        invalidate();
}

void ARAPlaybackRegionReader::willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion)
{
    jassertquiet (ARA::contains (playbackRenderer->getPlaybackRegions(), playbackRegion));

    invalidate();
}

} // namespace juce
