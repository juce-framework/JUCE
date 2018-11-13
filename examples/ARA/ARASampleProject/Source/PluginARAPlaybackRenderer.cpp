#include "PluginARAPlaybackRenderer.h"

ARASampleProjectPlaybackRenderer::ARASampleProjectPlaybackRenderer (ARADocumentController* documentController, TimeSliceThread& timeSliceThread, int bufferSize)
: ARA::PlugIn::PlaybackRenderer (documentController), 
  ARAAudioSourceUpdateListener(documentController),
  sampleReadingThread (timeSliceThread),
  sampleBufferSize (bufferSize)
{}

// every time we add a playback region, make sure we have a buffered audio source reader for it
// we'll use this reader to pull samples from our ARA host and render them back in the audio thread
void ARASampleProjectPlaybackRenderer::ensureReadersForAllPlaybackRegions () 
{
    for (ARA::PlugIn::PlaybackRegion* playbackRegion : getPlaybackRegions ())
    {
        ARAAudioSource* audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification ()->getAudioSource ());
        auto itAudioSource = audioSourceReaders.find (audioSource);
        if (itAudioSource == audioSourceReaders.end () || itAudioSource->second == nullptr)
        {
            audioSourceReaders.emplace (audioSource, audioSource->createBufferingAudioSource (sampleReadingThread, sampleBufferSize));
            audioSourceReaders[audioSource]->prepareToPlay (128, audioSource->getSampleRate ());
        }
    }
}

// this function renders playback regions in the ARA document that have been
// a) added to this playback renderer instance and
// b) lie within the time range of samples being renderered (in project time)
// effectively making this plug-in a pass-through renderer
void ARASampleProjectPlaybackRenderer::renderPlaybackRegions (AudioBuffer<float>& buffer, ARA::ARASampleRate sampleRate, ARA::ARASamplePosition samplePosition, bool isPlayingBack)
{
    // zero the samples and get out if we the host is not playing back
    if (isPlayingBack == false)
    {
        for (int c = 0; c < buffer.getNumChannels (); c++)
            FloatVectorOperations::clear (buffer.getArrayOfWritePointers ()[c], buffer.getNumSamples ());
        return;
    }

    // render back playback regions that lie within this range using our buffered ARA samples
    using namespace ARA;
    ARASamplePosition sampleEnd = samplePosition + buffer.getNumSamples ();
    for (PlugIn::PlaybackRegion* playbackRegion : getPlaybackRegions ())
    {
        // get the audio source for this region and make sure we have an audio source reader for it
        ARAAudioSource* audioSource = static_cast<ARAAudioSource*> (playbackRegion->getAudioModification ()->getAudioSource ());
        if (audioSourceReaders.count (audioSource) == 0)
            continue;

        // render silence if access is currently disabled
        if (!audioSource->isSampleAccessEnabled ())
            continue;

        // this simplified test code "rendering" only produces audio if sample rate and channel count match
        if ((audioSource->getChannelCount () != buffer.getNumChannels ()) || (audioSource->getSampleRate () != sampleRate))
            continue;

        // evaluate region borders in song time, calculate sample range to copy in song time
        ARASamplePosition regionStartSample = playbackRegion->getStartInPlaybackSamples (sampleRate);
        if (sampleEnd <= regionStartSample)
            continue;

        ARASamplePosition regionEndSample = playbackRegion->getEndInPlaybackSamples (sampleRate);
        if (regionEndSample <= samplePosition)
            continue;

        ARASamplePosition startSongSample = std::max (regionStartSample, samplePosition);
        ARASamplePosition endSongSample = std::min (regionEndSample, sampleEnd);

        // calculate offset between song and audio source samples, clip at region borders in audio source samples
        // (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
        ARASamplePosition offsetToPlaybackRegion = playbackRegion->getStartInAudioModificationSamples () - regionStartSample;

        // clamp sample ranges within the range we're rendering
        ARASamplePosition startAvailableSourceSamples = std::max<ARASamplePosition> (0, playbackRegion->getStartInAudioModificationSamples ());
        ARASamplePosition endAvailableSourceSamples = std::min (audioSource->getSampleCount (), playbackRegion->getEndInAudioModificationSamples ());

        startSongSample = std::max (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
        endSongSample = std::min (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);

        // use the buffered audio source reader to read samples into the audio block
        AudioSourceChannelInfo channelInfo (&buffer, (int) (startSongSample - samplePosition), (int) (endSongSample - startSongSample));
        audioSourceReaders[audioSource]->setNextReadPosition (startSongSample + offsetToPlaybackRegion);
        audioSourceReaders[audioSource]->getNextAudioBlock (channelInfo);
    }
}

void ARASampleProjectPlaybackRenderer::willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> /*newProperties*/) noexcept
{
    audioSourceReaders.erase (static_cast<ARAAudioSource*>(audioSource));
}

void ARASampleProjectPlaybackRenderer::willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    ARAAudioSource* araAudioSource = static_cast<ARAAudioSource*>(audioSource);
    auto it = audioSourceReaders.find (araAudioSource);
    if (it != audioSourceReaders.end () && enable == false)
        it->second.reset ();
}

void ARASampleProjectPlaybackRenderer::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* /*range*/, ARA::ARAContentUpdateFlags /*flags*/) noexcept
{
    ARAAudioSource* araAudioSource = static_cast<ARAAudioSource*>(audioSource);
    auto it = audioSourceReaders.find (araAudioSource);
    if (it != audioSourceReaders.end ())
        it->second.reset ();
}

void ARASampleProjectPlaybackRenderer::willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    audioSourceReaders.erase (static_cast<ARAAudioSource*>(audioSource));
}

void ARASampleProjectPlaybackRenderer::didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion* /*playbackRegion*/) noexcept
{
    ensureReadersForAllPlaybackRegions ();
}