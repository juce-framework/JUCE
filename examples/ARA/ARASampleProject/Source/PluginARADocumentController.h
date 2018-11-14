#pragma once

#include "JuceHeader.h"

//==============================================================================
/** 
    ARA DocumentController class for ARA sample project
    This is our plug-in's document controller implementation, which will 
    be the central point of communication between the ARA host and our plug-in
*/
class ARASampleProjectDocumentController : public juce::ARADocumentController
{
public:
    ARASampleProjectDocumentController() noexcept;
    
    // allow creating playback renderers at will so we can 
    AudioFormatReader* createRegionSequenceReader (ARA::PlugIn::RegionSequence* regionSequence);

protected:
    // ARA class creation overrides
    ARA::PlugIn::EditorView* doCreateEditorView() noexcept override;
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;

private:
    // Thread used by buffering audio sources to read samples from the host
    std::unique_ptr<TimeSliceThread> araAudioSourceReadingThread;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectDocumentController)
};

// This class wraps ARASampleProjectPlaybackRenderer inside a
// juce::AudioFormatReader to conveniently read region sequence samples
// TODO JUCE_ARA
// How should we handle model graph changes? We can use playback region
// property updates as a means of adding and removing playback regions, 
// and we'll have to handle region destruction notifications as well.
// in terms of audio source invalidations - those should be handled by
// the underlying audio source reader, I think
class ARARegionSequenceReader : public AudioFormatReader, 
                                ARAPlaybackRegion::Listener
{
public:
    ARARegionSequenceReader (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRenderer* playbackRenderer);
    ~ARARegionSequenceReader();

    bool readSamples (
        int** destSamples,
        int numDestChannels,
        int startOffsetInDestBuffer,
        int64 startSampleInFile,
        int numSamples) override;

    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept override;
    void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept override;

private:
    ARA::PlugIn::RegionSequence* regionSequence;
    ARA::PlugIn::PlaybackRenderer* playbackRenderer;
    ReadWriteLock lock;
};
