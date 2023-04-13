/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:                     ARAPluginDemo
 version:                  1.0.0
 vendor:                   JUCE
 website:                  http://juce.com
 description:              Audio plugin using the ARA API.

 dependencies:             juce_audio_basics, juce_audio_devices, juce_audio_formats,
                           juce_audio_plugin_client, juce_audio_processors,
                           juce_audio_utils, juce_core, juce_data_structures,
                           juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:                xcode_mac, vs2022

 moduleFlags:              JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:                     AudioProcessor
 mainClass:                ARADemoPluginAudioProcessor
 documentControllerClass:  ARADemoPluginDocumentControllerSpecialisation

 useLocalCopy:             1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include <ARA_Library/Utilities/ARAPitchInterpretation.h>
#include <ARA_Library/Utilities/ARATimelineConversion.h>

//==============================================================================
class ARADemoPluginAudioModification  : public ARAAudioModification
{
public:
    ARADemoPluginAudioModification (ARAAudioSource* audioSource,
                                    ARA::ARAAudioModificationHostRef hostRef,
                                    const ARAAudioModification* optionalModificationToClone)
        : ARAAudioModification (audioSource, hostRef, optionalModificationToClone)
    {
        if (optionalModificationToClone != nullptr)
            dimmed = static_cast<const ARADemoPluginAudioModification*> (optionalModificationToClone)->dimmed;
    }

    bool isDimmed() const           { return dimmed; }
    void setDimmed (bool shouldDim) { dimmed = shouldDim; }

private:
    bool dimmed = false;
};

//==============================================================================
struct PreviewState
{
    std::atomic<double> previewTime { 0.0 };
    std::atomic<ARAPlaybackRegion*> previewedRegion { nullptr };
};

class SharedTimeSliceThread  : public TimeSliceThread
{
public:
    SharedTimeSliceThread()
        : TimeSliceThread (String (JucePlugin_Name) + " ARA Sample Reading Thread")
    {
        startThread (Priority::high);  // Above default priority so playback is fluent, but below realtime
    }
};

class AsyncConfigurationCallback  : private AsyncUpdater
{
public:
    explicit AsyncConfigurationCallback (std::function<void()> callbackIn)
        : callback (std::move (callbackIn)) {}

    ~AsyncConfigurationCallback() override { cancelPendingUpdate(); }

    template <typename RequiresLock>
    auto withLock (RequiresLock&& fn)
    {
        const SpinLock::ScopedTryLockType scope (processingFlag);
        return fn (scope.isLocked());
    }

    void startConfigure() { triggerAsyncUpdate(); }

private:
    void handleAsyncUpdate() override
    {
        const SpinLock::ScopedLockType scope (processingFlag);
        callback();
    }

    std::function<void()> callback;
    SpinLock processingFlag;
};

static void crossfade (const float* sourceA,
                       const float* sourceB,
                       float aProportionAtStart,
                       float aProportionAtFinish,
                       float* destinationBuffer,
                       int numSamples)
{
    AudioBuffer<float> destination { &destinationBuffer, 1, numSamples };
    destination.copyFromWithRamp (0, 0, sourceA, numSamples, aProportionAtStart, aProportionAtFinish);
    destination.addFromWithRamp (0, 0, sourceB, numSamples, 1.0f - aProportionAtStart, 1.0f - aProportionAtFinish);
}

class Looper
{
public:
    Looper() : inputBuffer (nullptr), pos (loopRange.getStart())
    {
    }

    Looper (const AudioBuffer<float>* buffer, Range<int64> range)
        : inputBuffer (buffer), loopRange (range), pos (range.getStart())
    {
    }

    void writeInto (AudioBuffer<float>& buffer)
    {
        if (loopRange.getLength() == 0)
        {
            buffer.clear();
            return;
        }

        const auto numChannelsToCopy = std::min (inputBuffer->getNumChannels(), buffer.getNumChannels());
        const auto actualCrossfadeLengthSamples = std::min (loopRange.getLength() / 2, (int64) desiredCrossfadeLengthSamples);

        for (auto samplesCopied = 0; samplesCopied < buffer.getNumSamples();)
        {
            const auto [needsCrossfade, samplePosOfNextCrossfadeTransition] = [&]() -> std::pair<bool, int64>
            {
                if (const auto endOfFadeIn = loopRange.getStart() + actualCrossfadeLengthSamples; pos < endOfFadeIn)
                    return { true, endOfFadeIn };

                return { false, loopRange.getEnd() - actualCrossfadeLengthSamples };
            }();

            const auto samplesToNextCrossfadeTransition = samplePosOfNextCrossfadeTransition - pos;
            const auto numSamplesToCopy = std::min (buffer.getNumSamples() - samplesCopied,
                                                    (int) samplesToNextCrossfadeTransition);

            const auto getFadeInGainAtPos = [this, actualCrossfadeLengthSamples] (auto p)
            {
                return jmap ((float) p, (float) loopRange.getStart(), (float) loopRange.getStart() + (float) actualCrossfadeLengthSamples - 1.0f, 0.0f, 1.0f);
            };

            for (int i = 0; i < numChannelsToCopy; ++i)
            {
                if (needsCrossfade)
                {
                    const auto overlapStart = loopRange.getEnd() - actualCrossfadeLengthSamples
                                              + (pos - loopRange.getStart());

                    crossfade (inputBuffer->getReadPointer (i, (int) pos),
                               inputBuffer->getReadPointer (i, (int) overlapStart),
                               getFadeInGainAtPos (pos),
                               getFadeInGainAtPos (pos + numSamplesToCopy),
                               buffer.getWritePointer (i, samplesCopied),
                               numSamplesToCopy);
                }
                else
                {
                    buffer.copyFrom (i, samplesCopied, *inputBuffer, i, (int) pos, numSamplesToCopy);
                }
            }

            samplesCopied += numSamplesToCopy;
            pos += numSamplesToCopy;

            jassert (pos <= loopRange.getEnd() - actualCrossfadeLengthSamples);

            if (pos == loopRange.getEnd() - actualCrossfadeLengthSamples)
                pos = loopRange.getStart();
        }
    }

private:
    static constexpr int desiredCrossfadeLengthSamples = 50;

    const AudioBuffer<float>* inputBuffer;
    Range<int64> loopRange;
    int64 pos;
};

//==============================================================================
// Returns the modified sample range in the output buffer.
inline std::optional<Range<int64>> readPlaybackRangeIntoBuffer (Range<double> playbackRange,
                                                                const ARAPlaybackRegion* playbackRegion,
                                                                AudioBuffer<float>& buffer,
                                                                const std::function<AudioFormatReader* (ARAAudioSource*)>& getReader)
{
    const auto rangeInAudioModificationTime = playbackRange - playbackRegion->getStartInPlaybackTime()
                                                            + playbackRegion->getStartInAudioModificationTime();

    const auto audioModification = playbackRegion->getAudioModification<ARADemoPluginAudioModification>();
    const auto audioSource = audioModification->getAudioSource();
    const auto audioModificationSampleRate = audioSource->getSampleRate();

    const Range<int64_t> sampleRangeInAudioModification {
        ARA::roundSamplePosition (rangeInAudioModificationTime.getStart() * audioModificationSampleRate),
        ARA::roundSamplePosition (rangeInAudioModificationTime.getEnd() * audioModificationSampleRate) - 1
    };

    const auto inputOffset = jlimit ((int64_t) 0, audioSource->getSampleCount(), sampleRangeInAudioModification.getStart());

    // With the output offset it can always be said of the output buffer, that the zeroth element
    // corresponds to beginning of the playbackRange.
    const auto outputOffset = std::max (-sampleRangeInAudioModification.getStart(), (int64_t) 0);

    /* TODO: Handle different AudioSource and playback sample rates.

       The conversion should be done inside a specialized AudioFormatReader so that we could use
       playbackSampleRate everywhere in this function and we could still read `readLength` number of samples
       from the source.

       The current implementation will be incorrect when sampling rates differ.
    */
    const auto readLength = [&]
    {
        const auto sourceReadLength =
            std::min (sampleRangeInAudioModification.getEnd(), audioSource->getSampleCount()) - inputOffset;

        const auto outputReadLength =
            std::min (outputOffset + sourceReadLength, (int64_t) buffer.getNumSamples()) - outputOffset;

        return std::min (sourceReadLength, outputReadLength);
    }();

    if (readLength == 0)
        return Range<int64>();

    auto* reader = getReader (audioSource);

    if (reader != nullptr && reader->read (&buffer, (int) outputOffset, (int) readLength, inputOffset, true, true))
    {
        if (audioModification->isDimmed())
            buffer.applyGain ((int) outputOffset, (int) readLength, 0.25f);

        return Range<int64>::withStartAndLength (outputOffset, readLength);
    }

    return {};
}

class PossiblyBufferedReader
{
public:
    PossiblyBufferedReader() = default;

    explicit PossiblyBufferedReader (std::unique_ptr<BufferingAudioReader> readerIn)
        : setTimeoutFn ([ptr = readerIn.get()] (int ms) { ptr->setReadTimeout (ms); }),
          reader (std::move (readerIn))
    {}

    explicit PossiblyBufferedReader (std::unique_ptr<AudioFormatReader> readerIn)
        : setTimeoutFn(),
          reader (std::move (readerIn))
    {}

    void setReadTimeout (int ms)
    {
        NullCheckedInvocation::invoke (setTimeoutFn, ms);
    }

