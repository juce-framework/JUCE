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

#include <juce_core/system/juce_TargetPlatform.h>

#if JUCE_PLUGINHOST_ARA && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)

#include <JuceHeader.h>

#include <ARA_API/ARAInterface.h>
#include <ARA_Library/Dispatch/ARAHostDispatch.h>

class FileAudioSource
{
    auto getAudioSourceProperties() const
    {
        auto properties = ARAHostModel::AudioSource::getEmptyProperties();
        properties.name = formatReader->getFile().getFullPathName().toRawUTF8();
        properties.persistentID = formatReader->getFile().getFullPathName().toRawUTF8();
        properties.sampleCount = formatReader->lengthInSamples;
        properties.sampleRate = formatReader->sampleRate;
        properties.channelCount = (int) formatReader->numChannels;
        properties.merits64BitSamples = false;
        return properties;
    }

public:
    FileAudioSource (ARA::Host::DocumentController& dc, const juce::File& file)
        : formatReader ([&file]
          {
              auto result = rawToUniquePtr (WavAudioFormat().createMemoryMappedReader (file));
              result->mapEntireFile();
              return result;
          }()),
          audioSource (Converter::toHostRef (this), dc, getAudioSourceProperties())
    {
        audioSource.enableAudioSourceSamplesAccess (true);
    }

    bool readAudioSamples (float* const* buffers, int64 startSample, int64 numSamples)
    {
        // TODO: the ARA interface defines numSamples as int64. We should do multiple reads if necessary with the reader.
        if (numSamples > std::numeric_limits<int>::max())
            return false;

        return formatReader->read (buffers, (int) formatReader->numChannels, startSample, (int) (numSamples));
    }

    bool readAudioSamples (double* const* buffers, int64 startSample, int64 numSamples)
    {
        ignoreUnused (buffers, startSample, numSamples);
        return false;
    }

    MemoryMappedAudioFormatReader& getFormatReader() const { return *formatReader; }

    auto getPluginRef() const { return audioSource.getPluginRef(); }

    auto& getSource() { return audioSource; }

    using Converter = ARAHostModel::ConversionFunctions<FileAudioSource*, ARA::ARAAudioSourceHostRef>;

private:
    std::unique_ptr<MemoryMappedAudioFormatReader> formatReader;
    ARAHostModel::AudioSource audioSource;
};

//==============================================================================
class MusicalContext
{
    auto getMusicalContextProperties() const
    {
        auto properties = ARAHostModel::MusicalContext::getEmptyProperties();
        properties.name = "MusicalContext";
        properties.orderIndex = 0;
        properties.color = nullptr;
        return properties;
    }

public:
    MusicalContext (ARA::Host::DocumentController& dc)
        : context (Converter::toHostRef (this), dc, getMusicalContextProperties())
    {
    }

    auto getPluginRef() const { return context.getPluginRef(); }

private:
    using Converter = ARAHostModel::ConversionFunctions<MusicalContext*, ARA::ARAMusicalContextHostRef>;

    ARAHostModel::MusicalContext context;
};

//==============================================================================
class RegionSequence
{
    auto getRegionSequenceProperties() const
    {
        auto properties = ARAHostModel::RegionSequence::getEmptyProperties();
        properties.name = name.toRawUTF8();
        properties.orderIndex = 0;
        properties.musicalContextRef = context.getPluginRef();
        properties.color = nullptr;
        return properties;
    }

public:
    RegionSequence (ARA::Host::DocumentController& dc, MusicalContext& contextIn, String nameIn)
        : context (contextIn),
          name (std::move (nameIn)),
          sequence (Converter::toHostRef (this), dc, getRegionSequenceProperties())
    {
    }

    auto& getMusicalContext() const { return context; }
    auto getPluginRef() const { return sequence.getPluginRef(); }

private:
    using Converter = ARAHostModel::ConversionFunctions<RegionSequence*, ARA::ARARegionSequenceHostRef>;

    MusicalContext& context;
    String name;
    ARAHostModel::RegionSequence sequence;
};

class AudioModification
{
    auto getProperties() const
    {
        auto properties = ARAHostModel::AudioModification::getEmptyProperties();
        properties.persistentID = "x";
        return properties;
    }

public:
    AudioModification (ARA::Host::DocumentController& dc, FileAudioSource& source)
        : modification (Converter::toHostRef (this), dc, source.getSource(), getProperties())
    {
    }

    auto& getModification() { return modification; }

private:
    using Converter = ARAHostModel::ConversionFunctions<AudioModification*, ARA::ARAAudioModificationHostRef>;

    ARAHostModel::AudioModification modification;
};

