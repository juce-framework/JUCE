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

        tempBuffer.reset (new AudioBuffer<float> (getNumChannels(), getMaxSamplesPerBlock()));
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

    // zero out samples first, then add region output on top
    for (int c = 0; c < buffer.getNumChannels(); c++)
        FloatVectorOperations::clear (buffer.getArrayOfWritePointers()[c], buffer.getNumSamples());

    if (! isPlayingBack)
        return success;

    // render back playback regions that lie within this range using our buffered ARA samples
    using namespace ARA;
    ARASamplePosition sampleStart = timeInSamples;
    ARASamplePosition sampleEnd = timeInSamples + buffer.getNumSamples();
    for (PlugIn::PlaybackRegion* playbackRegion : getPlaybackRegions())
    {
        // get the audio source for this region and make sure we have an audio source reader for it
        auto audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource());
        if (audioSourceReaders.count (audioSource) == 0)
            continue;

        // render silence if access is currently disabled
        if (! audioSource->isSampleAccessEnabled())
            continue;

        // this simplified test code "rendering" only produces audio if sample rate and channel count match
        if ((audioSource->getChannelCount() != getNumChannels()) || (audioSource->getSampleRate() != getSampleRate()))
            continue;

        // evaluate region borders in song time, calculate sample range to copy in song time
        ARASamplePosition regionStartSample = playbackRegion->getStartInPlaybackSamples (getSampleRate());
        if (sampleEnd <= regionStartSample)
            continue;

        ARASamplePosition regionEndSample = playbackRegion->getEndInPlaybackSamples (getSampleRate());
        if (regionEndSample <= sampleStart)
            continue;

        ARASamplePosition startSongSample = jmax (regionStartSample, sampleStart);
        ARASamplePosition endSongSample = jmin (regionEndSample, sampleEnd);

        // calculate offset between song and audio source samples, clip at region borders in audio source samples
        // (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
        ARASamplePosition offsetToPlaybackRegion = playbackRegion->getStartInAudioModificationSamples() - regionStartSample;

        // clamp sample ranges within the range we're rendering
        ARASamplePosition startAvailableSourceSamples = jmax<ARASamplePosition> (0, playbackRegion->getStartInAudioModificationSamples());
        ARASamplePosition endAvailableSourceSamples = jmin (audioSource->getSampleCount(), playbackRegion->getEndInAudioModificationSamples());

        startSongSample = jmax (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
        endSongSample = jmin (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);

        // use the buffered audio source reader to read samples into our local read buffer
        int startInDestBuffer = (int) (startSongSample - sampleStart);
        int startInSource = (int) (startSongSample + offsetToPlaybackRegion);
        int numSamplesToRead = (int) (endSongSample - startSongSample);
        
        // set reader timeout depending on real time playback
        if (isNonRealtime)
            audioSourceReaders[audioSource]->setReadTimeout (2000); // TODO JUCE_ARA I set at a high value arbitrarily, but we should pick a better timeout
        else
            audioSourceReaders[audioSource]->setReadTimeout (-1);

        success &= audioSourceReaders[audioSource]->
            read ((int* const*) tempBuffer->getArrayOfWritePointers(), tempBuffer->getNumChannels(), startInSource, numSamplesToRead, true);
        
        // mix this region's samples into the output buffer
        for (int c = 0; c < getNumChannels(); c++)
            FloatVectorOperations::add (buffer.getWritePointer(c) + startInDestBuffer, tempBuffer->getReadPointer(c), numSamplesToRead);
    }

    return success;
}