    AudioFormatReader* get() const { return reader.get(); }

private:
    std::function<void (int)> setTimeoutFn;
    std::unique_ptr<AudioFormatReader> reader;
};

struct ProcessingLockInterface
{
    virtual ~ProcessingLockInterface() = default;
    virtual ScopedTryReadLock getProcessingLock() = 0;
};

//==============================================================================
class PlaybackRenderer  : public ARAPlaybackRenderer
{
public:
    PlaybackRenderer (ARA::PlugIn::DocumentController* dc, ProcessingLockInterface& lockInterfaceIn)
        : ARAPlaybackRenderer (dc), lockInterface (lockInterfaceIn) {}

    void prepareToPlay (double sampleRateIn,
                        int maximumSamplesPerBlockIn,
                        int numChannelsIn,
                        AudioProcessor::ProcessingPrecision,
                        AlwaysNonRealtime alwaysNonRealtime) override
    {
        numChannels = numChannelsIn;
        sampleRate = sampleRateIn;
        maximumSamplesPerBlock = maximumSamplesPerBlockIn;
        tempBuffer.reset (new AudioBuffer<float> (numChannels, maximumSamplesPerBlock));

        useBufferedAudioSourceReader = alwaysNonRealtime == AlwaysNonRealtime::no;

        for (const auto playbackRegion : getPlaybackRegions())
        {
            auto audioSource = playbackRegion->getAudioModification()->getAudioSource();

            if (audioSourceReaders.find (audioSource) == audioSourceReaders.end())
            {
                auto reader = std::make_unique<ARAAudioSourceReader> (audioSource);

                if (! useBufferedAudioSourceReader)
                {
                    audioSourceReaders.emplace (audioSource,
                                                PossiblyBufferedReader { std::move (reader) });
                }
                else
                {
                    const auto readAheadSize = jmax (4 * maximumSamplesPerBlock,
                                                     roundToInt (2.0 * sampleRate));
                    audioSourceReaders.emplace (audioSource,
                                                PossiblyBufferedReader { std::make_unique<BufferingAudioReader> (reader.release(),
                                                                                                                 *sharedTimesliceThread,
                                                                                                                 readAheadSize) });
                }
            }
        }
    }

    void releaseResources() override
    {
        audioSourceReaders.clear();
        tempBuffer.reset();
    }

    bool processBlock (AudioBuffer<float>& buffer,
                       AudioProcessor::Realtime realtime,
                       const AudioPlayHead::PositionInfo& positionInfo) noexcept override
    {
        const auto lock = lockInterface.getProcessingLock();

        if (! lock.isLocked())
            return true;

        const auto numSamples = buffer.getNumSamples();
        jassert (numSamples <= maximumSamplesPerBlock);
        jassert (numChannels == buffer.getNumChannels());
        jassert (realtime == AudioProcessor::Realtime::no || useBufferedAudioSourceReader);
        const auto timeInSamples = positionInfo.getTimeInSamples().orFallback (0);
        const auto isPlaying = positionInfo.getIsPlaying();

        bool success = true;
        bool didRenderAnyRegion = false;

        if (isPlaying)
        {
            const auto blockRange = Range<int64>::withStartAndLength (timeInSamples, numSamples);

            for (const auto& playbackRegion : getPlaybackRegions())
            {
                // Evaluate region borders in song time, calculate sample range to render in song time.
                // Note that this example does not use head- or tailtime, so the includeHeadAndTail
                // parameter is set to false here - this might need to be adjusted in actual plug-ins.
                const auto playbackSampleRange = playbackRegion->getSampleRange (sampleRate, ARAPlaybackRegion::IncludeHeadAndTail::no);
                auto renderRange = blockRange.getIntersectionWith (playbackSampleRange);

                if (renderRange.isEmpty())
                    continue;

                // Evaluate region borders in modification/source time and calculate offset between
                // song and source samples, then clip song samples accordingly
                // (if an actual plug-in supports time stretching, this must be taken into account here).
                Range<int64> modificationSampleRange { playbackRegion->getStartInAudioModificationSamples(),
                                                       playbackRegion->getEndInAudioModificationSamples() };
                const auto modificationSampleOffset = modificationSampleRange.getStart() - playbackSampleRange.getStart();

                renderRange = renderRange.getIntersectionWith (modificationSampleRange.movedToStartAt (playbackSampleRange.getStart()));

                if (renderRange.isEmpty())
                    continue;

                // Get the audio source for the region and find the reader for that source.
                // This simplified example code only produces audio if sample rate and channel count match -
                // a robust plug-in would need to do conversion, see ARA SDK documentation.
                const auto audioSource = playbackRegion->getAudioModification()->getAudioSource();
                const auto readerIt = audioSourceReaders.find (audioSource);

                if (std::make_tuple (audioSource->getChannelCount(), audioSource->getSampleRate()) != std::make_tuple (numChannels, sampleRate)
                    || (readerIt == audioSourceReaders.end()))
                {
                    success = false;
                    continue;
                }

                auto& reader = readerIt->second;
                reader.setReadTimeout (realtime == AudioProcessor::Realtime::no ? 100 : 0);

                // Calculate buffer offsets.
                const int numSamplesToRead = (int) renderRange.getLength();
                const int startInBuffer = (int) (renderRange.getStart() - blockRange.getStart());
                auto startInSource = renderRange.getStart() + modificationSampleOffset;

                // Read samples:
                // first region can write directly into output, later regions need to use local buffer.
                auto& readBuffer = (didRenderAnyRegion) ? *tempBuffer : buffer;

                if (! reader.get()->read (&readBuffer, startInBuffer, numSamplesToRead, startInSource, true, true))
                {
                    success = false;
                    continue;
                }

                // Apply dim if enabled
                if (playbackRegion->getAudioModification<ARADemoPluginAudioModification>()->isDimmed())
                    readBuffer.applyGain (startInBuffer, numSamplesToRead, 0.25f);  // dim by about 12 dB

                // Mix output of all regions
                if (didRenderAnyRegion)
                {
                    // Mix local buffer into the output buffer.
                    for (int c = 0; c < numChannels; ++c)
                        buffer.addFrom (c, startInBuffer, *tempBuffer, c, startInBuffer, numSamplesToRead);
                }
                else
                {
                    // Clear any excess at start or end of the region.
                    if (startInBuffer != 0)
                        buffer.clear (0, startInBuffer);

                    const int endInBuffer = startInBuffer + numSamplesToRead;
                    const int remainingSamples = numSamples - endInBuffer;

                    if (remainingSamples != 0)
                        buffer.clear (endInBuffer, remainingSamples);

                    didRenderAnyRegion = true;
                }
            }
        }

        // If no playback or no region did intersect, clear buffer now.
        if (! didRenderAnyRegion)
            buffer.clear();

        return success;
    }

    using ARAPlaybackRenderer::processBlock;

private:
    //==============================================================================
    ProcessingLockInterface& lockInterface;
    SharedResourcePointer<SharedTimeSliceThread> sharedTimesliceThread;
    std::map<ARAAudioSource*, PossiblyBufferedReader> audioSourceReaders;
    bool useBufferedAudioSourceReader = true;
    int numChannels = 2;
    double sampleRate = 48000.0;
    int maximumSamplesPerBlock = 128;
    std::unique_ptr<AudioBuffer<float>> tempBuffer;
};

