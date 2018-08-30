//------------------------------------------------------------------------------
//! \file        ARATestPlaybackRenderer.cpp
//! \description playback renderer implementation for the ARA sample plug-in
//! \project     ARA SDK, examples
//------------------------------------------------------------------------------
// Copyright (c) 2012-2018, Celemony Software GmbH, All Rights Reserved.
//
// License & Disclaimer:
//
// This Software Development Kit (SDK) may not be distributed in parts or
// its entirety without prior written agreement of Celemony Software GmbH.
//
// This SDK must not be used to modify, adapt, reproduce or transfer any
// software and/or technology used in any Celemony and/or Third-party
// application and/or software module (hereinafter referred as "the Software");
// to modify, translate, reverse engineer, decompile, disassemble or create
// derivative works based on the Software (except to the extent applicable laws
// specifically prohibit such restriction) or to lease, assign, distribute or
// otherwise transfer rights to the Software.
// Neither the name of the Celemony Software GmbH nor the names of its
// contributors may be used to endorse or promote products derived from this
// SDK without specific prior written permission.
//
// This SDK is provided by Celemony Software GmbH "as is" and any express or
// implied warranties, including, but not limited to, the implied warranties of
// non-infringement, merchantability and fitness for a particular purpose are
// disclaimed.
// In no event shall Celemony Software GmbH be liable for any direct, indirect,
// incidental, special, exemplary, or consequential damages (including, but not
// limited to, procurement of substitute goods or services; loss of use, data,
// or profits; or business interruption) however caused and on any theory of
// liability, whether in contract, strict liability, or tort (including
// negligence or otherwise) arising in any way out of the use of this software,
// even if advised of the possibility of such damage.
// Where the liability of Celemony Software GmbH is ruled out or restricted,
// this will also apply for the personal liability of employees, representatives
// and vicarious agents.
// The above restriction of liability will not apply if the damages suffered
// are attributable to willful intent or gross negligence or in the case of
// physical injury.
//------------------------------------------------------------------------------

#include "ARATestPlaybackRenderer.h"
#include "ARATestDocumentController.h"
#include "ARATestAudioSource.h"

namespace ARA
{
namespace PlugIn
{

void ARATestPlaybackRenderer::renderPlaybackRegions (float** ppOutput, ARAChannelCount channelCount, ARASampleRate sampleRate,
							ARASamplePosition samplePosition, ARASampleCount samplesToRender, bool isPlayingBack)
{
	ARATestDocumentController* docController = (ARATestDocumentController*)getDocumentController ();

	// initialize output buffers with silence, in case no viable playback region intersects with the
	// current buffer, or if the model is currently not accessible due to being edited.
	for (ARAChannelCount c = 0; c < channelCount; ++c)
		memset (ppOutput[c], 0, sizeof(float) * (size_t)samplesToRender);

	// only output samples while host is playing back
	if (!isPlayingBack)
		return;

	// flag that we've started rendering to prevent the document from being edited while in this callback
	// see TestDocumentController for details.
	if (docController->rendererWillAccessModelGraph (this))
	{
		ARASamplePosition sampleEnd = samplePosition + samplesToRender;
		for (auto playbackRegion : getPlaybackRegions ())
		{
			ARATestAudioSource* audioSource = (ARATestAudioSource*)playbackRegion->getAudioModification ()->getAudioSource ();

			// render silence if access is currently disabled
			if (!audioSource->isSampleAccessEnabled ())
				continue;

			// this simplified test code "rendering" only produces audio if sample rate and channel count match
			if ((audioSource->getChannelCount () != channelCount) || (audioSource->getSampleRate () != sampleRate))
				continue;

			// evaluate region borders in song time, calculate sample range to copy in song time
			ARASamplePosition regionStartSample = playbackRegion->getStartInPlaybackSamples (sampleRate);
			if (sampleEnd < regionStartSample)
				continue; 

			ARASamplePosition regionEndSample = playbackRegion->getEndInPlaybackSamples (sampleRate);
			if (regionEndSample < samplePosition)
				continue;

			ARASamplePosition startSongSample = std::max (regionStartSample, samplePosition);
			ARASamplePosition endSongSample = std::min (regionEndSample, sampleEnd);

			// calculate offset between song and audio source samples, clip at region borders in audio source samples
			// (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
			ARASamplePosition offsetToPlaybackRegionSamples = playbackRegion->getStartInAudioModificationSamples () - regionStartSample;

			ARASamplePosition startAvailableSourceSamples = std::max ((ARASamplePosition)0, playbackRegion->getStartInAudioModificationSamples ());
			ARASamplePosition endAvailableSourceSamples = std::min (audioSource->getSampleCount (), playbackRegion->getEndInAudioModificationSamples ());

			startSongSample = std::max (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegionSamples);
			endSongSample = std::min (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegionSamples);

			// add samples from audio source
			for (ARASamplePosition posInSong = startSongSample; posInSong < endSongSample; ++posInSong)
			{
				ARASamplePosition posInBuffer = posInSong - samplePosition;
				ARASamplePosition posInSource = posInSong + offsetToPlaybackRegionSamples;
				for (ARAChannelCount c = 0; c < audioSource->getChannelCount (); ++c)
					ppOutput[c][posInBuffer] += audioSource->getRenderSampleCacheForChannel (c)[posInSource];
			}
		}

		// let the document controller know we're done
		docController->rendererDidAccessModelGraph (this);
	}
}

}	// namespace PlugIn
}	// namespace ARA