//==============================================================================
class PlaybackRegion
{
    auto getPlaybackRegionProperties() const
    {
        auto properties = ARAHostModel::PlaybackRegion::getEmptyProperties();
        properties.transformationFlags = ARA::kARAPlaybackTransformationNoChanges;
        properties.startInModificationTime = 0.0;
        const auto& formatReader = audioSource.getFormatReader();
        properties.durationInModificationTime = (double) formatReader.lengthInSamples / formatReader.sampleRate;
        properties.startInPlaybackTime = 0.0;
        properties.durationInPlaybackTime = properties.durationInModificationTime;
        properties.musicalContextRef = sequence.getMusicalContext().getPluginRef();
        properties.regionSequenceRef = sequence.getPluginRef();

        properties.name = nullptr;
        properties.color = nullptr;
        return properties;
    }

public:
    PlaybackRegion (ARA::Host::DocumentController& dc,
                    RegionSequence& s,
                    AudioModification& m,
                    FileAudioSource& source)
        : sequence (s),
          audioSource (source),
          region (Converter::toHostRef (this), dc, m.getModification(), getPlaybackRegionProperties())
    {
        jassert (source.getPluginRef() == m.getModification().getAudioSource().getPluginRef());
    }

    auto& getPlaybackRegion() { return region; }

private:
    using Converter = ARAHostModel::ConversionFunctions<PlaybackRegion*, ARA::ARAPlaybackRegionHostRef>;

    RegionSequence& sequence;
    FileAudioSource& audioSource;
    ARAHostModel::PlaybackRegion region;
};

//==============================================================================
class AudioAccessController : public ARA::Host::AudioAccessControllerInterface
{
public:
    ARA::ARAAudioReaderHostRef createAudioReaderForSource (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                                           bool use64BitSamples) noexcept override
    {
        auto audioReader = std::make_unique<AudioReader> (audioSourceHostRef, use64BitSamples);
        auto audioReaderHostRef = Converter::toHostRef (audioReader.get());
        auto* readerPtr = audioReader.get();
        audioReaders.emplace (readerPtr, std::move (audioReader));
        return audioReaderHostRef;
    }

    bool readAudioSamples (ARA::ARAAudioReaderHostRef readerRef,
                           ARA::ARASamplePosition samplePosition,
                           ARA::ARASampleCount samplesPerChannel,
                           void* const* buffers) noexcept override
    {
        const auto use64BitSamples = Converter::fromHostRef (readerRef)->use64Bit;
        auto* audioSource = FileAudioSource::Converter::fromHostRef (Converter::fromHostRef (readerRef)->sourceHostRef);

        if (use64BitSamples)
            return audioSource->readAudioSamples (
                reinterpret_cast<double* const*> (buffers), samplePosition, samplesPerChannel);

        return audioSource->readAudioSamples (
            reinterpret_cast<float* const*> (buffers), samplePosition, samplesPerChannel);
    }

    void destroyAudioReader (ARA::ARAAudioReaderHostRef readerRef) noexcept override
    {
        audioReaders.erase (Converter::fromHostRef (readerRef));
    }

private:
    struct AudioReader
    {
        AudioReader (ARA::ARAAudioSourceHostRef source, bool use64BitSamples)
            : sourceHostRef (source), use64Bit (use64BitSamples)
        {
        }

        ARA::ARAAudioSourceHostRef sourceHostRef;
        bool use64Bit;
    };

    using Converter = ARAHostModel::ConversionFunctions<AudioReader*, ARA::ARAAudioReaderHostRef>;

    std::map<AudioReader*, std::unique_ptr<AudioReader>> audioReaders;
};

class ArchivingController : public ARA::Host::ArchivingControllerInterface
{
public:
    using ReaderConverter = ARAHostModel::ConversionFunctions<MemoryBlock*, ARA::ARAArchiveReaderHostRef>;
    using WriterConverter = ARAHostModel::ConversionFunctions<MemoryOutputStream*, ARA::ARAArchiveWriterHostRef>;

    ARA::ARASize getArchiveSize (ARA::ARAArchiveReaderHostRef archiveReaderHostRef) noexcept override
    {
        return (ARA::ARASize) ReaderConverter::fromHostRef (archiveReaderHostRef)->getSize();
    }

    bool readBytesFromArchive (ARA::ARAArchiveReaderHostRef archiveReaderHostRef,
                               ARA::ARASize position,
                               ARA::ARASize length,
                               ARA::ARAByte* buffer) noexcept override
    {
        auto* archiveReader = ReaderConverter::fromHostRef (archiveReaderHostRef);

        if ((position + length) <= archiveReader->getSize())
        {
            std::memcpy (buffer, addBytesToPointer (archiveReader->getData(), position), length);
            return true;
        }

        return false;
    }

    bool writeBytesToArchive (ARA::ARAArchiveWriterHostRef archiveWriterHostRef,
                              ARA::ARASize position,
                              ARA::ARASize length,
                              const ARA::ARAByte* buffer) noexcept override
    {
        auto* archiveWriter = WriterConverter::fromHostRef (archiveWriterHostRef);

        if (archiveWriter->setPosition ((int64) position) && archiveWriter->write (buffer, length))
            return true;

        return false;
    }