class EditorRenderer  : public ARAEditorRenderer,
                        private ARARegionSequence::Listener
{
public:
    EditorRenderer (ARA::PlugIn::DocumentController* documentController,
                    const PreviewState* previewStateIn,
                    ProcessingLockInterface& lockInterfaceIn)
        : ARAEditorRenderer (documentController),
          lockInterface (lockInterfaceIn),
          previewState (previewStateIn)
    {
        jassert (previewState != nullptr);
    }

    ~EditorRenderer() override
    {
        for (const auto& rs : regionSequences)
            rs->removeListener (this);
    }

    void didAddPlaybackRegionToRegionSequence (ARARegionSequence*, ARAPlaybackRegion*) override
    {
        asyncConfigCallback.startConfigure();
    }

    void didAddRegionSequence (ARA::PlugIn::RegionSequence* rs) noexcept override
    {
        auto* sequence = static_cast<ARARegionSequence*> (rs);
        sequence->addListener (this);
        regionSequences.insert (sequence);
        asyncConfigCallback.startConfigure();
    }

    void didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion*) noexcept override
    {
        asyncConfigCallback.startConfigure();
    }

    /*  An ARA host could be using either the `addPlaybackRegion()` or `addRegionSequence()` interface
        so we need to check the other side of both.

        The callback must have a signature of `bool (ARAPlaybackRegion*)`
    */
    template <typename Callback>
    void forEachPlaybackRegion (Callback&& cb)
    {
        for (const auto& playbackRegion : getPlaybackRegions())
            if (! cb (playbackRegion))
                return;

        for (const auto& regionSequence : getRegionSequences())
            for (const auto& playbackRegion : regionSequence->getPlaybackRegions())
                if (! cb (playbackRegion))
                    return;
    }

    void prepareToPlay (double sampleRateIn,
                        int maximumExpectedSamplesPerBlock,
                        int numChannels,
                        AudioProcessor::ProcessingPrecision,
                        AlwaysNonRealtime alwaysNonRealtime) override
    {
        sampleRate = sampleRateIn;
        previewBuffer = std::make_unique<AudioBuffer<float>> (numChannels, (int) (2 * sampleRateIn));

        ignoreUnused (maximumExpectedSamplesPerBlock, alwaysNonRealtime);
    }

    void releaseResources() override
    {
        audioSourceReaders.clear();
    }

    void reset() override
    {
        previewBuffer->clear();
    }

    bool processBlock (AudioBuffer<float>& buffer,
                       AudioProcessor::Realtime realtime,
                       const AudioPlayHead::PositionInfo& positionInfo) noexcept override
    {
        ignoreUnused (realtime);

        const auto lock = lockInterface.getProcessingLock();

        if (! lock.isLocked())
            return true;

        return asyncConfigCallback.withLock ([&] (bool locked)
        {
            if (! locked)
                return true;

            const auto fadeOutIfNecessary = [this, &buffer]
            {
                if (std::exchange (wasPreviewing, false))
                {
                    previewLooper.writeInto (buffer);
                    const auto fadeOutStart = std::max (0, buffer.getNumSamples() - 50);
                    buffer.applyGainRamp (fadeOutStart, buffer.getNumSamples() - fadeOutStart, 1.0f, 0.0f);
                }
            };

            if (positionInfo.getIsPlaying())
            {
                fadeOutIfNecessary();
                return true;
            }

            if (const auto previewedRegion = previewState->previewedRegion.load())
            {
                const auto regionIsAssignedToEditor = [&]()
                {
                    bool regionIsAssigned = false;

                    forEachPlaybackRegion ([&previewedRegion, &regionIsAssigned] (const auto& region)
                    {
                        if (region == previewedRegion)
                        {
                            regionIsAssigned = true;
                            return false;
                        }

                        return true;
                    });

                    return regionIsAssigned;
                }();

                if (regionIsAssignedToEditor)
                {
                    const auto previewTime = previewState->previewTime.load();
                    const auto previewDimmed = previewedRegion->getAudioModification<ARADemoPluginAudioModification>()
                                                              ->isDimmed();

                    if (! exactlyEqual (lastPreviewTime, previewTime)
                        || ! exactlyEqual (lastPlaybackRegion, previewedRegion)
                        || ! exactlyEqual (lastPreviewDimmed, previewDimmed))
                    {
                        Range<double> previewRangeInPlaybackTime { previewTime - 0.25, previewTime + 0.25 };
                        previewBuffer->clear();
                        const auto rangeInOutput = readPlaybackRangeIntoBuffer (previewRangeInPlaybackTime,
                                                                                previewedRegion,
                                                                                *previewBuffer,
                                                                                [this] (auto* source) -> auto*
                                                                                {
                                                                                    const auto iter = audioSourceReaders.find (source);
                                                                                    return iter != audioSourceReaders.end() ? iter->second.get() : nullptr;
                                                                                });

                        if (rangeInOutput)
                        {
                            lastPreviewTime = previewTime;
                            lastPlaybackRegion = previewedRegion;
                            lastPreviewDimmed = previewDimmed;
                            previewLooper = Looper (previewBuffer.get(), *rangeInOutput);
                        }
                    }
                    else
                    {
                        previewLooper.writeInto (buffer);

                        if (! std::exchange (wasPreviewing, true))
                        {
                            const auto fadeInLength = std::min (50, buffer.getNumSamples());
                            buffer.applyGainRamp (0, fadeInLength, 0.0f, 1.0f);
                        }
                    }
                }
            }
            else
            {
                fadeOutIfNecessary();
            }

            return true;
        });
    }

    using ARAEditorRenderer::processBlock;

private:
    void configure()
    {
        forEachPlaybackRegion ([this, maximumExpectedSamplesPerBlock = 1000] (const auto& playbackRegion)
        {
            const auto audioSource = playbackRegion->getAudioModification()->getAudioSource();

            if (audioSourceReaders.find (audioSource) == audioSourceReaders.end())
            {
                audioSourceReaders[audioSource] = std::make_unique<BufferingAudioReader> (
                        new ARAAudioSourceReader (playbackRegion->getAudioModification()->getAudioSource()),
                        *timeSliceThread,
                        std::max (4 * maximumExpectedSamplesPerBlock, (int) sampleRate));
            }

            return true;
        });
    }

    ProcessingLockInterface& lockInterface;
    const PreviewState* previewState = nullptr;
    AsyncConfigurationCallback asyncConfigCallback { [this] { configure(); } };
    double lastPreviewTime = 0.0;
    ARAPlaybackRegion* lastPlaybackRegion = nullptr;
    bool lastPreviewDimmed = false;
    bool wasPreviewing = false;
    std::unique_ptr<AudioBuffer<float>> previewBuffer;
    Looper previewLooper;

    double sampleRate = 48000.0;
    SharedResourcePointer<SharedTimeSliceThread> timeSliceThread;
    std::map<ARAAudioSource*, std::unique_ptr<BufferingAudioReader>> audioSourceReaders;

    std::set<ARARegionSequence*> regionSequences;
};

//==============================================================================
class ARADemoPluginDocumentControllerSpecialisation  : public ARADocumentControllerSpecialisation,
                                                       private ProcessingLockInterface
{
public:
    using ARADocumentControllerSpecialisation::ARADocumentControllerSpecialisation;

    PreviewState previewState;

protected:
    void willBeginEditing (ARADocument*) override
    {
        processBlockLock.enterWrite();
    }

    void didEndEditing (ARADocument*) override
    {
        processBlockLock.exitWrite();
    }

    ARAAudioModification* doCreateAudioModification (ARAAudioSource* audioSource,
                                                     ARA::ARAAudioModificationHostRef hostRef,
                                                     const ARAAudioModification* optionalModificationToClone) noexcept override
    {
        return new ARADemoPluginAudioModification (audioSource,
                                                   hostRef,
                                                   static_cast<const ARADemoPluginAudioModification*> (optionalModificationToClone));
    }

    ARAPlaybackRenderer* doCreatePlaybackRenderer() noexcept override
    {
        return new PlaybackRenderer (getDocumentController(), *this);
    }

    EditorRenderer* doCreateEditorRenderer() noexcept override
    {
        return new EditorRenderer (getDocumentController(), &previewState, *this);
    }

    bool doRestoreObjectsFromStream (ARAInputStream& input,
                                     const ARARestoreObjectsFilter* filter) noexcept override
    {
        // Start reading data from the archive, starting with the number of audio modifications in the archive
        const auto numAudioModifications = input.readInt64();

        // Loop over stored audio modification data
        for (int64 i = 0; i < numAudioModifications; ++i)
        {
            const auto progressVal = (float) i / (float) numAudioModifications;
            getDocumentController()->getHostArchivingController()->notifyDocumentUnarchivingProgress (progressVal);

            // Read audio modification persistent ID and analysis result from archive
            const String persistentID = input.readString();
            const bool dimmed = input.readBool();

            // Find audio modification to restore the state to (drop state if not to be loaded)
            auto audioModification = filter->getAudioModificationToRestoreStateWithID<ARADemoPluginAudioModification> (persistentID.getCharPointer());

            if (audioModification == nullptr)
                continue;

            const bool dimChanged = (dimmed != audioModification->isDimmed());
            audioModification->setDimmed (dimmed);

            // If the dim state changed, send a sample content change notification without notifying the host
            if (dimChanged)
            {
                audioModification->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected(), false);

                for (auto playbackRegion : audioModification->getPlaybackRegions())
                    playbackRegion->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected(), false);
            }
        }

        getDocumentController()->getHostArchivingController()->notifyDocumentUnarchivingProgress (1.0f);

        return ! input.failed();
    }

    bool doStoreObjectsToStream (ARAOutputStream& output, const ARAStoreObjectsFilter* filter) noexcept override
    {
        // This example implementation only deals with audio modification states
        const auto& audioModificationsToPersist { filter->getAudioModificationsToStore<ARADemoPluginAudioModification>() };

        const auto reportProgress = [archivingController = getDocumentController()->getHostArchivingController()] (float p)
        {
            archivingController->notifyDocumentArchivingProgress (p);
        };

        const ScopeGuard scope { [&reportProgress] { reportProgress (1.0f); } };

        // Write the number of audio modifications we are persisting
        const auto numAudioModifications = audioModificationsToPersist.size();

        if (! output.writeInt64 ((int64) numAudioModifications))
            return false;

        // For each audio modification to persist, persist its ID followed by whether it's dimmed
        for (size_t i = 0; i < numAudioModifications; ++i)
        {
            // Write persistent ID and dim state
            if (! output.writeString (audioModificationsToPersist[i]->getPersistentID()))
                return false;

            if (! output.writeBool (audioModificationsToPersist[i]->isDimmed()))
                return false;

            const auto progressVal = (float) i / (float) numAudioModifications;
            reportProgress (progressVal);
        }

        return true;
    }

private:
    ScopedTryReadLock getProcessingLock() override
    {
        return ScopedTryReadLock { processBlockLock };
    }

    ReadWriteLock processBlockLock;
};

struct PlayHeadState
{
    void update (const Optional<AudioPlayHead::PositionInfo>& info)
    {
        if (info.hasValue())
        {
            isPlaying.store (info->getIsPlaying(), std::memory_order_relaxed);
            timeInSeconds.store (info->getTimeInSeconds().orFallback (0), std::memory_order_relaxed);
            isLooping.store (info->getIsLooping(), std::memory_order_relaxed);
            const auto loopPoints = info->getLoopPoints();

            if (loopPoints.hasValue())
            {
                loopPpqStart = loopPoints->ppqStart;
                loopPpqEnd = loopPoints->ppqEnd;
            }
        }
        else
        {
            isPlaying.store (false, std::memory_order_relaxed);
            isLooping.store (false, std::memory_order_relaxed);
        }
    }

