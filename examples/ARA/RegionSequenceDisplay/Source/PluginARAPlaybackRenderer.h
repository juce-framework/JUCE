/*
  ==============================================================================

    PluginARAPlaybackRenderer.h
    Created: 2 Nov 2018 2:24:19pm
    Author:  john

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include <map>

class ARASampleProjectPlaybackRenderer : public ARA::PlugIn::PlaybackRenderer
{
public:
    ARASampleProjectPlaybackRenderer (ARADocumentController* documentController, TimeSliceThread& timeSliceThread, int bufferingSize);

    void renderPlaybackRegions (AudioBuffer<float>& buffer, ARA::ARASampleRate sampleRate, ARA::ARASamplePosition samplePosition, bool isPlayingBack);

protected:
    virtual void didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

private:
    // time slice thread reference used for reading audio source samples and the size of its buffer
    TimeSliceThread& araSampleThread;
    int araSampleBufferSize;

    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<ARAAudioSource*, std::unique_ptr<BufferingAudioSource>> _audioSourceMap;
};