    void notifyDocumentArchivingProgress (float value) noexcept override { ignoreUnused (value); }

    void notifyDocumentUnarchivingProgress (float value) noexcept override { ignoreUnused (value); }

    ARA::ARAPersistentID getDocumentArchiveID (ARA::ARAArchiveReaderHostRef archiveReaderHostRef) noexcept override
    {
        ignoreUnused (archiveReaderHostRef);

        return nullptr;
    }
};

class ContentAccessController : public ARA::Host::ContentAccessControllerInterface
{
public:
    using Converter = ARAHostModel::ConversionFunctions<ARA::ARAContentType, ARA::ARAContentReaderHostRef>;

    bool isMusicalContextContentAvailable (ARA::ARAMusicalContextHostRef musicalContextHostRef,
                                           ARA::ARAContentType type) noexcept override
    {
        ignoreUnused (musicalContextHostRef);

        return (type == ARA::kARAContentTypeTempoEntries || type == ARA::kARAContentTypeBarSignatures);
    }

    ARA::ARAContentGrade getMusicalContextContentGrade (ARA::ARAMusicalContextHostRef musicalContextHostRef,
                                                        ARA::ARAContentType type) noexcept override
    {
        ignoreUnused (musicalContextHostRef, type);

        return ARA::kARAContentGradeInitial;
    }

    ARA::ARAContentReaderHostRef
        createMusicalContextContentReader (ARA::ARAMusicalContextHostRef musicalContextHostRef,
                                           ARA::ARAContentType type,
                                           const ARA::ARAContentTimeRange* range) noexcept override
    {
        ignoreUnused (musicalContextHostRef, range);

        return Converter::toHostRef (type);
    }

    bool isAudioSourceContentAvailable (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                        ARA::ARAContentType type) noexcept override
    {
        ignoreUnused (audioSourceHostRef, type);

        return false;
    }

    ARA::ARAContentGrade getAudioSourceContentGrade (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                                     ARA::ARAContentType type) noexcept override
    {
        ignoreUnused (audioSourceHostRef, type);

        return 0;
    }

    ARA::ARAContentReaderHostRef
        createAudioSourceContentReader (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                        ARA::ARAContentType type,
                                        const ARA::ARAContentTimeRange* range) noexcept override
    {
        ignoreUnused (audioSourceHostRef, type, range);

        return nullptr;
    }

    ARA::ARAInt32 getContentReaderEventCount (ARA::ARAContentReaderHostRef contentReaderHostRef) noexcept override
    {
        const auto contentType = Converter::fromHostRef (contentReaderHostRef);

        if (contentType == ARA::kARAContentTypeTempoEntries || contentType == ARA::kARAContentTypeBarSignatures)
            return 2;

        return 0;
    }

    const void* getContentReaderDataForEvent (ARA::ARAContentReaderHostRef contentReaderHostRef,
                                              ARA::ARAInt32 eventIndex) noexcept override
    {
        if (Converter::fromHostRef (contentReaderHostRef) == ARA::kARAContentTypeTempoEntries)
        {
            if (eventIndex == 0)
            {
                tempoEntry.timePosition = 0.0;
                tempoEntry.quarterPosition = 0.0;
            }
            else if (eventIndex == 1)
            {
                tempoEntry.timePosition = 2.0;
                tempoEntry.quarterPosition = 4.0;
            }

            return &tempoEntry;
        }
        else if (Converter::fromHostRef (contentReaderHostRef) == ARA::kARAContentTypeBarSignatures)
        {
            if (eventIndex == 0)
            {
                barSignature.position = 0.0;
                barSignature.numerator = 4;
                barSignature.denominator = 4;
            }

            if (eventIndex == 1)
            {
                barSignature.position = 1.0;
                barSignature.numerator = 4;
                barSignature.denominator = 4;
            }

            return &barSignature;
        }

        jassertfalse;
        return nullptr;
    }

    void destroyContentReader (ARA::ARAContentReaderHostRef contentReaderHostRef) noexcept override
    {
        ignoreUnused (contentReaderHostRef);
    }

    ARA::ARAContentTempoEntry tempoEntry;
    ARA::ARAContentBarSignature barSignature;
};

class ModelUpdateController : public ARA::Host::ModelUpdateControllerInterface
{
public:
    void notifyAudioSourceAnalysisProgress (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                            ARA::ARAAnalysisProgressState state,
                                            float value) noexcept override
    {
        ignoreUnused (audioSourceHostRef, state, value);
    }

    void notifyAudioSourceContentChanged (ARA::ARAAudioSourceHostRef audioSourceHostRef,
                                          const ARA::ARAContentTimeRange* range,
                                          ARA::ContentUpdateScopes scopeFlags) noexcept override
    {
        ignoreUnused (audioSourceHostRef, range, scopeFlags);
    }