    std::atomic<bool> isPlaying { false },
                      isLooping { false };
    std::atomic<double> timeInSeconds { 0.0 },
                        loopPpqStart  { 0.0 },
                        loopPpqEnd    { 0.0 };
};

//==============================================================================
class ARADemoPluginAudioProcessorImpl  : public AudioProcessor,
                                         public AudioProcessorARAExtension
{
public:
    //==============================================================================
    ARADemoPluginAudioProcessorImpl()
        : AudioProcessor (getBusesProperties())
    {}

    ~ARADemoPluginAudioProcessorImpl() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        playHeadState.update (nullopt);
        prepareToPlayForARA (sampleRate, samplesPerBlock, getMainBusNumOutputChannels(), getProcessingPrecision());
    }

    void releaseResources() override
    {
        playHeadState.update (nullopt);
        releaseResourcesForARA();
    }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
            && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
            return false;

        return true;
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        ignoreUnused (midiMessages);

        ScopedNoDenormals noDenormals;

        auto* audioPlayHead = getPlayHead();
        playHeadState.update (audioPlayHead->getPosition());

        if (! processBlockForARA (buffer, isRealtime(), audioPlayHead))
            processBlockBypassed (buffer, midiMessages);
    }

    using AudioProcessor::processBlock;

    //==============================================================================
    const String getName() const override                             { return "ARAPluginDemo"; }
    bool acceptsMidi() const override                                 { return true; }
    bool producesMidi() const override                                { return true; }

    double getTailLengthSeconds() const override
    {
        double tail;
        if (getTailLengthSecondsForARA (tail))
            return tail;

        return 0.0;
    }

    //==============================================================================
    int getNumPrograms() override                                     { return 0; }
    int getCurrentProgram() override                                  { return 0; }
    void setCurrentProgram (int) override                             {}
    const String getProgramName (int) override                        { return "None"; }
    void changeProgramName (int, const String&) override              {}

    //==============================================================================
    void getStateInformation (MemoryBlock&) override                  {}
    void setStateInformation (const void*, int) override              {}

    PlayHeadState playHeadState;

private:
    //==============================================================================
    static BusesProperties getBusesProperties()
    {
        return BusesProperties().withInput  ("Input",  AudioChannelSet::stereo(), true)
            .withOutput ("Output", AudioChannelSet::stereo(), true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADemoPluginAudioProcessorImpl)
};

//==============================================================================
class TimeToViewScaling
{
public:
    class Listener
    {
    public:
        virtual ~Listener() = default;

        virtual void zoomLevelChanged (double newPixelPerSecond) = 0;
    };

    void addListener (Listener* l)    { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

    TimeToViewScaling() = default;

    void zoom (double factor)
    {
        zoomLevelPixelPerSecond = jlimit (minimumZoom, minimumZoom * 32, zoomLevelPixelPerSecond * factor);
        setZoomLevel (zoomLevelPixelPerSecond);
    }

    void setZoomLevel (double pixelPerSecond)
    {
        zoomLevelPixelPerSecond = pixelPerSecond;
        listeners.call ([this] (Listener& l) { l.zoomLevelChanged (zoomLevelPixelPerSecond); });
    }

    int getXForTime (double time) const
    {
        return roundToInt (time * zoomLevelPixelPerSecond);
    }

    double getTimeForX (int x) const
    {
        return x / zoomLevelPixelPerSecond;
    }

private:
    static constexpr auto minimumZoom = 10.0;

    double zoomLevelPixelPerSecond = minimumZoom * 4;
    ListenerList<Listener> listeners;
};

class RulersView : public Component,
                   public SettableTooltipClient,
                   private Timer,
                   private TimeToViewScaling::Listener,
                   private ARAMusicalContext::Listener
{
public:
    class CycleMarkerComponent : public Component
    {
        void paint (Graphics& g) override
        {
            g.setColour (Colours::yellow.darker (0.2f));
            const auto bounds = getLocalBounds().toFloat();
            g.drawRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), 6.0f, 2.0f);
        }
    };

    RulersView (PlayHeadState& playHeadStateIn, TimeToViewScaling& timeToViewScalingIn, ARADocument& document)
        : playHeadState (playHeadStateIn), timeToViewScaling (timeToViewScalingIn), araDocument (document)
    {
        timeToViewScaling.addListener (this);

        addChildComponent (cycleMarker);
        cycleMarker.setInterceptsMouseClicks (false, false);

        setTooltip ("Double-click to start playback, click to stop playback or to reposition, drag horizontal range to set cycle.");

        startTimerHz (30);
    }

    ~RulersView() override
    {
        stopTimer();

        timeToViewScaling.removeListener (this);
        selectMusicalContext (nullptr);
    }

    void paint (Graphics& g) override
    {
        auto drawBounds = g.getClipBounds();
        const auto drawStartTime = timeToViewScaling.getTimeForX (drawBounds.getX());
        const auto drawEndTime   = timeToViewScaling.getTimeForX (drawBounds.getRight());

        const auto bounds = getLocalBounds();

        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.fillRect (bounds);
        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting());
        g.drawRect (bounds);

        const auto rulerHeight = bounds.getHeight() / 3;
        g.drawRect (drawBounds.getX(), rulerHeight, drawBounds.getRight(), rulerHeight);
        g.setFont (Font (12.0f));

        const int lightLineWidth = 1;
        const int heavyLineWidth = 3;

        if (selectedMusicalContext != nullptr)
        {
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (selectedMusicalContext);
            const ARA::TempoConverter<decltype (tempoReader)> tempoConverter (tempoReader);

            // chord ruler: one rect per chord, skipping empty "no chords"
            const auto chordBounds = drawBounds.removeFromTop (rulerHeight);
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeSheetChords> chordsReader (selectedMusicalContext);

            if (tempoReader && chordsReader)
            {
                const ARA::ChordInterpreter interpreter (true);
                for (auto itChord = chordsReader.begin(); itChord != chordsReader.end(); ++itChord)
                {
                    if (interpreter.isNoChord (*itChord))
                        continue;

                    const auto chordStartTime = (itChord == chordsReader.begin()) ? 0 : tempoConverter.getTimeForQuarter (itChord->position);

                    if (chordStartTime >= drawEndTime)
                        break;

                    auto chordRect = chordBounds;
                    chordRect.setLeft (timeToViewScaling.getXForTime (chordStartTime));

                    if (std::next (itChord) != chordsReader.end())
                    {
                        const auto nextChordStartTime = tempoConverter.getTimeForQuarter (std::next (itChord)->position);

                        if (nextChordStartTime < drawStartTime)
                            continue;

                        chordRect.setRight (timeToViewScaling.getXForTime (nextChordStartTime));
                    }

                    g.drawRect (chordRect);
                    g.drawText (convertARAString (interpreter.getNameForChord (*itChord).c_str()),
                                chordRect.withTrimmedLeft (2),
                                Justification::centredLeft);
                }
            }

            // beat ruler: evaluates tempo and bar signatures to draw a line for each beat
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures> barSignaturesReader (selectedMusicalContext);

            if (barSignaturesReader)
            {
                const ARA::BarSignaturesConverter<decltype (barSignaturesReader)> barSignaturesConverter (barSignaturesReader);

                const double beatStart = barSignaturesConverter.getBeatForQuarter (tempoConverter.getQuarterForTime (drawStartTime));
                const double beatEnd   = barSignaturesConverter.getBeatForQuarter (tempoConverter.getQuarterForTime (drawEndTime));
                const int endBeat = roundToInt (std::floor (beatEnd));
                RectangleList<int> rects;

                for (int beat = roundToInt (std::ceil (beatStart)); beat <= endBeat; ++beat)
                {
                    const auto quarterPos = barSignaturesConverter.getQuarterForBeat (beat);
                    const int x = timeToViewScaling.getXForTime (tempoConverter.getTimeForQuarter (quarterPos));
                    const auto barSignature = barSignaturesConverter.getBarSignatureForQuarter (quarterPos);
                    const int lineWidth = (approximatelyEqual (quarterPos, barSignature.position)) ? heavyLineWidth : lightLineWidth;
                    const int beatsSinceBarStart = roundToInt( barSignaturesConverter.getBeatDistanceFromBarStartForQuarter (quarterPos));
                    const int lineHeight = (beatsSinceBarStart == 0) ? rulerHeight : rulerHeight / 2;
                    rects.addWithoutMerging (Rectangle<int> (x - lineWidth / 2, 2 * rulerHeight - lineHeight, lineWidth, lineHeight));
                }

                g.fillRectList (rects);
            }
        }

        // time ruler: one tick for each second
        {
            RectangleList<int> rects;

            for (auto time = std::floor (drawStartTime); time <= drawEndTime; time += 1.0)
            {
                const int lineWidth  = (std::fmod (time, 60.0) <= 0.001) ? heavyLineWidth : lightLineWidth;
                const int lineHeight = (std::fmod (time, 10.0) <= 0.001) ? rulerHeight : rulerHeight / 2;
                rects.addWithoutMerging (Rectangle<int> (timeToViewScaling.getXForTime (time) - lineWidth / 2,
                                                         bounds.getHeight() - lineHeight,
                                                         lineWidth,
                                                         lineHeight));
            }

            g.fillRectList (rects);
        }
    }

    void mouseDrag (const MouseEvent& m) override
    {
        isDraggingCycle = true;

        auto cycleRect = getBounds();
        cycleRect.setLeft  (jmin (m.getMouseDownX(), m.x));
        cycleRect.setRight (jmax (m.getMouseDownX(), m.x));
        cycleMarker.setBounds (cycleRect);
    }

    void mouseUp (const MouseEvent& m) override
    {
        auto playbackController = araDocument.getDocumentController()->getHostPlaybackController();

        if (playbackController != nullptr)
        {
            const auto startTime = timeToViewScaling.getTimeForX (jmin (m.getMouseDownX(), m.x));
            const auto endTime   = timeToViewScaling.getTimeForX (jmax (m.getMouseDownX(), m.x));

            if (playHeadState.isPlaying.load (std::memory_order_relaxed))
                playbackController->requestStopPlayback();
            else
                playbackController->requestSetPlaybackPosition (startTime);

            if (isDraggingCycle)
                playbackController->requestSetCycleRange (startTime, endTime - startTime);
        }

        isDraggingCycle = false;
    }

    void mouseDoubleClick (const MouseEvent&) override
    {
        if (auto* playbackController = araDocument.getDocumentController()->getHostPlaybackController())
        {
            if (! playHeadState.isPlaying.load (std::memory_order_relaxed))
                playbackController->requestStartPlayback();
        }
    }

    void selectMusicalContext (ARAMusicalContext* newSelectedMusicalContext)
    {
        if (auto* oldSelection = std::exchange (selectedMusicalContext, newSelectedMusicalContext);
            oldSelection != selectedMusicalContext)
        {
            if (oldSelection != nullptr)
                oldSelection->removeListener (this);

            if (selectedMusicalContext != nullptr)
                selectedMusicalContext->addListener (this);

            repaint();
        }
    }

    void zoomLevelChanged (double) override
    {
        repaint();
    }

    void doUpdateMusicalContextContent (ARAMusicalContext*, ARAContentUpdateScopes) override
    {
        repaint();
    }

