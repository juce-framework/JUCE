#pragma once

#include "juce_ARAAudioSource.h"

namespace juce
{

// All these readers follow a common pattern of "invalidation":
//
// Whenever the samples they are reading are altered, the readers become invalid and will stop
// accessing the model graph. These alterations are model edits such as property changes,
// content changes (if affecting sample scope), or the deletion of some model object involved
// in the read process.
// Since these edits are performed on the document controller thread, reader validity can immediately
// be checked after the edit has conluded, and any reader that has become invalid can be recreated.
//
// Note that encountering a failure in any individual read call does not invalidate the reader,
// so that the entity using the reader can decide whether to retry or to back out.
// This includes trying to read an audio source for which the host has currently disabled access:
// the failure will be immediately visible, but the reader will remain valid.
// This ensures that for example a realtime renderer can just keep go reading and will be seeing
// proper samples again once sample access is reenabled.
//
// If desired, the code calling readSamples() can also implement proper signaling of any read error
// to the document controller thread to trigger rebuilding the reader as needed.
// This will typically be done when implementing audio source analysis: if there is an error upon
// reading the samples that cannot be resolved within a reasonable timeout, then the anaylsis would
// be aborted. The document controller code that monitors the analysis tasks can evaluate this and
// re-launch a new analysis when appropriate (e.g. when access is re-enabled).
//
// When reading playback regions (directly or through a region sequnece reader), the reader will
// represent the regions as a single source object that covers the union of all affected regions.
// The first sample produced by the reader thus will be the first sample of the earliest region.
// This means that the location of this region has to be taken into account by the calling code if
// it wants to relate the samples to the model or any other reader output.

// TODO JUCE_ARA
// This file contains three different classes that can be 
// used in the JUCE_ARA library - should these classes
// be moved in to individual files, or is sharing files for
// smaller classes preferred?
class ARAAudioSourceReader : public AudioFormatReader,
                             ARAAudioSource::Listener
{
public:
    ARAAudioSourceReader (ARAAudioSource* audioSource, bool use64BitSamples = false);
    virtual ~ARAAudioSourceReader();

    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override;

    // returns false if the audio source sample content has changed
    // since the construction of the audio source reader
    bool isValid() const { return audioSourceBeingRead != nullptr; }
    void invalidate();

    void willUpdateAudioSourceProperties (ARAAudioSource* audioSource, ARAAudioSource::PropertiesPtr newProperties) override;
    void didUpdateAudioSourceContent (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags) override;
    void willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) override;
    void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) override;
    void willDestroyAudioSource (ARAAudioSource* audioSource) override;

private:
    ARAAudioSource* audioSourceBeingRead;
    std::unique_ptr<ARA::PlugIn::HostAudioReader> araHostReader;
    ReadWriteLock lock;
    std::vector<void*> tmpPtrs;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAAudioSourceReader)
};

//==============================================================================

class ARAPlaybackRegionReader : public AudioFormatReader,
                                public ARAPlaybackRegion::Listener
{
public:
    ARAPlaybackRegionReader (ARAPlaybackRenderer* renderer, std::vector<ARAPlaybackRegion*> const& playbackRegions, bool nonRealtime);
    virtual ~ARAPlaybackRegionReader();

    bool isValid() const { return (playbackRenderer != nullptr); }
    void invalidate();

    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override;

    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) override;
    void didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags) override;
    void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) override;

    int64 startInSamples = 0;
    bool isNonRealtime = false;

private:
    std::unique_ptr<ARAPlaybackRenderer> playbackRenderer;
    ReadWriteLock lock;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRegionReader)
};

//==============================================================================

class ARARegionSequenceReader : public ARAPlaybackRegionReader,
                                ARARegionSequence::Listener
{
public:
    ARARegionSequenceReader (ARAPlaybackRenderer* playbackRenderer, ARARegionSequence* regionSequence, bool nonRealtime);
    virtual ~ARARegionSequenceReader();

    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) override;
    void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) override;
    void willDestroyRegionSequence (ARARegionSequence* regionSequence) override;

private:
    ARARegionSequence* sequence;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARARegionSequenceReader)
};

} // namespace juce
