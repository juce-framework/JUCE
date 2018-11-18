#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectPlaybackRenderer.h"

ARASampleProjectPlaybackRenderer::ARASampleProjectPlaybackRenderer (ARADocumentController* documentController)
: ARAPlaybackRenderer (documentController)
{}

int ARASampleProjectPlaybackRenderer::getReadAheadSize() const
{
    int readAheadSizeBySampleRate = (int) (2.0 * getSampleRate() + 0.5);
    int readAheadSizeByBlockSize = 8 * getMaxSamplesPerBlock();
    return jmax (readAheadSizeBySampleRate, readAheadSizeByBlockSize);
}

std::unique_ptr<BufferingAudioSource> ARASampleProjectPlaybackRenderer::createBufferingAudioSourceReader (ARAAudioSource* audioSource)
{
    auto documentController = static_cast<ARASampleProjectDocumentController*> (audioSource->getDocument()->getDocumentController());
    auto newSourceReader = documentController->createBufferingAudioSourceReader (audioSource, documentController->getAudioSourceReadingThread(), getReadAheadSize());
    newSourceReader->prepareToPlay (getMaxSamplesPerBlock(), audioSource->getSampleRate());
    return std::unique_ptr<BufferingAudioSource> (newSourceReader);
}

void ARASampleProjectPlaybackRenderer::prepareToPlay (double newSampleRate, int newMaxSamplesPerBlock)
{
    auto oldSampleRate = getSampleRate();
    auto oldMaxSamplesPerBlock = getMaxSamplesPerBlock();
    auto oldReadAheadSize = getReadAheadSize();

    ARAPlaybackRenderer::prepareToPlay(newSampleRate, newMaxSamplesPerBlock);

    if ((oldSampleRate != getSampleRate()) ||
        (oldMaxSamplesPerBlock != getMaxSamplesPerBlock()) ||
        (oldReadAheadSize != getReadAheadSize()))
    {
        for (auto& readerPair : audioSourceReaders)
            readerPair.second = createBufferingAudioSourceReader (readerPair.first);
    }
}

// this function renders playback regions in the ARA document that have been
// a) added to this playback renderer instance and
// b) lie within the time range of samples being renderered (in project time)
// effectively making this plug-in a pass-through renderer
void ARASampleProjectPlaybackRenderer::processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack)
{
    jassert (buffer.getNumSamples() <= getMaxSamplesPerBlock());

    // zero the samples and get out if we the host is not playing back
    if (! isPlayingBack)
    {
        for (int c = 0; c < buffer.getNumChannels(); c++)
            FloatVectorOperations::clear (buffer.getArrayOfWritePointers()[c], buffer.getNumSamples());
        return;
    }

    // render back playback regions that lie within this range using our buffered ARA samples
    using namespace ARA;
    ARASamplePosition sampleStart = timeInSamples;
    ARASamplePosition sampleEnd = timeInSamples + buffer.getNumSamples();
    for (PlugIn::PlaybackRegion* playbackRegion : getPlaybackRegions())
    {
        // get the audio source for this region and make sure we have an audio source reader for it
        ARAAudioSource* audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource());
        if (audioSourceReaders.count (audioSource) == 0)
            continue;

        // render silence if access is currently disabled
        if (! audioSource->isSampleAccessEnabled())
            continue;

        // this simplified test code "rendering" only produces audio if sample rate and channel count match
        if ((audioSource->getChannelCount() != buffer.getNumChannels()) || (audioSource->getSampleRate() != getSampleRate()))
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

        // use the buffered audio source reader to read samples into the audio block
        AudioSourceChannelInfo channelInfo (&buffer, (int) (startSongSample - sampleStart), (int) (endSongSample - startSongSample));
        audioSourceReaders[audioSource]->setNextReadPosition (startSongSample + offsetToPlaybackRegion);
        audioSourceReaders[audioSource]->getNextAudioBlock (channelInfo);
    }
}

// every time we add a playback region, make sure we have a buffered audio source reader for it
// we'll use this reader to pull samples from our ARA host and render them back in the audio thread
void ARASampleProjectPlaybackRenderer::didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    ARAAudioSource* audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource());
    if (audioSourceReaders.count (audioSource) == 0)
        audioSourceReaders.emplace (audioSource, createBufferingAudioSourceReader (audioSource));
}

// we can delete the reader associated with this playback region's audio source
// if no other playback regions in the playback renderer share the same audio source
void ARASampleProjectPlaybackRenderer::willRemovePlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    auto audioSource = playbackRegion->getAudioModification()->getAudioSource();
    for (auto otherPlaybackRegion : getPlaybackRegions())
        if (playbackRegion != otherPlaybackRegion)
            if (otherPlaybackRegion->getAudioModification()->getAudioSource() == audioSource)
                return;

    audioSourceReaders.erase (static_cast<ARAAudioSource*> (audioSource));
}