private:
    void updateCyclePosition()
    {
        if (selectedMusicalContext != nullptr)
        {
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (selectedMusicalContext);
            const ARA::TempoConverter<decltype (tempoReader)> tempoConverter (tempoReader);

            const auto loopStartTime = tempoConverter.getTimeForQuarter (playHeadState.loopPpqStart.load (std::memory_order_relaxed));
            const auto loopEndTime   = tempoConverter.getTimeForQuarter (playHeadState.loopPpqEnd.load   (std::memory_order_relaxed));

            auto cycleRect = getBounds();
            cycleRect.setLeft  (timeToViewScaling.getXForTime (loopStartTime));
            cycleRect.setRight (timeToViewScaling.getXForTime (loopEndTime));
            cycleMarker.setVisible (true);
            cycleMarker.setBounds (cycleRect);
        }
        else
        {
            cycleMarker.setVisible (false);
        }
    }

    void timerCallback() override
    {
        if (! isDraggingCycle)
            updateCyclePosition();
    }

private:
    PlayHeadState& playHeadState;
    TimeToViewScaling& timeToViewScaling;
    ARADocument& araDocument;
    ARAMusicalContext* selectedMusicalContext = nullptr;
    CycleMarkerComponent cycleMarker;
    bool isDraggingCycle = false;
};

class RulersHeader : public Component
{
public:
    RulersHeader()
    {
        chordsLabel.setText ("Chords", NotificationType::dontSendNotification);
        addAndMakeVisible (chordsLabel);

        barsLabel.setText ("Bars", NotificationType::dontSendNotification);
        addAndMakeVisible (barsLabel);

        timeLabel.setText ("Time", NotificationType::dontSendNotification);
        addAndMakeVisible (timeLabel);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        const auto rulerHeight = bounds.getHeight() / 3;

        for (auto* label : { &chordsLabel, &barsLabel, &timeLabel })
            label->setBounds (bounds.removeFromTop (rulerHeight));
    }

    void paint (Graphics& g) override
    {
        auto bounds = getLocalBounds();
        const auto rulerHeight = bounds.getHeight() / 3;
        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.fillRect (bounds);
        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting());
        g.drawRect (bounds);
        bounds.removeFromTop (rulerHeight);
        g.drawRect (bounds.removeFromTop (rulerHeight));
    }

private:
    Label chordsLabel, barsLabel, timeLabel;
};

//==============================================================================
struct WaveformCache : private ARAAudioSource::Listener
{
    WaveformCache() : thumbnailCache (20)
    {
    }

    ~WaveformCache() override
    {
        for (const auto& entry : thumbnails)
        {
            entry.first->removeListener (this);
        }
    }

    //==============================================================================
    void willDestroyAudioSource (ARAAudioSource* audioSource) override
    {
        removeAudioSource (audioSource);
    }

    AudioThumbnail& getOrCreateThumbnail (ARAAudioSource* audioSource)
    {
        const auto iter = thumbnails.find (audioSource);

        if (iter != std::end (thumbnails))
            return *iter->second;

        auto thumb = std::make_unique<AudioThumbnail> (128, dummyManager, thumbnailCache);
        auto& result = *thumb;

        ++hash;
        thumb->setReader (new ARAAudioSourceReader (audioSource), hash);

        audioSource->addListener (this);
        thumbnails.emplace (audioSource, std::move (thumb));
        return result;
    }

private:
    void removeAudioSource (ARAAudioSource* audioSource)
    {
        audioSource->removeListener (this);
        thumbnails.erase (audioSource);
    }

    int64 hash = 0;
    AudioFormatManager dummyManager;
    AudioThumbnailCache thumbnailCache;
    std::map<ARAAudioSource*, std::unique_ptr<AudioThumbnail>> thumbnails;
};

class PlaybackRegionView : public Component,
                           public ChangeListener,
                           public SettableTooltipClient,
                           private ARAAudioSource::Listener,
                           private ARAPlaybackRegion::Listener,
                           private ARAEditorView::Listener
{
public:
    PlaybackRegionView (ARAEditorView& editorView, ARAPlaybackRegion& region, WaveformCache& cache)
        : araEditorView (editorView), playbackRegion (region), waveformCache (cache), previewRegionOverlay (*this)
    {
        auto* audioSource = playbackRegion.getAudioModification()->getAudioSource();

        waveformCache.getOrCreateThumbnail (audioSource).addChangeListener (this);

        audioSource->addListener (this);
        playbackRegion.addListener (this);
        araEditorView.addListener (this);
        addAndMakeVisible (previewRegionOverlay);

        setTooltip ("Double-click to toggle dim state of the region, click and hold to prelisten region near click.");
    }

    ~PlaybackRegionView() override
    {
        auto* audioSource = playbackRegion.getAudioModification()->getAudioSource();

        audioSource->removeListener (this);
        playbackRegion.removeListener (this);
        araEditorView.removeListener (this);

        waveformCache.getOrCreateThumbnail (audioSource).removeChangeListener (this);
    }

    void mouseDown (const MouseEvent& m) override
    {
        const auto relativeTime = (double) m.getMouseDownX() / getLocalBounds().getWidth();
        const auto previewTime = playbackRegion.getStartInPlaybackTime()
                                 + relativeTime * playbackRegion.getDurationInPlaybackTime();
        auto& previewState = ARADocumentControllerSpecialisation::getSpecialisedDocumentController<ARADemoPluginDocumentControllerSpecialisation> (playbackRegion.getDocumentController())->previewState;
        previewState.previewTime.store (previewTime);
        previewState.previewedRegion.store (&playbackRegion);
        previewRegionOverlay.update();
    }

    void mouseUp (const MouseEvent&) override
    {
        auto& previewState = ARADocumentControllerSpecialisation::getSpecialisedDocumentController<ARADemoPluginDocumentControllerSpecialisation> (playbackRegion.getDocumentController())->previewState;
        previewState.previewTime.store (0.0);
        previewState.previewedRegion.store (nullptr);
        previewRegionOverlay.update();
    }

    void mouseDoubleClick (const MouseEvent&) override
    {
        // Set the dim flag on our region's audio modification when double-clicked
        auto audioModification = playbackRegion.getAudioModification<ARADemoPluginAudioModification>();
        audioModification->setDimmed (! audioModification->isDimmed());

        // Send a content change notification for the modification and all associated playback regions
        audioModification->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected(), true);
        for (auto region : audioModification->getPlaybackRegions())
            region->notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected(), true);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        repaint();
    }

    void didEnableAudioSourceSamplesAccess (ARAAudioSource*, bool) override
    {
        repaint();
    }

    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion*,
                                             ARAPlaybackRegion::PropertiesPtr newProperties) override
    {
        if (playbackRegion.getName() != newProperties->name
            || playbackRegion.getColor() != newProperties->color)
        {
            repaint();
        }
    }

    void didUpdatePlaybackRegionContent (ARAPlaybackRegion*, ARAContentUpdateScopes) override
    {
        repaint();
    }

    void onNewSelection (const ARAViewSelection& viewSelection) override
    {
        const auto& selectedPlaybackRegions = viewSelection.getPlaybackRegions();
        const bool selected = std::find (selectedPlaybackRegions.begin(), selectedPlaybackRegions.end(), &playbackRegion) != selectedPlaybackRegions.end();
        if (selected != isSelected)
        {
            isSelected = selected;
            repaint();
        }
    }

    void paint (Graphics& g) override
    {
        g.fillAll (convertOptionalARAColour (playbackRegion.getEffectiveColor(), Colours::black));

        const auto* audioModification = playbackRegion.getAudioModification<ARADemoPluginAudioModification>();
        g.setColour (audioModification->isDimmed() ? Colours::darkgrey.darker() : Colours::darkgrey.brighter());

        if (audioModification->getAudioSource()->isSampleAccessEnabled())
        {
            auto& thumbnail = waveformCache.getOrCreateThumbnail (playbackRegion.getAudioModification()->getAudioSource());
            thumbnail.drawChannels (g,
                                    getLocalBounds(),
                                    playbackRegion.getStartInAudioModificationTime(),
                                    playbackRegion.getEndInAudioModificationTime(),
                                    1.0f);
        }
        else
        {
            g.setFont (Font (12.0f));
            g.drawText ("Audio Access Disabled", getLocalBounds(), Justification::centred);
        }

        g.setColour (Colours::white.withMultipliedAlpha (0.9f));
        g.setFont (Font (12.0f));
        g.drawText (convertOptionalARAString (playbackRegion.getEffectiveName()),
                    getLocalBounds(),
                    Justification::topLeft);

        if (audioModification->isDimmed())
            g.drawText ("DIMMED", getLocalBounds(), Justification::bottomLeft);

        g.setColour (isSelected ? Colours::white : Colours::black);
        g.drawRect (getLocalBounds());
    }

    void resized() override
    {
        repaint();
    }