    void notifyAudioModificationContentChanged (ARA::ARAAudioModificationHostRef audioModificationHostRef,
                                                const ARA::ARAContentTimeRange* range,
                                                ARA::ContentUpdateScopes scopeFlags) noexcept override
    {
        ignoreUnused (audioModificationHostRef, range, scopeFlags);
    }

    void notifyPlaybackRegionContentChanged (ARA::ARAPlaybackRegionHostRef playbackRegionHostRef,
                                             const ARA::ARAContentTimeRange* range,
                                             ARA::ContentUpdateScopes scopeFlags) noexcept override
    {
        ignoreUnused (playbackRegionHostRef, range, scopeFlags);
    }
};

class PlaybackController : public ARA::Host::PlaybackControllerInterface
{
public:
    void requestStartPlayback() noexcept override {}
    void requestStopPlayback() noexcept override {}

    void requestSetPlaybackPosition (ARA::ARATimePosition timePosition) noexcept override
    {
        ignoreUnused (timePosition);
    }

    void requestSetCycleRange (ARA::ARATimePosition startTime, ARA::ARATimeDuration duration) noexcept override
    {
        ignoreUnused (startTime, duration);
    }

    void requestEnableCycle (bool enable) noexcept override { ignoreUnused (enable); }
};

struct SimplePlayHead  : public juce::AudioPlayHead
{
    Optional<PositionInfo> getPosition() const override
    {
        PositionInfo result;
        result.setTimeInSamples (timeInSamples.load());
        result.setIsPlaying (isPlaying.load());
        return result;
    }

    std::atomic<int64_t> timeInSamples { 0 };
    std::atomic<bool> isPlaying { false };
};

struct HostPlaybackController
{
    virtual ~HostPlaybackController() = default;

    virtual void setPlaying (bool isPlaying) = 0;
    virtual void goToStart() = 0;
    virtual File getAudioSource() const = 0;
    virtual void setAudioSource (File audioSourceFile) = 0;
    virtual void clearAudioSource() = 0;
};

class AudioSourceComponent  : public Component,
                              public FileDragAndDropTarget,
                              public ChangeListener
{
public:
    explicit AudioSourceComponent (HostPlaybackController& controller, juce::ChangeBroadcaster& bc)
        : hostPlaybackController (controller),
          broadcaster (bc),
          waveformComponent (*this)
    {
        audioSourceLabel.setText ("You can drag and drop .wav files here", NotificationType::dontSendNotification);

        addAndMakeVisible (audioSourceLabel);
        addAndMakeVisible (waveformComponent);

        playButton.setButtonText ("Play / Pause");
        playButton.onClick = [this]
        {
            isPlaying = ! isPlaying;
            hostPlaybackController.setPlaying (isPlaying);
        };

        goToStartButton.setButtonText ("Go to start");
        goToStartButton.onClick = [this] { hostPlaybackController.goToStart(); };

        addAndMakeVisible (goToStartButton);
        addAndMakeVisible (playButton);

        broadcaster.addChangeListener (this);

        update();
    }

    ~AudioSourceComponent() override
    {
        broadcaster.removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        update();
    }

    void resized() override
    {
        auto localBounds = getLocalBounds();
        auto buttonsArea = localBounds.removeFromBottom (40).reduced (5);
        auto waveformArea = localBounds.removeFromBottom (150).reduced (5);

        juce::FlexBox fb;
        fb.justifyContent = juce::FlexBox::JustifyContent::center;
        fb.alignContent = juce::FlexBox::AlignContent::center;

        fb.items = { juce::FlexItem (goToStartButton).withMinWidth (100.0f).withMinHeight ((float) buttonsArea.getHeight()),
                     juce::FlexItem (playButton).withMinWidth (100.0f).withMinHeight ((float) buttonsArea.getHeight()) };

        fb.performLayout (buttonsArea);

        waveformComponent.setBounds (waveformArea);

        audioSourceLabel.setBounds (localBounds);
    }

    bool isInterestedInFileDrag (const StringArray& files) override
    {
        if (files.size() != 1)
            return false;

        if (files.getReference (0).endsWithIgnoreCase (".wav"))
            return true;

        return false;
    }

    void update()
    {
        const auto currentAudioSource = hostPlaybackController.getAudioSource();

        if (currentAudioSource.existsAsFile())
        {
            waveformComponent.setSource (currentAudioSource);
            audioSourceLabel.setText (currentAudioSource.getFullPathName(),
                                      NotificationType::dontSendNotification);
        }
        else
        {
            waveformComponent.clearSource();
            audioSourceLabel.setText ("You can drag and drop .wav files here", NotificationType::dontSendNotification);
        }
    }

    void filesDropped (const StringArray& files, int, int) override
    {
        hostPlaybackController.setAudioSource (files.getReference (0));
        update();
    }

private:
    class WaveformComponent  : public Component,
                               public ChangeListener
    {
    public:
        WaveformComponent (AudioSourceComponent& p)
            : parent (p),
              thumbCache (7),
              audioThumb (128, formatManager, thumbCache)
        {
            setWantsKeyboardFocus (true);
            formatManager.registerBasicFormats();
            audioThumb.addChangeListener (this);
        }

        ~WaveformComponent() override
        {
            audioThumb.removeChangeListener (this);
        }

        void mouseDown (const MouseEvent&) override
        {
            isSelected = true;
            repaint();
        }

        void changeListenerCallback (ChangeBroadcaster*) override
        {
            repaint();
        }

        void paint (juce::Graphics& g) override
        {
            if (! isEmpty)
            {
                auto rect = getLocalBounds();

                const auto waveformColour = Colours::cadetblue;

                if (rect.getWidth() > 2)
                {
                    g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
                    g.drawRect (rect);
                    rect.reduce (1, 1);
                    g.setColour (waveformColour.darker (1.0f));
                    g.fillRect (rect);
                }

                g.setColour (Colours::cadetblue);
                audioThumb.drawChannels (g, rect, 0.0, audioThumb.getTotalLength(), 1.0f);
            }
        }

        void setSource (const File& source)
        {
            isEmpty = false;
            audioThumb.setSource (new FileInputSource (source));
        }

        void clearSource()
        {
            isEmpty = true;
            isSelected = false;
            audioThumb.clear();
        }

        bool keyPressed (const KeyPress& key) override
        {
            if (isSelected && key == KeyPress::deleteKey)
            {
                parent.hostPlaybackController.clearAudioSource();
                return true;
            }

            return false;
        }

    private:
        AudioSourceComponent& parent;

        bool isEmpty = true;
        bool isSelected = false;
        AudioFormatManager formatManager;
        AudioThumbnailCache thumbCache;
        AudioThumbnail audioThumb;
    };

    HostPlaybackController& hostPlaybackController;
    juce::ChangeBroadcaster& broadcaster;
    Label audioSourceLabel;
    WaveformComponent waveformComponent;
    bool isPlaying { false };
    TextButton playButton, goToStartButton;
};

