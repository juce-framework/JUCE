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
class ARASampleProjectPlaybackRenderer  : public ARAPlaybackRenderer
{
public:
    ARASampleProjectPlaybackRenderer (ARADocumentController* documentController);

    void prepareToPlay (double newSampleRate, int newNumChannels, int newMaxSamplesPerBlock, bool mayBeRealtime) override;
    bool processBlock (AudioBuffer<float>& buffer, int64 timeInSamples, bool isPlayingBack, bool isNonRealtime) override;
    void releaseResources() override;

private:
    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<ARAAudioSource*, std::unique_ptr<AudioFormatReader>> audioSourceReaders;

    // temp buffers to use for summing signals if rendering multiple regions
    std::unique_ptr<AudioBuffer<float>> tempBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectPlaybackRenderer)
};
