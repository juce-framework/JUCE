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
        startThread (7);  // Above default priority so playback is fluent, but below realtime
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
            buffer.clear();

        const auto numChannelsToCopy = std::min (inputBuffer->getNumChannels(), buffer.getNumChannels());

        for (auto samplesCopied = 0; samplesCopied < buffer.getNumSamples();)
        {
            const auto numSamplesToCopy =
                std::min (buffer.getNumSamples() - samplesCopied, (int) (loopRange.getEnd() - pos));

            for (int i = 0; i < numChannelsToCopy; ++i)
            {
                buffer.copyFrom (i, samplesCopied, *inputBuffer, i, (int) pos, numSamplesToCopy);
            }

            samplesCopied += numSamplesToCopy;
            pos += numSamplesToCopy;

            jassert (pos <= loopRange.getEnd());

            if (pos == loopRange.getEnd())
                pos = loopRange.getStart();
        }
    }

private:
    const AudioBuffer<float>* inputBuffer;
    Range<int64> loopRange;
    int64 pos;
};

class OptionalRange
{
public:
    using Type = Range<int64>;

    OptionalRange() : valid (false) {}
    explicit OptionalRange (Type valueIn) : valid (true), value (std::move (valueIn)) {}

    explicit operator bool() const noexcept { return valid; }

    const auto& operator*() const
    {
        jassert (valid);
        return value;
    }

private:
    bool valid;
    Type value;
};

//==============================================================================
// Returns the modified sample range in the output buffer.
inline OptionalRange readPlaybackRangeIntoBuffer (Range<double> playbackRange,
                                                  const ARAPlaybackRegion* playbackRegion,
                                                  AudioBuffer<float>& buffer,
                                                  const std::function<AudioFormatReader* (ARA::PlugIn::AudioSource*)>& getReader)
{
    const auto rangeInAudioModificationTime = playbackRange.movedToStartAt (playbackRange.getStart()
                                                                            - playbackRegion->getStartInAudioModificationTime());

    const auto audioSource = playbackRegion->getAudioModification()->getAudioSource();
    const auto audioModificationSampleRate = audioSource->getSampleRate();

    const Range<int64_t> sampleRangeInAudioModification {
        ARA::roundSamplePosition (rangeInAudioModificationTime.getStart() * audioModificationSampleRate),
        ARA::roundSamplePosition (rangeInAudioModificationTime.getEnd() * audioModificationSampleRate) - 1
    };

    const auto inputOffset = jlimit ((int64_t) 0, audioSource->getSampleCount(), sampleRangeInAudioModification.getStart());

    const auto outputOffset = -std::min (sampleRangeInAudioModification.getStart(), (int64_t) 0);

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
        return OptionalRange { {} };

    auto* reader = getReader (audioSource);

    if (reader != nullptr && reader->read (&buffer, (int) outputOffset, (int) readLength, inputOffset, true, true))
        return OptionalRange { { outputOffset, readLength } };

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

//==============================================================================
class PlaybackRenderer  : public ARAPlaybackRenderer
{
public:
    using ARAPlaybackRenderer::ARAPlaybackRenderer;

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

private:
    //==============================================================================
    // We're subclassing here only to provide a proper default c'tor for our shared resource

    SharedResourcePointer<SharedTimeSliceThread> sharedTimesliceThread;
    std::map<ARA::PlugIn::AudioSource*, PossiblyBufferedReader> audioSourceReaders;
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
    EditorRenderer (ARA::PlugIn::DocumentController* documentController, const PreviewState* previewStateIn)
        : ARAEditorRenderer (documentController), previewState (previewStateIn), previewBuffer()
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
        auto* sequence = dynamic_cast<ARARegionSequence*> (rs);
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

        return asyncConfigCallback.withLock ([&] (bool locked)
        {
            if (! locked)
                return true;

            if (positionInfo.getIsPlaying())
                return true;

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

                    if (lastPreviewTime != previewTime || lastPlaybackRegion != previewedRegion)
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
                            previewLooper = Looper (previewBuffer.get(), *rangeInOutput);
                        }
                    }
                    else
                    {
                        previewLooper.writeInto (buffer);
                    }
                }
            }

            return true;
        });
    }

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

    const PreviewState* previewState = nullptr;
    AsyncConfigurationCallback asyncConfigCallback { [this] { configure(); } };
    double lastPreviewTime = 0.0;
    ARAPlaybackRegion* lastPlaybackRegion = nullptr;
    std::unique_ptr<AudioBuffer<float>> previewBuffer;
    Looper previewLooper;

    double sampleRate = 48000.0;
    SharedResourcePointer<SharedTimeSliceThread> timeSliceThread;
    std::map<ARA::PlugIn::AudioSource*, std::unique_ptr<BufferingAudioReader>> audioSourceReaders;

    std::set<ARARegionSequence*> regionSequences;
};

