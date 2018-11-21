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

    void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock) override;
    bool processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack) override;

protected:
    // this hook is used to make sure we have an audio source reader for this playback region
    void didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;
    void willRemovePlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

private:
    // calculate how big our read-ahead-buffer needs to be
    int getReadAheadSize() const;
    std::unique_ptr<BufferingAudioReader> createBufferingAudioSourceReader (ARAAudioSource* audioSource);

private:
    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<ARAAudioSource*, std::unique_ptr<BufferingAudioReader>> audioSourceReaders;

    std::vector<float> localReadBuffer;
    std::vector<int*> localReadBufferPointers;
};