private:
    class PreviewRegionOverlay  : public Component
    {
        static constexpr auto previewLength = 0.5;

    public:
        PreviewRegionOverlay (PlaybackRegionView& ownerIn) : owner (ownerIn)
        {
        }

        void update()
        {
            const auto& previewState = owner.getDocumentController()->previewState;

            if (previewState.previewedRegion.load() == &owner.playbackRegion)
            {
                const auto previewStartTime = previewState.previewTime.load() - owner.playbackRegion.getStartInPlaybackTime();
                const auto pixelPerSecond = owner.getWidth() / owner.playbackRegion.getDurationInPlaybackTime();

                setBounds (roundToInt ((previewStartTime - previewLength / 2) * pixelPerSecond),
                           0,
                           roundToInt (previewLength * pixelPerSecond),
                           owner.getHeight());

                setVisible (true);
            }
            else
            {
                setVisible (false);
            }

            repaint();
        }

        void paint (Graphics& g) override
        {
            g.setColour (Colours::yellow.withAlpha (0.5f));
            g.fillRect (getLocalBounds());
        }

    private:
        PlaybackRegionView& owner;
    };

    ARADemoPluginDocumentControllerSpecialisation* getDocumentController() const
    {
        return ARADocumentControllerSpecialisation::getSpecialisedDocumentController<ARADemoPluginDocumentControllerSpecialisation> (playbackRegion.getDocumentController());
    }

    ARAEditorView& araEditorView;
    ARAPlaybackRegion& playbackRegion;
    WaveformCache& waveformCache;
    PreviewRegionOverlay previewRegionOverlay;
    bool isSelected = false;
};

class RegionSequenceView : public Component,
                           public ChangeBroadcaster,
                           private TimeToViewScaling::Listener,
                           private ARARegionSequence::Listener,
                           private ARAPlaybackRegion::Listener
{
public:
    RegionSequenceView (ARAEditorView& editorView, TimeToViewScaling& scaling, ARARegionSequence& rs, WaveformCache& cache)
        : araEditorView (editorView), timeToViewScaling (scaling), regionSequence (rs), waveformCache (cache)
    {
        regionSequence.addListener (this);

        for (auto* playbackRegion : regionSequence.getPlaybackRegions())
            createAndAddPlaybackRegionView (playbackRegion);

        updatePlaybackDuration();

        timeToViewScaling.addListener (this);
    }

    ~RegionSequenceView() override
    {
        timeToViewScaling.removeListener (this);

        regionSequence.removeListener (this);

        for (const auto& it : playbackRegionViews)
            it.first->removeListener (this);
    }

    //==============================================================================
    // ARA Document change callback overrides
    void willUpdateRegionSequenceProperties (ARARegionSequence*,
                                             ARARegionSequence::PropertiesPtr newProperties) override
    {
        if (regionSequence.getColor() != newProperties->color)
        {
            for (auto& pbr : playbackRegionViews)
                pbr.second->repaint();
        }
    }

    void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence*,
                                                     ARAPlaybackRegion* playbackRegion) override
    {
        playbackRegion->removeListener (this);
        removeChildComponent (playbackRegionViews[playbackRegion].get());
        playbackRegionViews.erase (playbackRegion);
        updatePlaybackDuration();
    }

    void didAddPlaybackRegionToRegionSequence (ARARegionSequence*, ARAPlaybackRegion* playbackRegion) override
    {
        createAndAddPlaybackRegionView (playbackRegion);
        updatePlaybackDuration();
    }

    void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) override
    {
        playbackRegion->removeListener (this);
        removeChildComponent (playbackRegionViews[playbackRegion].get());
        playbackRegionViews.erase (playbackRegion);
        updatePlaybackDuration();
    }

    void didUpdatePlaybackRegionProperties (ARAPlaybackRegion*) override
    {
        updatePlaybackDuration();
    }

    void zoomLevelChanged (double) override
    {
        resized();
    }

    void resized() override
    {
        for (auto& pbr : playbackRegionViews)
        {
            const auto playbackRegion = pbr.first;
            pbr.second->setBounds (
                getLocalBounds()
                    .withTrimmedLeft (timeToViewScaling.getXForTime (playbackRegion->getStartInPlaybackTime()))
                    .withWidth (timeToViewScaling.getXForTime (playbackRegion->getDurationInPlaybackTime())));
        }
    }

    auto getPlaybackDuration() const noexcept
    {
        return playbackDuration;
    }

private:
    void createAndAddPlaybackRegionView (ARAPlaybackRegion* playbackRegion)
    {
        playbackRegionViews[playbackRegion] = std::make_unique<PlaybackRegionView> (araEditorView,
                                                                                    *playbackRegion,
                                                                                    waveformCache);
        playbackRegion->addListener (this);
        addAndMakeVisible (*playbackRegionViews[playbackRegion]);
    }

    void updatePlaybackDuration()
    {
        const auto iter = std::max_element (
            playbackRegionViews.begin(),
            playbackRegionViews.end(),
            [] (const auto& a, const auto& b) { return a.first->getEndInPlaybackTime() < b.first->getEndInPlaybackTime(); });

        playbackDuration = iter != playbackRegionViews.end() ? iter->first->getEndInPlaybackTime()
                                                             : 0.0;

        sendChangeMessage();
    }

    ARAEditorView& araEditorView;
    TimeToViewScaling& timeToViewScaling;
    ARARegionSequence& regionSequence;
    WaveformCache& waveformCache;
    std::unordered_map<ARAPlaybackRegion*, std::unique_ptr<PlaybackRegionView>> playbackRegionViews;
    double playbackDuration = 0.0;
};

class ZoomControls : public Component
{
public:
    ZoomControls()
    {
        addAndMakeVisible (zoomInButton);
        addAndMakeVisible (zoomOutButton);
    }

    void setZoomInCallback  (std::function<void()> cb)   { zoomInButton.onClick  = std::move (cb); }
    void setZoomOutCallback (std::function<void()> cb)   { zoomOutButton.onClick = std::move (cb); }

    void resized() override
    {
        FlexBox fb;
        fb.justifyContent = FlexBox::JustifyContent::flexEnd;

        for (auto* button : { &zoomInButton, &zoomOutButton })
            fb.items.add (FlexItem (*button).withMinHeight (30.0f).withMinWidth (30.0f).withMargin ({ 5, 5, 5, 0 }));

        fb.performLayout (getLocalBounds());
    }

private:
    TextButton zoomInButton { "+" }, zoomOutButton { "-" };
};