//==============================================================================
class ARADemoPluginDocumentControllerSpecialisation  : public ARADocumentControllerSpecialisation
{
public:
    using ARADocumentControllerSpecialisation::ARADocumentControllerSpecialisation;

    PreviewState previewState;

protected:
    ARAPlaybackRenderer* doCreatePlaybackRenderer() noexcept override
    {
        return new PlaybackRenderer (getDocumentController());
    }

    EditorRenderer* doCreateEditorRenderer() noexcept override
    {
        return new EditorRenderer (getDocumentController(), &previewState);
    }

    bool doRestoreObjectsFromStream (ARAInputStream& input,
                                     const ARARestoreObjectsFilter* filter) noexcept override
    {
        ignoreUnused (input, filter);
        return false;
    }

    bool doStoreObjectsToStream (ARAOutputStream& output, const ARAStoreObjectsFilter* filter) noexcept override
    {
        ignoreUnused (output, filter);
        return false;
    }
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
         prepareToPlayForARA (sampleRate, samplesPerBlock, getMainBusNumOutputChannels(), getProcessingPrecision());
    }

    void releaseResources() override
    {
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

        if (! processBlockForARA (buffer, isRealtime(), getPlayHead()))
            processBlockBypassed (buffer, midiMessages);
    }

    //==============================================================================
    const String getName() const override                             { return "ARAPluginDemo"; }
    bool acceptsMidi() const override                                 { return true; }
    bool producesMidi() const override                                { return true; }
    double getTailLengthSeconds() const override                      { return 0.0; }

    //==============================================================================
    int getNumPrograms() override                                     { return 0; }
    int getCurrentProgram() override                                  { return 0; }
    void setCurrentProgram (int) override                             {}
    const String getProgramName (int) override                        { return "None"; }
    void changeProgramName (int, const String&) override              {}

    //==============================================================================
    void getStateInformation (MemoryBlock&) override                  {}
    void setStateInformation (const void*, int) override              {}

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
                           public ChangeListener
{
public:
    PlaybackRegionView (ARAPlaybackRegion& region, WaveformCache& cache)
        : playbackRegion (region), waveformCache (cache)
    {
        auto* audioSource = playbackRegion.getAudioModification()->getAudioSource();

        waveformCache.getOrCreateThumbnail (audioSource).addChangeListener (this);
    }

    ~PlaybackRegionView() override
    {
        waveformCache.getOrCreateThumbnail (playbackRegion.getAudioModification()->getAudioSource())
            .removeChangeListener (this);
    }

    void mouseDown (const MouseEvent& m) override
    {
        const auto relativeTime = (double) m.getMouseDownX() / getLocalBounds().getWidth();
        const auto previewTime = playbackRegion.getStartInPlaybackTime()
                                 + relativeTime * playbackRegion.getDurationInPlaybackTime();
        auto& previewState = ARADocumentControllerSpecialisation::getSpecialisedDocumentController<ARADemoPluginDocumentControllerSpecialisation> (playbackRegion.getDocumentController())->previewState;
        previewState.previewTime.store (previewTime);
        previewState.previewedRegion.store (&playbackRegion);
    }

    void mouseUp (const MouseEvent&) override
    {
        auto& previewState = ARADocumentControllerSpecialisation::getSpecialisedDocumentController<ARADemoPluginDocumentControllerSpecialisation> (playbackRegion.getDocumentController())->previewState;
        previewState.previewTime.store (0.0);
        previewState.previewedRegion.store (nullptr);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        repaint();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white.darker());
        g.setColour (Colours::darkgrey.darker());
        auto& thumbnail = waveformCache.getOrCreateThumbnail (playbackRegion.getAudioModification()->getAudioSource());
        thumbnail.drawChannels (g,
                                getLocalBounds(),
                                playbackRegion.getStartInAudioModificationTime(),
                                playbackRegion.getEndInAudioModificationTime(),
                                1.0f);
        g.setColour (Colours::black);
        g.drawRect (getLocalBounds());
    }

    void resized() override
    {
        repaint();
    }

private:
    ARAPlaybackRegion& playbackRegion;
    WaveformCache& waveformCache;
};

