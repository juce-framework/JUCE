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
class ARASampleProjectPlaybackRenderer : public ARA::PlugIn::PlaybackRenderer,
                                         ARAAudioSourceUpdateListener
{
public:
    ARASampleProjectPlaybackRenderer (ARADocumentController* documentController, TimeSliceThread& timeSliceThread, int bufferingSize);

    // called from doEndEditing to ensure that all of our playback regions have audio source readers
    void ensureReadersForAllPlaybackRegions ();

    // render playback regions added to this render if they fall within the range of samples being rendered
    void renderPlaybackRegions (AudioBuffer<float>& buffer, ARA::ARASampleRate sampleRate, ARA::ARASamplePosition samplePosition, bool isPlayingBack);

    // hooks we use to invalidate our audio sources
    void willUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> newProperties) noexcept override;
    void willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept override;
    void doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept override;
    void willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept override;

protected:

    // use this hook to verify that we have audio source readers for this playback region
    virtual void didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

private:
    // time slice thread reference used for reading audio source samples and the size of its buffer
    TimeSliceThread& sampleReadingThread;
    int sampleBufferSize;

    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<ARAAudioSource*, std::unique_ptr<BufferingAudioSource>> audioSourceReaders;
};