class ARAPluginInstanceWrapper  : public AudioPluginInstance
{
public:
    class ARATestHost  : public HostPlaybackController,
                         public juce::ChangeBroadcaster
    {
    public:
        class Editor  : public AudioProcessorEditor
        {
        public:
            explicit Editor (ARATestHost& araTestHost)
                : AudioProcessorEditor (araTestHost.getAudioPluginInstance()),
                  audioSourceComponent (araTestHost, araTestHost)
            {
                audioSourceComponent.update();
                addAndMakeVisible (audioSourceComponent);
                setSize (512, 220);
            }

            ~Editor() override { getAudioProcessor()->editorBeingDeleted (this); }

            void resized() override { audioSourceComponent.setBounds (getLocalBounds()); }

        private:
            AudioSourceComponent audioSourceComponent;
        };

        explicit ARATestHost (ARAPluginInstanceWrapper& instanceIn)
            : instance (instanceIn)
        {
            if (instance.inner->getPluginDescription().hasARAExtension)
            {
                instance.inner->setPlayHead (&playHead);

                createARAFactoryAsync (*instance.inner, [this] (ARAFactoryWrapper araFactory)
                                                        {
                                                            init (std::move (araFactory));
                                                        });
            }
        }

        void init (ARAFactoryWrapper araFactory)
        {
            if (araFactory.get() != nullptr)
            {
                documentController = ARAHostDocumentController::create (std::move (araFactory),
                                                                        "AudioPluginHostDocument",
                                                                        std::make_unique<AudioAccessController>(),
                                                                        std::make_unique<ArchivingController>(),
                                                                        std::make_unique<ContentAccessController>(),
                                                                        std::make_unique<ModelUpdateController>(),
                                                                        std::make_unique<PlaybackController>());

                if (documentController != nullptr)
                {
                    const auto allRoles = ARA::kARAPlaybackRendererRole | ARA::kARAEditorRendererRole | ARA::kARAEditorViewRole;
                    const auto plugInExtensionInstance = documentController->bindDocumentToPluginInstance (*instance.inner,
                                                                                                           allRoles,
                                                                                                           allRoles);
                    playbackRenderer = plugInExtensionInstance.getPlaybackRendererInterface();
                    editorRenderer   = plugInExtensionInstance.getEditorRendererInterface();
                    synchronizeStateWithDocumentController();
                }
                else
                    jassertfalse;
            }
            else
                jassertfalse;
        }

        void getStateInformation (juce::MemoryBlock& b)
        {
            std::lock_guard<std::mutex> configurationLock (instance.innerMutex);

            if (context != nullptr)
                context->getStateInformation (b);
        }

        void setStateInformation (const void* d, int s)
        {
            {
                std::lock_guard<std::mutex> lock { contextUpdateSourceMutex };
                contextUpdateSource = ContextUpdateSource { d, s };
            }

            synchronise();
        }

        ~ARATestHost() override { instance.inner->releaseResources(); }

        void afterProcessBlock (int numSamples)
        {
            const auto isPlayingNow = isPlaying.load();
            playHead.isPlaying.store (isPlayingNow);

            if (isPlayingNow)
            {
                const auto currentAudioSourceLength = audioSourceLength.load();
                const auto currentPlayHeadPosition = playHead.timeInSamples.load();

                // Rudimentary attempt to not seek beyond our sample data, assuming a fairly stable numSamples
                // value. We should gain control over calling the AudioProcessorGraph's processBlock() calls so
                // that we can do sample precise looping.
                if (currentAudioSourceLength - currentPlayHeadPosition < numSamples)
                    playHead.timeInSamples.store (0);
                else
                    playHead.timeInSamples.fetch_add (numSamples);
            }

            if (goToStartSignal.exchange (false))
                playHead.timeInSamples.store (0);
        }

        File getAudioSource() const override
        {
            std::lock_guard<std::mutex> lock { instance.innerMutex };

            if (context != nullptr)
                return context->audioFile;

            return {};
        }

        void setAudioSource (File audioSourceFile) override
        {
            if (audioSourceFile.existsAsFile())
            {
                {
                    std::lock_guard<std::mutex> lock { contextUpdateSourceMutex };
                    contextUpdateSource = ContextUpdateSource (std::move (audioSourceFile));
                }

                synchronise();
            }
        }

        void clearAudioSource() override
        {
            {
                std::lock_guard<std::mutex> lock { contextUpdateSourceMutex };
                contextUpdateSource = ContextUpdateSource (ContextUpdateSource::Type::reset);
            }

            synchronise();
        }

        void setPlaying (bool isPlayingIn) override { isPlaying.store (isPlayingIn); }

        void goToStart() override { goToStartSignal.store (true); }

        Editor* createEditor() { return new Editor (*this); }

        AudioPluginInstance& getAudioPluginInstance() { return instance; }

    private:
        /**  Use this to put the plugin in an unprepared state for the duration of adding and removing PlaybackRegions
             to and from Renderers.
        */
        class ScopedPluginDeactivator
        {
        public:
            explicit ScopedPluginDeactivator (ARAPluginInstanceWrapper& inst) : instance (inst)
            {
                if (instance.prepareToPlayParams.isValid)
                    instance.inner->releaseResources();
            }

            ~ScopedPluginDeactivator()
            {
                if (instance.prepareToPlayParams.isValid)
                    instance.inner->prepareToPlay (instance.prepareToPlayParams.sampleRate,
                                                   instance.prepareToPlayParams.samplesPerBlock);
            }

        private:
            ARAPluginInstanceWrapper& instance;

            JUCE_DECLARE_NON_COPYABLE (ScopedPluginDeactivator)
        };

        class ContextUpdateSource
        {
        public:
            enum class Type
            {
                empty,
                audioSourceFile,
                stateInformation,
                reset
            };

            ContextUpdateSource() = default;

            explicit ContextUpdateSource (const File& file)
                : type (Type::audioSourceFile),
                  audioSourceFile (file)
            {
            }

            ContextUpdateSource (const void* d, int s)
                : type (Type::stateInformation),
                  stateInformation (d, (size_t) s)
            {
            }

            ContextUpdateSource (Type t) : type (t)
            {
                jassert (t == Type::reset);
            }

            Type getType() const { return type; }

            const File& getAudioSourceFile() const
            {
                jassert (type == Type::audioSourceFile);

                return audioSourceFile;
            }

            const MemoryBlock& getStateInformation() const
            {
                jassert (type == Type::stateInformation);

                return stateInformation;
            }

        private:
            Type type = Type::empty;

            File audioSourceFile;
            MemoryBlock stateInformation;
        };

        void synchronise()
        {
            const SpinLock::ScopedLockType scope (instance.innerProcessBlockFlag);
            std::lock_guard<std::mutex> configurationLock (instance.innerMutex);
            synchronizeStateWithDocumentController();
        }

        void synchronizeStateWithDocumentController()
        {
            bool resetContext = false;

            auto newContext = [&]() -> std::unique_ptr<Context>
            {
                std::lock_guard<std::mutex> lock { contextUpdateSourceMutex };

                switch (contextUpdateSource.getType())
                {
                    case ContextUpdateSource::Type::empty:
                        return {};

                    case ContextUpdateSource::Type::audioSourceFile:
                        if (! (contextUpdateSource.getAudioSourceFile().existsAsFile()))
                            return {};

                        {
                            const ARAEditGuard editGuard (documentController->getDocumentController());
                            return std::make_unique<Context> (documentController->getDocumentController(),
                                                              contextUpdateSource.getAudioSourceFile());
                        }

                    case ContextUpdateSource::Type::stateInformation:
                        jassert (contextUpdateSource.getStateInformation().getSize() <= std::numeric_limits<int>::max());

                        return Context::createFromStateInformation (documentController->getDocumentController(),
                                                                    contextUpdateSource.getStateInformation().getData(),
                                                                    (int) contextUpdateSource.getStateInformation().getSize());

                    case ContextUpdateSource::Type::reset:
                        resetContext = true;
                        return {};
                }

                jassertfalse;
                return {};
            }();

            if (newContext != nullptr)
            {
                {
                    ScopedPluginDeactivator deactivator (instance);

                    context = std::move (newContext);
                    audioSourceLength.store (context->fileAudioSource.getFormatReader().lengthInSamples);

                    auto& region = context->playbackRegion.getPlaybackRegion();
                    playbackRenderer.add (region);
                    editorRenderer.add (region);
                }

                sendChangeMessage();
            }

            if (resetContext)
            {
                {
                    ScopedPluginDeactivator deactivator (instance);

                    context.reset();
                    audioSourceLength.store (0);
                }

                sendChangeMessage();
            }
        }

        struct Context
        {
            Context (ARA::Host::DocumentController& dc, const File& audioFileIn)
                : audioFile (audioFileIn),
                  musicalContext    (dc),
                  regionSequence    (dc, musicalContext, "track 1"),
                  fileAudioSource   (dc, audioFile),
                  audioModification (dc, fileAudioSource),
                  playbackRegion    (dc, regionSequence, audioModification, fileAudioSource)
            {
            }

            static std::unique_ptr<Context> createFromStateInformation (ARA::Host::DocumentController& dc, const void* d, int s)
            {
                if (auto xml = getXmlFromBinary (d, s))
                {
                    if (xml->hasTagName (xmlRootTag))
                    {
                        File file { xml->getStringAttribute (xmlAudioFileAttrib) };

                        if (file.existsAsFile())
                            return std::make_unique<Context> (dc, std::move (file));
                    }
                }

                return {};
            }

            void getStateInformation (juce::MemoryBlock& b)
            {
                XmlElement root { xmlRootTag };
                root.setAttribute (xmlAudioFileAttrib, audioFile.getFullPathName());
                copyXmlToBinary (root, b);
            }

            const static Identifier xmlRootTag;
            const static Identifier xmlAudioFileAttrib;

            File audioFile;

            MusicalContext musicalContext;
            RegionSequence regionSequence;
            FileAudioSource fileAudioSource;
            AudioModification audioModification;
            PlaybackRegion playbackRegion;
        };

        SimplePlayHead playHead;
        ARAPluginInstanceWrapper& instance;

        std::unique_ptr<ARAHostDocumentController> documentController;
        ARAHostModel::PlaybackRendererInterface playbackRenderer;
        ARAHostModel::EditorRendererInterface editorRenderer;

        std::unique_ptr<Context> context;

        mutable std::mutex contextUpdateSourceMutex;
        ContextUpdateSource contextUpdateSource;

        std::atomic<bool> isPlaying { false };
        std::atomic<bool> goToStartSignal { false };
        std::atomic<int64> audioSourceLength { 0 };
    };

