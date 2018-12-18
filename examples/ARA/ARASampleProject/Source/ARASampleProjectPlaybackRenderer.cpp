#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectPlaybackRenderer.h"

ARASampleProjectPlaybackRenderer::ARASampleProjectPlaybackRenderer (ARADocumentController* documentController)
: ARAPlaybackRenderer (documentController)
{}

void ARASampleProjectPlaybackRenderer::prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock, bool mayBeRealtime)
{
    bool needAllocate = ! isPrepared() ||
                        (newSampleRate != getSampleRate()) ||
                        (newNumChannels != getNumChannels()) ||
                        (newMaxSamplesPerBlock != getMaxSamplesPerBlock());

    ARAPlaybackRenderer::prepareToPlay (newSampleRate, newNumChannels, newMaxSamplesPerBlock, mayBeRealtime);

    if (needAllocate)
    {
        audioSourceReaders.clear();

        const auto documentController = getDocumentController<ARASampleProjectDocumentController>();

        for (auto playbackRegion : getPlaybackRegions())
        {
            auto audioSource = playbackRegion->getAudioModification()->getAudioSource<ARAAudioSource>();
            if (audioSourceReaders.count (audioSource) == 0)
            {
                auto sourceReader = documentController->createAudioSourceReader (audioSource);

                if (mayBeRealtime)
                {
                    // if we're being used in real-time, wrap our source reader in buffering
                    // reader  to avoid blocking while reading samples in processBlock
                    const int readAheadSizeBySampleRate = roundToInt (2.0 * getSampleRate());
                    const int readAheadSizeByBlockSize = 8 * getMaxSamplesPerBlock();
                    const int readAheadSize = jmax (readAheadSizeBySampleRate, readAheadSizeByBlockSize);

                    sourceReader = new BufferingAudioReader (sourceReader, documentController->getAudioSourceReadingThread(), readAheadSize);
                }

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
            auto audioSource = playbackRegion->getAudioModification()->getAudioSource<ARAAudioSource>();
            auto readerIt = audioSourceReaders.find(audioSource);
            if (readerIt == audioSourceReaders.end())
            {
                success = false;
                continue;
            }
            auto& reader = readerIt->second;

            // render silence if access is currently disabled
            // (the audio reader deals with this internally too, checking it here is merely an optimization)
            if (! audioSource->isSampleAccessEnabled())
            {
                success = false;
                continue;
            }

            // this simplified test code "rendering" only produces audio if sample rate and channel count match
            if ((audioSource->getChannelCount() != getNumChannels()) || (audioSource->getSampleRate() != getSampleRate()))
                continue;

            // evaluate region borders in song time, calculate sample range to copy in song time
            int64 regionStartSample = playbackRegion->getStartInPlaybackSamples (getSampleRate());
            if (sampleEnd <= regionStartSample)
                continue;

            int64 regionEndSample = playbackRegion->getEndInPlaybackSamples (getSampleRate());
            if (regionEndSample <= sampleStart)
                continue;

            int64 startSongSample = jmax (regionStartSample, sampleStart);
            int64 endSongSample = jmin (regionEndSample, sampleEnd);

            // calculate offset between song and audio source samples, clip at region borders in audio source samples
            // (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
            int64 offsetToPlaybackRegion = playbackRegion->getStartInAudioModificationSamples() - regionStartSample;

            int64 startAvailableSourceSamples = jmax (0LL, playbackRegion->getStartInAudioModificationSamples());
            int64 endAvailableSourceSamples = jmin (audioSource->getSampleCount(), playbackRegion->getEndInAudioModificationSamples());

            startSongSample = jmax (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
            endSongSample = jmin (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);

            // calculate buffer offsets
            int startInDestBuffer = (int) (startSongSample - sampleStart);
            int64 startInSource = startSongSample + offsetToPlaybackRegion;
            int numSamplesToRead = (int) (endSongSample - startSongSample);

            // if we're using a buffering reader then set the appropriate timeout
            BufferingAudioReader* bufferingReader = dynamic_cast<BufferingAudioReader*> (reader.get());
            if (bufferingReader != nullptr)
                bufferingReader->setReadTimeout (isNonRealtime ? 100 : 0);

            // read samples
            bool bufferSuccess;
            if (didRenderFirstRegion)
            {
                // target buffer is initialized, so read region samples into local buffer
                bufferSuccess = reader->read (tempBuffer.get(), 0, numSamplesToRead, startInSource, true, true);

                // if successful, mix local buffer into the output buffer
                if (bufferSuccess)
                {
                    for (int c = 0; c < getNumChannels(); ++c)
                        buffer.addFrom(c, startInDestBuffer, *tempBuffer, c, 0, numSamplesToRead);
                }
            }
            else
            {
                // this is the first region to hit the buffer, so initialize its samples
                bufferSuccess = reader->read (&buffer, startInDestBuffer, numSamplesToRead, startInSource, true, true);

                // if successful, clear any excess at start or end of the region and mark successful buffer initialization
                if (bufferSuccess)
                {
                    if (startInDestBuffer != 0)
                        buffer.clear(0, startInDestBuffer);

                    int samplesWritten = startInDestBuffer + numSamplesToRead;
                    int remainingSamples = buffer.getNumSamples() - samplesWritten;
                    if (remainingSamples != 0)
                        buffer.clear(samplesWritten, remainingSamples);

                    didRenderFirstRegion = true;
                }
            }
            success &= bufferSuccess;
        }
    }

    // if no playback or no region did intersect, clear buffer now
    if (! didRenderFirstRegion)
        buffer.clear();

    return success;
}
