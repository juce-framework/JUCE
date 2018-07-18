#include "AraTestDocumentController.h"
#include "AraTestPlaybackRenderer.h"
#include "AraTestAudioSource.h"

namespace ARA
{
namespace PlugIn
{

void AraTestPlaybackRenderer::renderPlaybackRegions(float** ppOutput, int channelCount, ARASampleRate sampleRate,
							ARASamplePosition samplePosition, ARASampleCount samplesToRender, bool isPlayingBack)
{
	AraTestDocumentController* docController = (AraTestDocumentController*)getDocumentController ();

	// initialize output buffers with silence, in case no viable playback region intersects with the
	// current buffer, or if the model is currently not accessible due to being edited.
	for (int c = 0; c < channelCount; ++c)
		memset(ppOutput[c], 0, sizeof(float) * (size_t)samplesToRender);

	// only output samples while host is playing back
	if (!isPlayingBack)
		return;

	// flag that we've started rendering to prevent the document from being edited while in this callback
	// see TestDocumentController for details.
	if (docController->onRendererBeginsAccessingModelGraph (this))
	{
		ARASamplePosition sampleEnd = samplePosition + samplesToRender;
		for (auto playbackRegion : getPlaybackRegions ())
		{
			AraTestAudioSource* audioSource = (AraTestAudioSource*)playbackRegion->getAudioModification ()->getAudioSource ();

			// this simplified test code "rendering" only produces audio if sample rate and channel count match
			if ((audioSource->getChannelCount () != channelCount) || (audioSource->getSampleRate () != sampleRate))
				continue;

			// evaluate region borders in song time, calculate sample range to copy in song time
			ARASamplePosition regionStartSample = playbackRegion->getStartInSongInSamples (sampleRate);
			if (sampleEnd < regionStartSample)
				continue; 

			ARASamplePosition regionEndSample = playbackRegion->getEndInSongInSamples (sampleRate);
			if (regionEndSample < samplePosition)
				continue;

			ARASamplePosition startSongSample = std::max (regionStartSample, samplePosition);
			ARASamplePosition endSongSample = std::min (regionEndSample, sampleEnd);

			// calculate offset between song and audio source samples, clip at region borders in audio source samples
			// (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
			ARASamplePosition offsetToPlaybackRegionSamples = playbackRegion->getStartInAudioModificationInSamples () - regionStartSample;

			ARASamplePosition startAvailableSourceSamples = std::max ((ARASamplePosition)0, playbackRegion->getStartInAudioModificationInSamples ());
			ARASamplePosition endAvailableSourceSamples = std::min (audioSource->getSampleCount (), playbackRegion->getEndInAudioModificationInSamples ());

			startSongSample = std::max (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegionSamples);
			endSongSample = std::min (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegionSamples);

			// add samples from audio source
			for (ARASamplePosition posInSong = startSongSample; posInSong < endSongSample; ++posInSong)
			{
				ARASamplePosition posInBuffer = posInSong - samplePosition;
				ARASamplePosition posInSource = posInSong + offsetToPlaybackRegionSamples;
				for (int c = 0; c < audioSource->getChannelCount (); ++c)
					ppOutput[c][posInBuffer] += audioSource->getChannelBuffer (c)[posInSource];
			}
		}

		// let the document controller know we're done
		docController->onRendererEndsAccessingModelGraph (this);
	}
}

}	// namespace PlugIn
}	// namespace Ara
