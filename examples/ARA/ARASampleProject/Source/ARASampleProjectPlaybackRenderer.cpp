#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectPlaybackRenderer.h"

ARASampleProjectPlaybackRenderer::ARASampleProjectPlaybackRenderer (ARADocumentController* documentController)
: ARAPlaybackRenderer (documentController)
{}

void ARASampleProjectPlaybackRenderer::prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock)
{
    bool needAllocate = ! isPrepared() ||
                        (newSampleRate != getSampleRate()) ||
                        (newNumChannels != getNumChannels()) ||
                        (newMaxSamplesPerBlock != getMaxSamplesPerBlock());

    ARAPlaybackRenderer::prepareToPlay (newSampleRate, newNumChannels, newMaxSamplesPerBlock);

    if (needAllocate)
    {
        audioSourceReaders.clear();

        const int readAheadSizeBySampleRate = (int) (2.0 * getSampleRate() + 0.5);
        const int readAheadSizeByBlockSize = 8 * getMaxSamplesPerBlock();
        const int readAheadSize = jmax (readAheadSizeBySampleRate, readAheadSizeByBlockSize);

        const auto documentController = static_cast<ARASampleProjectDocumentController*> (getDocumentController());

        for (auto playbackRegion : getPlaybackRegions())
        {
            auto audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource());
            if (audioSourceReaders.count (audioSource) == 0)
            {
                auto sourceReader = documentController->createBufferingAudioSourceReader (audioSource, readAheadSize);
                audioSourceReaders.emplace (audioSource, sourceReader);
            }
        }

        if (getPlaybackRegions().size() > 1)
            tempBuffer.reset (new AudioBuffer<float> (getNumChannels(), getMaxSamplesPerBlock()));
        else
            tempBuffer.reset();
    }
}

void ARASampleProjectPlaybackRenderer::releaseResources()
{
    audioSourceReaders.clear();
    tempBuffer = nullptr;

    ARAPlaybackRenderer::releaseResources();
}

// this function renders playback regions in the ARA document that have been
// a) added to this playback renderer instance and
// b) lie within the time range of samples being renderered (in project time)
// effectively making this plug-in a pass-through renderer
bool ARASampleProjectPlaybackRenderer::processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack, bool isNonRealtime)
{
    jassert (buffer.getNumSamples() <= getMaxSamplesPerBlock());

    bool success = true;
    bool didRenderFirstRegion = false;

    if (isPlayingBack)
    {
        int64 sampleStart = timeInSamples;
        int64 sampleEnd = timeInSamples + buffer.getNumSamples();
        for (auto playbackRegion : getPlaybackRegions())
        {
            // get the audio source for this region and make sure we have an audio source reader for it
            auto audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource());
            auto readerIt = audioSourceReaders.find(audioSource);
            if (readerIt == audioSourceReaders.end())
                continue;
            auto& reader = readerIt->second;

            // render silence if access is currently disabled
            if (! audioSource->isSampleAccessEnabled())
                continue;

            // this simplified test code "rendering" only produces audio if sample rate and channel count match
            if ((audioSource->getChannelCount() != getNumChannels()) || (audioSource->getSampleRate() != getSampleRate()))
                continue;

            // evaluate region borders in song time
            int64 regionStartSample = playbackRegion->getStartInPlaybackSamples (getSampleRate());
            if (sampleEnd <= regionStartSample)
                continue;

            int64 regionEndSample = playbackRegion->getEndInPlaybackSamples (getSampleRate());
            if (regionEndSample <= sampleStart)
                continue;

            // set reader timeout depending on real time playback
            if (isNonRealtime)
                reader->setReadTimeout (2000); // TODO JUCE_ARA I set at a high value arbitrarily, but we should pick a better timeout
            else
                reader->setReadTimeout (0);

            // calculate offset between song and audio source samples, clip at region borders in audio source samples
            // (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
            int64 offsetToPlaybackRegion = playbackRegion->getStartInAudioModificationSamples() - regionStartSample;

            // optimized read and mix samples
            if (didRenderFirstRegion)
            {
                // clamp song sample ranges within the range we're rendering
                int64 startSongSample = jmax (regionStartSample, sampleStart);
                int64 endSongSample = jmin (regionEndSample, sampleEnd);

                // clamp source sample ranges within the modifiction range
                int64 startAvailableSourceSamples = jmax<int64> (0, playbackRegion->getStartInAudioModificationSamples());
                int64 endAvailableSourceSamples = jmin (audioSource->getSampleCount(), playbackRegion->getEndInAudioModificationSamples());

                // intersect both clamped ranges - this is the range we need to mix into the buffer
                startSongSample = jmax (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
                endSongSample = jmin (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);

                // target buffer is initialized, so read region samples into local buffer
                int startInDestBuffer = (int) (startSongSample - sampleStart);
                int64 startInSource = startSongSample + offsetToPlaybackRegion;
                int numSamplesToRead = (int) (endSongSample - startSongSample);
                bool bufferSuccess = reader->read ((int* const*) tempBuffer->getArrayOfWritePointers(), tempBuffer->getNumChannels(), startInSource, numSamplesToRead, true);

                // if successful, mix local buffer into the output buffer
                if (bufferSuccess)
                {
                    for (int c = 0; c < getNumChannels(); c++)
                        buffer.addFrom(c, startInDestBuffer, *tempBuffer, c, 0, numSamplesToRead);
                }

                success &= bufferSuccess;
            }
            else
            {
                // this is the first region to hit the buffer, so initialize its samples
                // We're reading an entire buffer from the reader - if the region starts or ends
                // inside the buffer, the reader will zero out the excess samples, which is exactly
                // what we need here.
                success &= reader->read (&buffer, 0, buffer.getNumSamples(), sampleStart + offsetToPlaybackRegion, true, true);

                didRenderFirstRegion = true;
            }
        }
    }

    // if no playback or no region did intersect, clear buffer now
    if (! didRenderFirstRegion)
        buffer.clear();

    return success;
}
