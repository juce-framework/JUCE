/*
  ==============================================================================

    PluginARAPlaybackRenderer.cpp
    Created: 2 Nov 2018 2:24:04pm
    Author:  john

  ==============================================================================
*/

#include "PluginARAPlaybackRenderer.h"

ARASampleProjectPlaybackRenderer::ARASampleProjectPlaybackRenderer (ARADocumentController* documentController, TimeSliceThread& timeSliceThread, int bufferingSize)
: ARA::PlugIn::PlaybackRenderer (documentController),
  araSampleThread (timeSliceThread),
  araSampleBufferSize (bufferingSize)
{}

void ARASampleProjectPlaybackRenderer::didAddPlaybackRegion(ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    ARAAudioSource* audioSource = static_cast<ARAAudioSource*>(playbackRegion->getAudioModification()->getAudioSource());
    if (_audioSourceMap.count(audioSource) == 0)
    {
        _audioSourceMap.emplace(audioSource, audioSource->createBufferingAudioSource(araSampleThread, araSampleBufferSize));
        _audioSourceMap[audioSource]->prepareToPlay(128, audioSource->getSampleRate());
    }
}

void ARASampleProjectPlaybackRenderer::renderPlaybackRegions (AudioBuffer<float>& buffer, ARA::ARASampleRate sampleRate, ARA::ARASamplePosition samplePosition, bool isPlayingBack)
{
    if (!isPlayingBack)
    {
        for (int c = 0; c < buffer.getNumChannels (); c++)
            FloatVectorOperations::clear(buffer.getArrayOfWritePointers()[c], buffer.getNumSamples());
    }
    else
    {
        using namespace ARA;
        ARA::ARASamplePosition sampleEnd = samplePosition + buffer.getNumSamples();
        for (ARA::PlugIn::PlaybackRegion* playbackRegion : getPlaybackRegions())
        {
            ARAAudioSource* audioSource = static_cast<ARAAudioSource*>(playbackRegion->getAudioModification()->getAudioSource());
            if (_audioSourceMap.count(audioSource) == 0)
                continue;

            // render silence if access is currently disabled
            if (!audioSource->isSampleAccessEnabled ())
                continue;

            // this simplified test code "rendering" only produces audio if sample rate and channel count match
            if ((audioSource->getChannelCount () != buffer.getNumChannels()) || (audioSource->getSampleRate () != sampleRate))
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

            ARASamplePosition startAvailableSourceSamples = std::max ((ARASamplePosition) 0, playbackRegion->getStartInAudioModificationSamples ());
            ARASamplePosition endAvailableSourceSamples = std::min (audioSource->getSampleCount (), playbackRegion->getEndInAudioModificationSamples ());

            startSongSample = std::max (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
            endSongSample = std::min (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);

            AudioSourceChannelInfo channelInfo (&buffer, (int) (startSongSample - samplePosition), (int) (endSongSample - startSongSample));
            _audioSourceMap[audioSource]->setNextReadPosition(startSongSample + offsetToPlaybackRegion);
            _audioSourceMap[audioSource]->getNextAudioBlock(channelInfo);
        }
    }
}