class PlayheadPositionLabel : public Label,
                              private Timer
{
public:
    PlayheadPositionLabel (PlayHeadState& playHeadStateIn)
        : playHeadState (playHeadStateIn)
    {
        startTimerHz (30);
    }

    ~PlayheadPositionLabel() override
    {
        stopTimer();
    }

    void selectMusicalContext (ARAMusicalContext* newSelectedMusicalContext)
    {
        selectedMusicalContext = newSelectedMusicalContext;
    }

private:
    void timerCallback() override
    {
        const auto timePosition = playHeadState.timeInSeconds.load (std::memory_order_relaxed);

        auto text = timeToTimecodeString (timePosition);

        if (playHeadState.isPlaying.load (std::memory_order_relaxed))
            text += " (playing)";
        else
            text += " (stopped)";

        if (selectedMusicalContext != nullptr)
        {
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (selectedMusicalContext);
            const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures> barSignaturesReader (selectedMusicalContext);

            if (tempoReader && barSignaturesReader)
            {
                const ARA::TempoConverter<decltype (tempoReader)> tempoConverter (tempoReader);
                const ARA::BarSignaturesConverter<decltype (barSignaturesReader)> barSignaturesConverter (barSignaturesReader);
                const auto quarterPosition = tempoConverter.getQuarterForTime (timePosition);
                const auto barIndex = barSignaturesConverter.getBarIndexForQuarter (quarterPosition);
                const auto beatDistance = barSignaturesConverter.getBeatDistanceFromBarStartForQuarter (quarterPosition);
                const auto quartersPerBeat = 4.0 / (double) barSignaturesConverter.getBarSignatureForQuarter (quarterPosition).denominator;
                const auto beatIndex = (int) beatDistance;
                const auto tickIndex = juce::roundToInt ((beatDistance - beatIndex) * quartersPerBeat * 960.0);

                text += newLine;
                text += String::formatted ("bar %d | beat %d | tick %03d", (barIndex >= 0) ? barIndex + 1 : barIndex, beatIndex + 1, tickIndex + 1);
                text += "  -  ";

                const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeSheetChords> chordsReader (selectedMusicalContext);

                if (chordsReader && chordsReader.getEventCount() > 0)
                {
                    const auto begin = chordsReader.begin();
                    const auto end = chordsReader.end();
                    auto it = begin;

                    while (it != end && it->position <= quarterPosition)
                        ++it;

                    if (it != begin)
                        --it;

                    const ARA::ChordInterpreter interpreter (true);
                    text += "chord ";
                    text += String (interpreter.getNameForChord (*it));
                }
                else
                {
                    text += "(no chords provided)";
                }
            }
        }

        setText (text, NotificationType::dontSendNotification);
    }

    // Copied from AudioPluginDemo.h: quick-and-dirty function to format a timecode string
    static String timeToTimecodeString (double seconds)
    {
        auto millisecs = roundToInt (seconds * 1000.0);
        auto absMillisecs = std::abs (millisecs);

        return String::formatted ("%02d:%02d:%02d.%03d",
                                  millisecs / 3600000,
                                  (absMillisecs / 60000) % 60,
                                  (absMillisecs / 1000)  % 60,
                                  absMillisecs % 1000);
    }

    PlayHeadState& playHeadState;
    ARAMusicalContext* selectedMusicalContext = nullptr;
};

class TrackHeader : public Component,
                    private ARARegionSequence::Listener,
                    private ARAEditorView::Listener
{
public:
    TrackHeader (ARAEditorView& editorView, ARARegionSequence& regionSequenceIn)
        : araEditorView (editorView), regionSequence (regionSequenceIn)
    {
        updateTrackName (regionSequence.getName());
        onNewSelection (araEditorView.getViewSelection());

        addAndMakeVisible (trackNameLabel);

        regionSequence.addListener (this);
        araEditorView.addListener (this);
    }

    ~TrackHeader() override
    {
        araEditorView.removeListener (this);
        regionSequence.removeListener (this);
    }

    void willUpdateRegionSequenceProperties (ARARegionSequence*, ARARegionSequence::PropertiesPtr newProperties) override
    {
        if (regionSequence.getName() != newProperties->name)
            updateTrackName (newProperties->name);
        if (regionSequence.getColor() != newProperties->color)
            repaint();
    }

    void resized() override
    {
        trackNameLabel.setBounds (getLocalBounds().reduced (2));
    }

    void paint (Graphics& g) override
    {
        const auto backgroundColour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
        g.setColour (isSelected ? backgroundColour.brighter() : backgroundColour);
        g.fillRoundedRectangle (getLocalBounds().reduced (2).toFloat(), 6.0f);
        g.setColour (backgroundColour.contrasting());
        g.drawRoundedRectangle (getLocalBounds().reduced (2).toFloat(), 6.0f, 1.0f);

        if (auto colour = regionSequence.getColor())
        {
            g.setColour (convertARAColour (colour));
            g.fillRect (getLocalBounds().removeFromTop (16).reduced (6));
            g.fillRect (getLocalBounds().removeFromBottom (16).reduced (6));
        }
    }

    void onNewSelection (const ARAViewSelection& viewSelection) override
    {
        const auto& selectedRegionSequences = viewSelection.getRegionSequences();
        const bool selected = std::find (selectedRegionSequences.begin(), selectedRegionSequences.end(), &regionSequence) != selectedRegionSequences.end();

        if (selected != isSelected)
        {
            isSelected = selected;
            repaint();
        }
    }

private:
    void updateTrackName (ARA::ARAUtf8String optionalName)
    {
        trackNameLabel.setText (optionalName ? optionalName : "No track name",
                                NotificationType::dontSendNotification);
    }

    ARAEditorView& araEditorView;
    ARARegionSequence& regionSequence;
    Label trackNameLabel;
    bool isSelected = false;
};

constexpr auto trackHeight = 60;

class VerticalLayoutViewportContent : public Component
{
public:
    void resized() override
    {
        auto bounds = getLocalBounds();

        for (auto* component : getChildren())
        {
            component->setBounds (bounds.removeFromTop (trackHeight));
            component->resized();
        }
    }
};

class VerticalLayoutViewport : public Viewport
{
public:
    VerticalLayoutViewport()
    {
        setViewedComponent (&content, false);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).brighter());
    }

    std::function<void (Rectangle<int>)> onVisibleAreaChanged;

    VerticalLayoutViewportContent content;

private:
    void visibleAreaChanged (const Rectangle<int>& newVisibleArea) override
    {
        NullCheckedInvocation::invoke (onVisibleAreaChanged, newVisibleArea);
    }
};

class OverlayComponent : public Component,
                         private Timer,
                         private TimeToViewScaling::Listener
{
public:
    class PlayheadMarkerComponent : public Component
    {
        void paint (Graphics& g) override { g.fillAll (Colours::yellow.darker (0.2f)); }
    };

    OverlayComponent (PlayHeadState& playHeadStateIn, TimeToViewScaling& timeToViewScalingIn)
        : playHeadState (playHeadStateIn), timeToViewScaling (timeToViewScalingIn)
    {
        addChildComponent (playheadMarker);
        setInterceptsMouseClicks (false, false);
        startTimerHz (30);

        timeToViewScaling.addListener (this);
    }

    ~OverlayComponent() override
    {
        timeToViewScaling.removeListener (this);

        stopTimer();
    }

    void resized() override
    {
        updatePlayHeadPosition();
    }

    void setHorizontalOffset (int offset)
    {
        horizontalOffset = offset;
    }

    void setSelectedTimeRange (std::optional<ARA::ARAContentTimeRange> timeRange)
    {
        selectedTimeRange = timeRange;
        repaint();
    }

    void zoomLevelChanged (double) override
    {
        updatePlayHeadPosition();
        repaint();
    }

    void paint (Graphics& g) override
    {
        if (selectedTimeRange)
        {
            auto bounds = getLocalBounds();
            bounds.setLeft (timeToViewScaling.getXForTime (selectedTimeRange->start));
            bounds.setRight (timeToViewScaling.getXForTime (selectedTimeRange->start + selectedTimeRange->duration));
            g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).brighter().withAlpha (0.3f));
            g.fillRect (bounds);
            g.setColour (Colours::whitesmoke.withAlpha (0.5f));
            g.drawRect (bounds);
        }
    }

private:
    void updatePlayHeadPosition()
    {
        if (playHeadState.isPlaying.load (std::memory_order_relaxed))
        {
            const auto markerX = timeToViewScaling.getXForTime (playHeadState.timeInSeconds.load (std::memory_order_relaxed));
            const auto playheadLine = getLocalBounds().withTrimmedLeft ((int) (markerX - markerWidth / 2.0) - horizontalOffset)
                                                      .removeFromLeft ((int) markerWidth);
            playheadMarker.setVisible (true);
            playheadMarker.setBounds (playheadLine);
        }
        else
        {
            playheadMarker.setVisible (false);
        }
    }

    void timerCallback() override
    {
        updatePlayHeadPosition();
    }

    static constexpr double markerWidth = 2.0;

    PlayHeadState& playHeadState;
    TimeToViewScaling& timeToViewScaling;
    int horizontalOffset = 0;
    std::optional<ARA::ARAContentTimeRange> selectedTimeRange;
    PlayheadMarkerComponent playheadMarker;
};