    explicit ARAPluginInstanceWrapper (std::unique_ptr<AudioPluginInstance> innerIn)
        : inner (std::move (innerIn)), araHost (*this)
    {
        jassert (inner != nullptr);

        for (auto isInput : { true, false })
            matchBuses (isInput);

        setBusesLayout (inner->getBusesLayout());
    }

    //==============================================================================
    AudioProcessorEditor* createARAHostEditor() { return araHost.createEditor(); }

    //==============================================================================
    const String getName() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getName();
    }

    StringArray getAlternateDisplayNames() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getAlternateDisplayNames();
    }

    double getTailLengthSeconds() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getTailLengthSeconds();
    }

    bool acceptsMidi() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->acceptsMidi();
    }

    bool producesMidi() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->producesMidi();
    }

    AudioProcessorEditor* createEditor() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->createEditorIfNeeded();
    }

    bool hasEditor() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->hasEditor();
    }

    int getNumPrograms() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getNumPrograms();
    }

    int getCurrentProgram() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getCurrentProgram();
    }

    void setCurrentProgram (int i) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->setCurrentProgram (i);
    }

    const String getProgramName (int i) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->getProgramName (i);
    }

    void changeProgramName (int i, const String& n) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->changeProgramName (i, n);
    }

    void getStateInformation (juce::MemoryBlock& b) override
    {
        XmlElement state ("ARAPluginInstanceWrapperState");

        {
            MemoryBlock m;
            araHost.getStateInformation (m);
            state.createNewChildElement ("host")->addTextElement (m.toBase64Encoding());
        }

        {
            std::lock_guard<std::mutex> lock (innerMutex);

            MemoryBlock m;
            inner->getStateInformation (m);
            state.createNewChildElement ("plugin")->addTextElement (m.toBase64Encoding());
        }

        copyXmlToBinary (state, b);
    }

    void setStateInformation (const void* d, int s) override
    {
        if (auto xml = getXmlFromBinary (d, s))
        {
            if (xml->hasTagName ("ARAPluginInstanceWrapperState"))
            {
                if (auto* hostState = xml->getChildByName ("host"))
                {
                    MemoryBlock m;
                    m.fromBase64Encoding (hostState->getAllSubText());
                    jassert (m.getSize() <= std::numeric_limits<int>::max());
                    araHost.setStateInformation (m.getData(), (int) m.getSize());
                }

                if (auto* pluginState = xml->getChildByName ("plugin"))
                {
                    std::lock_guard<std::mutex> lock (innerMutex);

                    MemoryBlock m;
                    m.fromBase64Encoding (pluginState->getAllSubText());
                    jassert (m.getSize() <= std::numeric_limits<int>::max());
                    inner->setStateInformation (m.getData(), (int) m.getSize());
                }
            }
        }
    }

    void getCurrentProgramStateInformation (juce::MemoryBlock& b) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->getCurrentProgramStateInformation (b);
    }

    void setCurrentProgramStateInformation (const void* d, int s) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->setCurrentProgramStateInformation (d, s);
    }

    void prepareToPlay (double sr, int bs) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->setRateAndBufferSizeDetails (sr, bs);
        inner->prepareToPlay (sr, bs);
        prepareToPlayParams = { sr, bs };
    }

    void releaseResources() override { inner->releaseResources(); }

    void memoryWarningReceived() override { inner->memoryWarningReceived(); }

    void processBlock (AudioBuffer<float>& a, MidiBuffer& m) override
    {
        const SpinLock::ScopedTryLockType scope (innerProcessBlockFlag);

        if (! scope.isLocked())
            return;

        inner->processBlock (a, m);
        araHost.afterProcessBlock (a.getNumSamples());
    }

    void processBlock (AudioBuffer<double>& a, MidiBuffer& m) override
    {
        const SpinLock::ScopedTryLockType scope (innerProcessBlockFlag);

        if (! scope.isLocked())
            return;

        inner->processBlock (a, m);
        araHost.afterProcessBlock (a.getNumSamples());
    }

    void processBlockBypassed (AudioBuffer<float>& a, MidiBuffer& m) override
    {
        const SpinLock::ScopedTryLockType scope (innerProcessBlockFlag);

        if (! scope.isLocked())
            return;

        inner->processBlockBypassed (a, m);
        araHost.afterProcessBlock (a.getNumSamples());
    }

    void processBlockBypassed (AudioBuffer<double>& a, MidiBuffer& m) override
    {
        const SpinLock::ScopedTryLockType scope (innerProcessBlockFlag);

        if (! scope.isLocked())
            return;

        inner->processBlockBypassed (a, m);
        araHost.afterProcessBlock (a.getNumSamples());
    }

    bool supportsDoublePrecisionProcessing() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->supportsDoublePrecisionProcessing();
    }

    bool supportsMPE() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->supportsMPE();
    }

    bool isMidiEffect() const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->isMidiEffect();
    }

    void reset() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->reset();
    }

    void setNonRealtime (bool b) noexcept override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->setNonRealtime (b);
    }

    void refreshParameterList() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->refreshParameterList();
    }

    void numChannelsChanged() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->numChannelsChanged();
    }

    void numBusesChanged() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->numBusesChanged();
    }

    void processorLayoutsChanged() override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->processorLayoutsChanged();
    }

    void setPlayHead (AudioPlayHead* p) override { ignoreUnused (p); }

    void updateTrackProperties (const TrackProperties& p) override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        inner->updateTrackProperties (p);
    }

    bool isBusesLayoutSupported (const BusesLayout& layout) const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return inner->checkBusesLayoutSupported (layout);
    }

    bool canAddBus (bool) const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return true;
    }
    bool canRemoveBus (bool) const override
    {
        std::lock_guard<std::mutex> lock (innerMutex);
        return true;
    }

    //==============================================================================
    void fillInPluginDescription (PluginDescription& description) const override
    {
        return inner->fillInPluginDescription (description);
    }

private:
    void matchBuses (bool isInput)
    {
        const auto inBuses = inner->getBusCount (isInput);

        while (getBusCount (isInput) < inBuses)
            addBus (isInput);

        while (inBuses < getBusCount (isInput))
            removeBus (isInput);
    }

    // Used for mutual exclusion between the audio and other threads
    SpinLock innerProcessBlockFlag;

    // Used for mutual exclusion on non-audio threads
    mutable std::mutex innerMutex;

    std::unique_ptr<AudioPluginInstance> inner;

    ARATestHost araHost;

    struct PrepareToPlayParams
    {
        PrepareToPlayParams() : isValid (false) {}

        PrepareToPlayParams (double sampleRateIn, int samplesPerBlockIn)
            : isValid (true), sampleRate (sampleRateIn), samplesPerBlock (samplesPerBlockIn)
        {
        }

        bool isValid;
        double sampleRate;
        int samplesPerBlock;
    };

    PrepareToPlayParams prepareToPlayParams;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginInstanceWrapper)
};
#endif
