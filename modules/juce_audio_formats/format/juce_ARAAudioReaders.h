/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

namespace juce
{

class AudioProcessor;

/* All these readers follow a common pattern of "invalidation":

   Whenever the samples they are reading are altered, the readers become invalid and will stop
   accessing the model graph. These alterations are model edits such as property changes, content
   changes (if affecting sample scope), or the deletion of some model object involved in the read
   process. Since these edits are performed on the document controller thread, reader validity can
   immediately be checked after the edit has been concluded, and any reader that has become invalid
   can be recreated.

   Note that encountering a failure in any individual read call does not invalidate the reader, so
   that the entity using the reader can decide whether to retry or to back out. This includes trying
   to read an audio source for which the host has currently disabled access: the failure will be
   immediately visible, but the reader will remain valid. This ensures that for example a realtime
   renderer can just keep reading and will be seeing proper samples again once sample access is
   re-enabled.

   If desired, the code calling readSamples() can also implement proper signaling of any read error
   to the document controller thread to trigger rebuilding the reader as needed. This will typically
   be done when implementing audio source analysis: if there is an error upon reading the samples
   that cannot be resolved within a reasonable timeout, then the analysis would be aborted. The
   document controller code that monitors the analysis tasks can evaluate this and re-launch a new
   analysis when appropriate (e.g. when access is re-enabled).

   When reading playback regions (directly or through a region sequence reader), the reader will
   represent the regions as a single source object that covers the union of all affected regions.
   The first sample produced by the reader thus will be the first sample of the earliest region.
   This means that the location of this region has to be taken into account by the calling code if
   it wants to relate the samples to the model or any other reader output.
*/

//==============================================================================
/**
    Subclass of AudioFormatReader that reads samples from a single ARA audio source.

    Plug-Ins typically use this from their rendering code, wrapped in a BufferingAudioReader
    to bridge between realtime rendering and non-realtime audio reading.

    The reader becomes invalidated if
        - the audio source content is updated in a way that affects its samples,
        - the audio source sample access is disabled, or
        - the audio source being read is destroyed.

    @tags{ARA}
*/
class JUCE_API  ARAAudioSourceReader  : public AudioFormatReader,
                                        private ARAAudioSource::Listener
{
public:
    /** Use an ARAAudioSource to construct an audio source reader for the given \p audioSource. */
    explicit ARAAudioSourceReader (ARAAudioSource* audioSource);

    ~ARAAudioSourceReader() override;

    bool readSamples (int* const* destSamples,
                      int numDestChannels,
                      int startOffsetInDestBuffer,
                      int64 startSampleInFile,
                      int numSamples) override;

    /** Returns true as long as the reader's underlying ARAAudioSource remains accessible and its
        sample content is not changed.
    */
    bool isValid() const { return audioSourceBeingRead != nullptr; }

    /** Invalidate the reader - the reader will call this internally if needed, but can also be
        invalidated from the outside (from message thread only!).
    */
    void invalidate();

    void willUpdateAudioSourceProperties (ARAAudioSource* audioSource,
                                          ARAAudioSource::PropertiesPtr newProperties) override;
    void doUpdateAudioSourceContent (ARAAudioSource* audioSource,
                                     ARAContentUpdateScopes scopeFlags) override;
    void willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) override;
    void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) override;
    void willDestroyAudioSource (ARAAudioSource* audioSource) override;

private:
    ARAAudioSource* audioSourceBeingRead;
    std::unique_ptr<ARA::PlugIn::HostAudioReader> hostReader;
    ReadWriteLock lock;
    std::vector<void*> tmpPtrs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAAudioSourceReader)
};

//==============================================================================
/**
    Subclass of AudioFormatReader that reads samples from a group of playback regions.

    Plug-Ins typically use this to draw the output of a playback region in their UI.

    In order to read from playback regions, the reader requires an audio processor that acts as ARA
    playback renderer. Configuring the audio processor for real-time operation results in the reader
    being real-time capable too, unlike most other AudioFormatReaders. The reader instance will take
    care of adding all regions being read to the renderer and invoke its processBlock function in
    order to read the region samples.

    The reader becomes invalid if
        - any region properties are updated in a way that would affect its samples,
        - any region content is updated in a way that would affect its samples, or
        - any of its regions are destroyed.

    @tags{ARA}
*/
class JUCE_API  ARAPlaybackRegionReader   : public AudioFormatReader,
                                            private ARAPlaybackRegion::Listener
{
public:
    /** Create an ARAPlaybackRegionReader instance to read the given \p playbackRegion, using the
        sample rate and channel count of the underlying ARAAudioSource.

        @param playbackRegion The playback region that should be read - must not be nullptr!
    */
    explicit ARAPlaybackRegionReader (ARAPlaybackRegion* playbackRegion);

    /** Create an ARAPlaybackRegionReader instance to read the given \p playbackRegions

        @param sampleRate      The sample rate that should be used for reading.
        @param numChannels     The channel count that should be used for reading.
        @param playbackRegions The vector of playback regions that should be read - must not be empty!
                               All regions must be part of the same ARADocument.
    */
    ARAPlaybackRegionReader (double sampleRate, int numChannels,
                             const std::vector<ARAPlaybackRegion*>& playbackRegions);

    ~ARAPlaybackRegionReader() override;

    /** Returns true as long as any of the reader's underlying playback region's haven't changed. */
    bool isValid() const { return (playbackRenderer != nullptr); }

    /** Invalidate the reader - this should be called if the sample content of any of the reader's
        ARAPlaybackRegions changes.
    */
    void invalidate();

    bool readSamples (int* const* destSamples,
                      int numDestChannels,
                      int startOffsetInDestBuffer,
                      int64 startSampleInFile,
                      int numSamples) override;

    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion,
                                             ARAPlaybackRegion::PropertiesPtr newProperties) override;
    void didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion,
                                         ARAContentUpdateScopes scopeFlags) override;
    void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) override;

    /** The starting point of the reader in playback samples */
    int64 startInSamples = 0;

private:
    std::unique_ptr<ARAPlaybackRenderer> playbackRenderer;
    AudioPlayHead::PositionInfo positionInfo;
    ReadWriteLock lock;

    static constexpr int maximumBlockSize = 4 * 1024;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRegionReader)
};

} // namespace juce