class DocumentView  : public Component,
                      public ChangeListener,
                      public ARAMusicalContext::Listener,
                      private ARADocument::Listener,
                      private ARAEditorView::Listener
{
public:
    DocumentView (ARAEditorView& editorView, PlayHeadState& playHeadState)
        : araEditorView (editorView),
          araDocument (*editorView.getDocumentController()->getDocument<ARADocument>()),
          rulersView (playHeadState, timeToViewScaling, araDocument),
          overlay (playHeadState, timeToViewScaling),
          playheadPositionLabel (playHeadState)
    {
        if (araDocument.getMusicalContexts().size() > 0)
            selectMusicalContext (araDocument.getMusicalContexts().front());

        addAndMakeVisible (rulersHeader);

        viewport.content.addAndMakeVisible (rulersView);

        viewport.onVisibleAreaChanged = [this] (const auto& r)
        {
            viewportHeightOffset = r.getY();
            overlay.setHorizontalOffset (r.getX());
            resized();
        };

        addAndMakeVisible (viewport);
        addAndMakeVisible (overlay);
        addAndMakeVisible (playheadPositionLabel);

        zoomControls.setZoomInCallback  ([this] { zoom (2.0); });
        zoomControls.setZoomOutCallback ([this] { zoom (0.5); });
        addAndMakeVisible (zoomControls);

        invalidateRegionSequenceViews();

        araDocument.addListener (this);
        araEditorView.addListener (this);
    }

    ~DocumentView() override
    {
        araEditorView.removeListener (this);
        araDocument.removeListener (this);
        selectMusicalContext (nullptr);
    }

    //==============================================================================
    // ARADocument::Listener overrides
    void didAddMusicalContextToDocument (ARADocument*, ARAMusicalContext* musicalContext) override
    {
        if (selectedMusicalContext == nullptr)
            selectMusicalContext (musicalContext);
    }

    void willDestroyMusicalContext (ARAMusicalContext* musicalContext) override
    {
        if (selectedMusicalContext == musicalContext)
            selectMusicalContext (nullptr);
    }

    void didReorderRegionSequencesInDocument (ARADocument*) override
    {
        invalidateRegionSequenceViews();
    }

    void didAddRegionSequenceToDocument (ARADocument*, ARARegionSequence*) override
    {
        invalidateRegionSequenceViews();
    }

    void willRemoveRegionSequenceFromDocument (ARADocument*, ARARegionSequence* regionSequence) override
    {
        removeRegionSequenceView (regionSequence);
    }

    void didEndEditing (ARADocument*) override
    {
        rebuildRegionSequenceViews();
        update();
    }

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster*) override
    {
        update();
    }

    //==============================================================================
    // ARAEditorView::Listener overrides
    void onNewSelection (const ARAViewSelection& viewSelection) override
    {
        auto getNewSelectedMusicalContext = [&viewSelection]() -> ARAMusicalContext*
        {
            if (! viewSelection.getRegionSequences().empty())
                return viewSelection.getRegionSequences<ARARegionSequence>().front()->getMusicalContext();
            else if (! viewSelection.getPlaybackRegions().empty())
                return viewSelection.getPlaybackRegions<ARAPlaybackRegion>().front()->getRegionSequence()->getMusicalContext();

            return nullptr;
        };

        if (auto* newSelectedMusicalContext = getNewSelectedMusicalContext())
            if (newSelectedMusicalContext != selectedMusicalContext)
                selectMusicalContext (newSelectedMusicalContext);

        if (const auto timeRange = viewSelection.getTimeRange())
            overlay.setSelectedTimeRange (*timeRange);
        else
            overlay.setSelectedTimeRange (std::nullopt);
    }

    void onHideRegionSequences (const std::vector<ARARegionSequence*>& regionSequences) override
    {
        hiddenRegionSequences = regionSequences;
        invalidateRegionSequenceViews();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        FlexBox fb;
        fb.justifyContent = FlexBox::JustifyContent::spaceBetween;
        fb.items.add (FlexItem (playheadPositionLabel).withWidth (450.0f).withMinWidth (250.0f));
        fb.items.add (FlexItem (zoomControls).withMinWidth (80.0f));
        fb.performLayout (bounds.removeFromBottom (40));

        auto headerBounds = bounds.removeFromLeft (headerWidth);
        rulersHeader.setBounds (headerBounds.removeFromTop (trackHeight));
        layOutVertically (headerBounds, trackHeaders, viewportHeightOffset);

        viewport.setBounds (bounds);
        overlay.setBounds (bounds.reduced (1));

        const auto width = jmax (timeToViewScaling.getXForTime (timelineLength), viewport.getWidth());
        const auto height = (int) (regionSequenceViews.size() + 1) * trackHeight;
        viewport.content.setSize (width, height);
        viewport.content.resized();
    }

    //==============================================================================
    static constexpr int headerWidth = 120;

private:
    struct RegionSequenceViewKey
    {
        explicit RegionSequenceViewKey (ARARegionSequence* regionSequence)
            : orderIndex (regionSequence->getOrderIndex()), sequence (regionSequence)
        {
        }

        bool operator< (const RegionSequenceViewKey& other) const
        {
            return std::tie (orderIndex, sequence) < std::tie (other.orderIndex, other.sequence);
        }

        ARA::ARAInt32 orderIndex;
        ARARegionSequence* sequence;
    };

    void selectMusicalContext (ARAMusicalContext* newSelectedMusicalContext)
    {
        if (auto oldContext = std::exchange (selectedMusicalContext, newSelectedMusicalContext);
            oldContext != selectedMusicalContext)
        {
            if (oldContext != nullptr)
                oldContext->removeListener (this);

            if (selectedMusicalContext != nullptr)
                selectedMusicalContext->addListener (this);

            rulersView.selectMusicalContext (selectedMusicalContext);
            playheadPositionLabel.selectMusicalContext (selectedMusicalContext);
        }
    }

    void zoom (double factor)
    {
        timeToViewScaling.zoom (factor);
        update();
    }

    template <typename T>
    void layOutVertically (Rectangle<int> bounds, T& components, int verticalOffset = 0)
    {
        bounds = bounds.withY (bounds.getY() - verticalOffset).withHeight (bounds.getHeight() + verticalOffset);

        for (auto& component : components)
        {
            component.second->setBounds (bounds.removeFromTop (trackHeight));
            component.second->resized();
        }
    }

    void update()
    {
        timelineLength = 0.0;

        for (const auto& view : regionSequenceViews)
            timelineLength = std::max (timelineLength, view.second->getPlaybackDuration());

        resized();
    }

    void addTrackViews (ARARegionSequence* regionSequence)
    {
        const auto insertIntoMap = [](auto& map, auto key, auto value) -> auto&
        {
            auto it = map.insert ({ std::move (key), std::move (value) });
            return *(it.first->second);
        };

        auto& regionSequenceView = insertIntoMap (
            regionSequenceViews,
            RegionSequenceViewKey { regionSequence },
            std::make_unique<RegionSequenceView> (araEditorView, timeToViewScaling, *regionSequence, waveformCache));

        regionSequenceView.addChangeListener (this);
        viewport.content.addAndMakeVisible (regionSequenceView);

        auto& trackHeader = insertIntoMap (trackHeaders,
                                           RegionSequenceViewKey { regionSequence },
                                           std::make_unique<TrackHeader> (araEditorView, *regionSequence));

        addAndMakeVisible (trackHeader);
    }

    void removeRegionSequenceView (ARARegionSequence* regionSequence)
    {
        const auto& view = regionSequenceViews.find (RegionSequenceViewKey { regionSequence });

        if (view != regionSequenceViews.cend())
        {
            removeChildComponent (view->second.get());
            regionSequenceViews.erase (view);
        }

        invalidateRegionSequenceViews();
    }

    void invalidateRegionSequenceViews()
    {
        regionSequenceViewsAreValid = false;
        rebuildRegionSequenceViews();
    }

    void rebuildRegionSequenceViews()
    {
        if (! regionSequenceViewsAreValid && ! araDocument.getDocumentController()->isHostEditingDocument())
        {
            for (auto& view : regionSequenceViews)
                removeChildComponent (view.second.get());

            regionSequenceViews.clear();

            for (auto& view : trackHeaders)
                removeChildComponent (view.second.get());

            trackHeaders.clear();

            for (auto* regionSequence : araDocument.getRegionSequences())
                if (std::find (hiddenRegionSequences.begin(), hiddenRegionSequences.end(), regionSequence) == hiddenRegionSequences.end())
                    addTrackViews (regionSequence);

            update();

            regionSequenceViewsAreValid = true;
        }
    }

    ARAEditorView& araEditorView;
    ARADocument& araDocument;

    bool regionSequenceViewsAreValid = false;

    TimeToViewScaling timeToViewScaling;
    double timelineLength = 0.0;

    ARAMusicalContext* selectedMusicalContext = nullptr;

    std::vector<ARARegionSequence*> hiddenRegionSequences;

    WaveformCache waveformCache;
    std::map<RegionSequenceViewKey, std::unique_ptr<TrackHeader>> trackHeaders;
    std::map<RegionSequenceViewKey, std::unique_ptr<RegionSequenceView>> regionSequenceViews;
    RulersHeader rulersHeader;
    RulersView rulersView;
    VerticalLayoutViewport viewport;
    OverlayComponent overlay;
    ZoomControls zoomControls;
    PlayheadPositionLabel playheadPositionLabel;
    TooltipWindow tooltip;

    int viewportHeightOffset = 0;
};


class ARADemoPluginProcessorEditor  : public AudioProcessorEditor,
                                      public AudioProcessorEditorARAExtension
{
public:
    explicit ARADemoPluginProcessorEditor (ARADemoPluginAudioProcessorImpl& p)
        : AudioProcessorEditor (&p),
          AudioProcessorEditorARAExtension (&p)
    {
        if (auto* editorView = getARAEditorView())
            documentView = std::make_unique<DocumentView> (*editorView, p.playHeadState);

        addAndMakeVisible (documentView.get());

        // ARA requires that plugin editors are resizable to support tight integration
        // into the host UI
        setResizable (true, false);
        setSize (800, 300);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        if (! isARAEditorView())
        {
            g.setColour (Colours::white);
            g.setFont (15.0f);
            g.drawFittedText ("ARA host isn't detected. This plugin only supports ARA mode",
                              getLocalBounds(),
                              Justification::centred,
                              1);
        }
    }

    void resized() override
    {
        if (documentView != nullptr)
            documentView->setBounds (getLocalBounds());
    }

private:
    std::unique_ptr<Component> documentView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADemoPluginProcessorEditor)
};

class ARADemoPluginAudioProcessor  : public ARADemoPluginAudioProcessorImpl
{
public:
    bool hasEditor() const override               { return true; }
    AudioProcessorEditor* createEditor() override { return new ARADemoPluginProcessorEditor (*this); }
};