class RegionSequenceView : public Component,
                           public ARARegionSequence::Listener,
                           public ChangeBroadcaster,
                           private ARAPlaybackRegion::Listener
{
public:
    RegionSequenceView (ARARegionSequence& rs, WaveformCache& cache, double pixelPerSec)
        : regionSequence (rs), waveformCache (cache), zoomLevelPixelPerSecond (pixelPerSec)
    {
        regionSequence.addListener (this);

        for (auto* playbackRegion : regionSequence.getPlaybackRegions())
            createAndAddPlaybackRegionView (playbackRegion);

        updatePlaybackDuration();
    }

    ~RegionSequenceView() override
    {
        regionSequence.removeListener (this);

        for (const auto& it : playbackRegionViews)
            it.first->removeListener (this);
    }

    //==============================================================================
    // ARA Document change callback overrides
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

    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion*, ARAPlaybackRegion::PropertiesPtr) override
    {
    }

    void didUpdatePlaybackRegionProperties (ARAPlaybackRegion*) override
    {
        updatePlaybackDuration();
    }

    void resized() override
    {
        for (auto& pbr : playbackRegionViews)
        {
            const auto playbackRegion = pbr.first;
            pbr.second->setBounds (
                getLocalBounds()
                    .withTrimmedLeft (roundToInt (playbackRegion->getStartInPlaybackTime() * zoomLevelPixelPerSecond))
                    .withWidth (roundToInt (playbackRegion->getDurationInPlaybackTime() * zoomLevelPixelPerSecond)));
        }
    }

    auto getPlaybackDuration() const noexcept
    {
        return playbackDuration;
    }

    void setZoomLevel (double pixelPerSecond)
    {
        zoomLevelPixelPerSecond = pixelPerSecond;
        resized();
    }

