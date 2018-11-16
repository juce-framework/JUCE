#pragma once

#include "JuceHeader.h"
#include <map>

//==============================================================================
/** 
    PlaybackRenderer implementation of the ARA sample project
    This class fulfils the ARA PlaybackRenderer role of a plug-in instance, and
    will be used to render audio samples for playback by the host. In this simple 
    demo we're using a buffered ARA sample reader to pull audio samples from the host 
    and render them back, effectively making this ARA enabled pass-through renderer
*/
class ARASampleProjectPlaybackRenderer : public ARAPlaybackRenderer
{
public:
    ARASampleProjectPlaybackRenderer (ARADocumentController* documentController);

    // render playback regions added to this render if they fall within the range of samples being rendered
    void processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack) override;

protected:
    // use this hook to verify that we have audio source readers for this playback region
    void didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

private:
    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<ARAAudioSource*, std::unique_ptr<BufferingAudioSource>> audioSourceReaders;
};