private:
    void createAndAddPlaybackRegionView (ARAPlaybackRegion* playbackRegion)
    {
        playbackRegionViews[playbackRegion] = std::make_unique<PlaybackRegionView> (*playbackRegion, waveformCache);
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

    ARARegionSequence& regionSequence;
    WaveformCache& waveformCache;
    std::unordered_map<ARAPlaybackRegion*, std::unique_ptr<PlaybackRegionView>> playbackRegionViews;
    double playbackDuration = 0.0;
    double zoomLevelPixelPerSecond;
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

class TrackHeader : public Component
{
public:
    explicit TrackHeader (const ARARegionSequence& regionSequenceIn) : regionSequence (regionSequenceIn)
    {
        update();

        addAndMakeVisible (trackNameLabel);
    }

    void resized() override
    {
        trackNameLabel.setBounds (getLocalBounds().reduced (2));
    }

    void paint (Graphics& g) override
    {
        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.fillRoundedRectangle (getLocalBounds().reduced (2).toType<float>(), 6.0f);
        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting());
        g.drawRoundedRectangle (getLocalBounds().reduced (2).toType<float>(), 6.0f, 1.0f);
    }

private:
    void update()
    {
        const auto getWithDefaultValue =
            [] (const ARA::PlugIn::OptionalProperty<ARA::ARAUtf8String>& optional, String defaultValue)
        {
            if (const ARA::ARAUtf8String value = optional)
                return String (value);

            return defaultValue;
        };

        trackNameLabel.setText (getWithDefaultValue (regionSequence.getName(), "No track name"),
                                NotificationType::dontSendNotification);
    }

    const ARARegionSequence& regionSequence;
    Label trackNameLabel;
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

    void setOverlayComponent (Component* component)
    {
        if (overlayComponent != nullptr && overlayComponent != component)
            removeChildComponent (overlayComponent);

        addChildComponent (component);
        overlayComponent = component;
    }

private:
    Component* overlayComponent = nullptr;
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
                         private Timer
{
public:
    class PlayheadMarkerComponent : public Component
    {
        void paint (Graphics& g) override { g.fillAll (juce::Colours::yellow.darker (0.2f)); }
    };

    OverlayComponent(std::function<AudioPlayHead*()> getAudioPlayheadIn)
        : getAudioPlayhead (std::move (getAudioPlayheadIn))
    {
        addChildComponent (playheadMarker);
        setInterceptsMouseClicks (false, false);
        startTimerHz (30);
    }

    ~OverlayComponent() override
    {
        stopTimer();
    }

    void resized() override
    {
        doResize();
    }

    void setZoomLevel (double pixelPerSecondIn)
    {
        pixelPerSecond = pixelPerSecondIn;
    }

    void setHorizontalOffset (int offset)
    {
        horizontalOffset = offset;
    }

private:
    void doResize()
    {
        auto* aph = getAudioPlayhead();
        const auto info = aph->getPosition();

        if (info.hasValue() && info->getIsPlaying())
        {
            const auto markerX = info->getTimeInSeconds().orFallback (0) * pixelPerSecond;
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
        doResize();
    }

    static constexpr double markerWidth = 2.0;

    std::function<AudioPlayHead*()> getAudioPlayhead;
    double pixelPerSecond = 1.0;
    int horizontalOffset = 0;
    PlayheadMarkerComponent playheadMarker;
};

class DocumentView  : public Component,
                      public ChangeListener,
                      private ARADocument::Listener,
                      private ARAEditorView::Listener
{
public:
    explicit DocumentView (ARADocument& document, std::function<AudioPlayHead*()> getAudioPlayhead)
        : araDocument (document),
          overlay (std::move (getAudioPlayhead))
    {
        addAndMakeVisible (tracksBackground);

        viewport.onVisibleAreaChanged = [this] (const auto& r)
        {
            viewportHeightOffset = r.getY();
            overlay.setHorizontalOffset (r.getX());
            resized();
        };

        addAndMakeVisible (viewport);

        overlay.setZoomLevel (zoomLevelPixelPerSecond);
        addAndMakeVisible (overlay);

        zoomControls.setZoomInCallback  ([this] { zoom (2.0); });
        zoomControls.setZoomOutCallback ([this] { zoom (0.5); });
        addAndMakeVisible (zoomControls);

        invalidateRegionSequenceViews();
        araDocument.addListener (this);
    }

    ~DocumentView() override
    {
        araDocument.removeListener (this);
    }

    //==============================================================================
    // ARADocument::Listener overrides
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
    void onNewSelection (const ARA::PlugIn::ViewSelection&) override
    {
    }

    void onHideRegionSequences (const std::vector<ARARegionSequence*>&) override
    {
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        const auto bottomControlsBounds = bounds.removeFromBottom (40);
        const auto headerBounds = bounds.removeFromLeft (headerWidth).reduced (2);

        zoomControls.setBounds (bottomControlsBounds);
        layOutVertically (headerBounds, trackHeaders, viewportHeightOffset);
        tracksBackground.setBounds (bounds);
        viewport.setBounds (bounds);
        overlay.setBounds (bounds);
    }

    //==============================================================================
    void setZoomLevel (double pixelPerSecond)
    {
        zoomLevelPixelPerSecond = pixelPerSecond;

        for (const auto& view : regionSequenceViews)
            view.second->setZoomLevel (zoomLevelPixelPerSecond);

        overlay.setZoomLevel (zoomLevelPixelPerSecond);

        update();
    }

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

    void zoom (double factor)
    {
        zoomLevelPixelPerSecond = jlimit (minimumZoom, minimumZoom * 32, zoomLevelPixelPerSecond * factor);
        setZoomLevel (zoomLevelPixelPerSecond);
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

        const Rectangle<int> timelineSize (roundToInt (timelineLength * zoomLevelPixelPerSecond),
                                           (int) regionSequenceViews.size() * trackHeight);
        viewport.content.setSize (timelineSize.getWidth(), timelineSize.getHeight());
        viewport.content.resized();

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
            std::make_unique<RegionSequenceView> (*regionSequence, waveformCache, zoomLevelPixelPerSecond));

        regionSequenceView.addChangeListener (this);
        viewport.content.addAndMakeVisible (regionSequenceView);

        auto& trackHeader = insertIntoMap (trackHeaders,
                                           RegionSequenceViewKey { regionSequence },
                                           std::make_unique<TrackHeader> (*regionSequence));

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
            {
                addTrackViews (regionSequence);
            }

            update();

            regionSequenceViewsAreValid = true;
        }
    }

    class TracksBackgroundComponent : public Component
    {
        void paint (Graphics& g) override
        {
            g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).brighter());
        }
    };

    static constexpr auto minimumZoom = 10.0;
    static constexpr auto trackHeight = 60;

    ARADocument& araDocument;

    bool regionSequenceViewsAreValid = false;
    double timelineLength = 0;
    double zoomLevelPixelPerSecond = minimumZoom * 4;

    WaveformCache waveformCache;
    TracksBackgroundComponent tracksBackground;
    std::map<RegionSequenceViewKey, std::unique_ptr<TrackHeader>> trackHeaders;
    std::map<RegionSequenceViewKey, std::unique_ptr<RegionSequenceView>> regionSequenceViews;
    VerticalLayoutViewport viewport;
    OverlayComponent overlay;
    ZoomControls zoomControls;

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
        {
            auto* document = ARADocumentControllerSpecialisation::getSpecialisedDocumentController(editorView->getDocumentController())->getDocument();
            documentView = std::make_unique<DocumentView> (*document,
                                                           [this]() { return getAudioProcessor()->getPlayHead(); });
        }

        addAndMakeVisible (documentView.get());

        // ARA requires that plugin editors are resizable to support tight integration
        // into the host UI
        setResizable (true, false);
        setSize (400, 300);
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
