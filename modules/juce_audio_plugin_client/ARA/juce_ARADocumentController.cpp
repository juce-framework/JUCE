#include "juce_ARADocumentController.h"

const ARA::ARAFactory* ARA::PlugIn::DocumentController::getARAFactory() noexcept
{
    using namespace ARA;

    static ARAFactory* factory = nullptr;
    if (factory == nullptr)
    {
        factory = new SizedStruct<ARA_MEMBER_PTR_ARGS (ARAFactory, supportedPlaybackTransformationFlags)>(
                                                        // Supported API generations
                                                        kARAAPIGeneration_2_0_Draft, kARAAPIGeneration_2_0_Final,
                                                        // Factory ID
                                                        JucePlugin_ARAFactoryID,
                                                        // ARA lifetime management functions
                                                        ARAInitialize, ARAUninitialize,
                                                        // Strings for user dialogs
                                                        JucePlugin_Name, JucePlugin_Manufacturer,
                                                        JucePlugin_ManufacturerWebsite, JucePlugin_VersionString,
                                                        // DocumentController factory function
                                                        ARACreateDocumentControllerWithDocumentInstance,
                                                        // Document archive IDs
                                                        JucePlugin_ARADocumentArchiveID,
                                                        // Legacy document archive IDs - will be updated below
                                                        0U, nullptr,
                                                        // Analyzeable content types - will be updated below
                                                        0U, nullptr,
                                                        // Playback transformation flags - will be updated below
                                                        0);

        // Parse any legacy document archive IDs
        String legacyDocumentArchiveIDString = JucePlugin_ARACompatibleArchiveIDs;
        if (legacyDocumentArchiveIDString.isNotEmpty())
        {
            static StringArray legacyDocumentArchiveIDStrings = StringArray::fromLines (legacyDocumentArchiveIDString);
            static std::vector<ARAPersistentID> legacyDocumentArchiveIDs;
            for (auto& legacyID : legacyDocumentArchiveIDStrings)
                legacyDocumentArchiveIDs.push_back (legacyID.toRawUTF8());

            factory->compatibleDocumentArchiveIDs = legacyDocumentArchiveIDs.data();
            factory->compatibleDocumentArchiveIDsCount = legacyDocumentArchiveIDs.size();
        }

        // Update analyzeable content types
        static std::vector<ARAContentType> contentTypes;
        static ARAContentType araContentVars[]{
            kARAContentTypeNotes,
            kARAContentTypeTempoEntries,
            kARAContentTypeBarSignatures,
            kARAContentTypeStaticTuning,
            kARAContentTypeDynamicTuningOffsets,
            kARAContentTypeKeySignatures,
            kARAContentTypeSheetChords
        };
        for (size_t i = 0; i < sizeof (araContentVars) / sizeof (ARAContentType); ++i)
        {
            if (JucePlugin_ARAContentTypes & (1 << i))
                contentTypes.push_back (araContentVars[i]);
        }

        factory->analyzeableContentTypesCount = (ARASize) contentTypes.size();
        factory->analyzeableContentTypes = contentTypes.data();

        // Update playback transformation flags
        static ARAPlaybackTransformationFlags araPlaybackTransformations[]{
            kARAPlaybackTransformationTimestretch,
            kARAPlaybackTransformationTimestretchReflectingTempo,
            kARAPlaybackTransformationContentBasedFadeAtTail,
            kARAPlaybackTransformationContentBasedFadeAtHead
        };

        factory->supportedPlaybackTransformationFlags = 0;
        for (size_t i = 0; i < sizeof (araPlaybackTransformations) / sizeof (ARAPlaybackTransformationFlags); ++i)
        {
            if (JucePlugin_ARATransformationFlags & (1 << i))
                factory->supportedPlaybackTransformationFlags |= araPlaybackTransformations[i];
        }
    }

    return factory;
}

namespace juce
{

//==============================================================================

ARADocumentController::ARADocumentController (const ARA::ARADocumentControllerHostInstance* instance)
  : DocumentController (instance)
{
    startTimerHz (20);  // TODO JUCE_ARA we could start the timer on demand when the first audio source is created or activated, stop when last is deleted or deactivated.
}

//==============================================================================

void ARADocumentController::notifyAudioSourceContentChanged (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags)
{
    jassert (scopeFlags.affectEverything() || !scopeFlags.affectSamples());

    audioSourceUpdates[audioSource] += scopeFlags;
}

void ARADocumentController::notifyAudioModificationContentChanged (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags)
{
    audioModificationUpdates[audioModification] += scopeFlags;
}

void ARADocumentController::notifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags)
{
    playbackRegionUpdates[playbackRegion] += scopeFlags;
}

//==============================================================================

void ARADocumentController::notifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressStarted, 0.0f))
        internalAnalysisProgressIsSynced.clear (std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressStarted (audioSource);
}

void ARADocumentController::notifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource, float progress)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressUpdated,  progress))
        internalAnalysisProgressIsSynced.clear (std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressUpdated (audioSource, progress);
}

void ARADocumentController::notifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressCompleted, 1.0f))
        internalAnalysisProgressIsSynced.clear (std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressCompleted (audioSource);
}

//==============================================================================

// some helper macros to ease repeated declaration & implementation of notification functions below:

#define notify_listeners(function, ModelObjectPtrType, modelObject,  ...) \
    static_cast<ModelObjectPtrType> (modelObject)->notifyListeners ([&] (std::remove_pointer<ModelObjectPtrType>::type::Listener& l) { l.function (static_cast<ModelObjectPtrType> (modelObject), ##__VA_ARGS__); })

// no notification arguments
#define OVERRIDE_TO_NOTIFY_1(function, ModelObjectPtrType, modelObject) \
    void ARADocumentController::function (ARA::PlugIn::ModelObjectPtrType modelObject) noexcept \
    { \
        notify_listeners (function, ARA##ModelObjectPtrType, modelObject); \
    }

// single notification argument, model object version
#define OVERRIDE_TO_NOTIFY_2(function, ModelObjectPtrType, modelObject, ArgumentType, argument) \
    void ARADocumentController::function (ARA::PlugIn::ModelObjectPtrType modelObject, ARA::PlugIn::ArgumentType argument) noexcept \
    { \
        notify_listeners (function, ARA##ModelObjectPtrType, modelObject, static_cast<ARA##ArgumentType> (argument)); \
    }

// single notification argument, non-model object version
#define OVERRIDE_TO_NOTIFY_3(function, ModelObjectPtrType, modelObject, ArgumentType, argument) \
    void ARADocumentController::function (ARA::PlugIn::ModelObjectPtrType modelObject, ArgumentType argument) noexcept \
    { \
        notify_listeners (function, ARA##ModelObjectPtrType, modelObject, argument); \
    }

//==============================================================================

ARA::PlugIn::Document* ARADocumentController::doCreateDocument (ARA::PlugIn::DocumentController* documentController) noexcept
{
    return new ARADocument (static_cast<ARADocumentController*> (documentController));
}

void ARADocumentController::willBeginEditing() noexcept
{
    notify_listeners (willBeginEditing, ARADocument*, getDocument());
}

void ARADocumentController::didEndEditing() noexcept
{
    notify_listeners (didEndEditing, ARADocument*, getDocument());
}

void ARADocumentController::doNotifyModelContentUpdates() noexcept
{
    auto hostModelUpdateController = getHostModelUpdateController();
    if (hostModelUpdateController != nullptr)
    {
        for (auto& audioSourceUpdate : audioSourceUpdates)
            hostModelUpdateController->notifyAudioSourceContentChanged (audioSourceUpdate.first->getHostRef(), nullptr, audioSourceUpdate.second);

        for (auto& audioModificationUpdate : audioModificationUpdates)
            hostModelUpdateController->notifyAudioModificationContentChanged (audioModificationUpdate.first->getHostRef(), nullptr, audioModificationUpdate.second);

        for (auto& playbackRegionUpdate : playbackRegionUpdates)
            hostModelUpdateController->notifyPlaybackRegionContentChanged (playbackRegionUpdate.first->getHostRef(), nullptr, playbackRegionUpdate.second);
    }

    audioSourceUpdates.clear();
    audioModificationUpdates.clear();
    playbackRegionUpdates.clear();
}

//==============================================================================

bool ARADocumentController::doRestoreObjectsFromStream (InputStream& /*input*/, const ARA::PlugIn::RestoreObjectsFilter* /*filter*/) noexcept
{
    return true;
}

bool ARADocumentController::doStoreObjectsToStream (OutputStream& /*output*/, const ARA::PlugIn::StoreObjectsFilter* /*filter*/) noexcept
{
    return true;
}

bool ARADocumentController::doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept
{
    ArchiveReader reader (archiveReader);
    return doRestoreObjectsFromStream (reader, filter);
}

bool ARADocumentController::doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept
{
    ArchiveWriter writer (archiveWriter);
    return doStoreObjectsToStream (writer, filter);
}

//==============================================================================

OVERRIDE_TO_NOTIFY_3 (willUpdateDocumentProperties, Document*, document, ARADocument::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateDocumentProperties, Document*, document)
OVERRIDE_TO_NOTIFY_2 (didAddMusicalContextToDocument, Document*, document, MusicalContext*, musicalContext)
OVERRIDE_TO_NOTIFY_2 (willRemoveMusicalContextFromDocument, Document*, document, MusicalContext*, musicalContext)
OVERRIDE_TO_NOTIFY_1 (didReorderMusicalContextsInDocument, Document*, document)
OVERRIDE_TO_NOTIFY_2 (didAddRegionSequenceToDocument, Document*, document, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_2 (willRemoveRegionSequenceFromDocument, Document*, document, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_1 (didReorderRegionSequencesInDocument, Document*, document)
OVERRIDE_TO_NOTIFY_2 (didAddAudioSourceToDocument, Document*, document, AudioSource*, audioSource)
OVERRIDE_TO_NOTIFY_2 (willRemoveAudioSourceFromDocument, Document*, document, AudioSource*, audioSource)
OVERRIDE_TO_NOTIFY_1 (willDestroyDocument, Document*, document)

//==============================================================================

ARA::PlugIn::MusicalContext* ARADocumentController::doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept
{
    return new ARAMusicalContext (static_cast<ARADocument*> (document), hostRef);
}

void ARADocumentController::doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes scopeFlags) noexcept
{
    auto araMusicalContext = static_cast<ARAMusicalContext*> (musicalContext);
    araMusicalContext->notifyListeners ([&] (ARAMusicalContext::Listener& l) { l.didUpdateMusicalContextContent(araMusicalContext, scopeFlags); });
}

OVERRIDE_TO_NOTIFY_3 (willUpdateMusicalContextProperties, MusicalContext*, musicalContext, ARAMusicalContext::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateMusicalContextProperties, MusicalContext*, musicalContext)
OVERRIDE_TO_NOTIFY_1 (willDestroyMusicalContext, MusicalContext*, musicalContext)

//==============================================================================

ARA::PlugIn::RegionSequence* ARADocumentController::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return new ARARegionSequence (static_cast<ARADocument*> (document), hostRef);
}

OVERRIDE_TO_NOTIFY_3 (willUpdateRegionSequenceProperties, RegionSequence*, regionSequence, ARARegionSequence::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateRegionSequenceProperties, RegionSequence*, regionSequence)
OVERRIDE_TO_NOTIFY_2 (didAddPlaybackRegionToRegionSequence, RegionSequence*, regionSequence, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_2 (willRemovePlaybackRegionFromRegionSequence, RegionSequence*, regionSequence, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_1 (willDestroyRegionSequence, RegionSequence*, regionSequence)

//==============================================================================

ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    return new ARAAudioSource (static_cast<ARADocument*> (document), hostRef);
}

void ARADocumentController::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* AudioSource, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes scopeFlags) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*> (AudioSource);
    araAudioSource->notifyListeners ([&] (ARAAudioSource::Listener& l) { l.didUpdateAudioSourceContent(araAudioSource, scopeFlags); });
}

OVERRIDE_TO_NOTIFY_3 (willUpdateAudioSourceProperties, AudioSource*, audioSource, ARAAudioSource::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateAudioSourceProperties, AudioSource*, audioSource)
OVERRIDE_TO_NOTIFY_3 (willEnableAudioSourceSamplesAccess, AudioSource*, audioSource, bool, enable)
OVERRIDE_TO_NOTIFY_3 (didEnableAudioSourceSamplesAccess, AudioSource*, audioSource, bool, enable)
OVERRIDE_TO_NOTIFY_2 (didAddAudioModificationToAudioSource, AudioSource*, audioSource, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_2 (willRemoveAudioModificationFromAudioSource, AudioSource*, audioSource, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_3 (willDeactivateAudioSourceForUndoHistory, AudioSource*, audioSource, bool, deactivate)
OVERRIDE_TO_NOTIFY_3 (didDeactivateAudioSourceForUndoHistory, AudioSource*, audioSource, bool, deactivate)
OVERRIDE_TO_NOTIFY_1 (willDestroyAudioSource, AudioSource*, audioSource)

//==============================================================================

ARA::PlugIn::AudioModification* ARADocumentController::doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
{
    return new ARAAudioModification (static_cast<ARAAudioSource*> (audioSource), hostRef, static_cast<const ARAAudioModification*> (optionalModificationToClone));
}

OVERRIDE_TO_NOTIFY_3 (willUpdateAudioModificationProperties, AudioModification*, audioModification, ARAAudioModification::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateAudioModificationProperties, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_2 (didAddPlaybackRegionToAudioModification, AudioModification*, audioModification, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_2 (willRemovePlaybackRegionFromAudioModification, AudioModification*, audioModification, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_3 (willDeactivateAudioModificationForUndoHistory, AudioModification*, audioModification, bool, deactivate)
OVERRIDE_TO_NOTIFY_3 (didDeactivateAudioModificationForUndoHistory, AudioModification*, audioModification, bool, deactivate)
OVERRIDE_TO_NOTIFY_1 (willDestroyAudioModification, AudioModification*, audioModification)

//==============================================================================

ARA::PlugIn::PlaybackRegion* ARADocumentController::doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept
{
    return new ARAPlaybackRegion (static_cast<ARAAudioModification*> (modification), hostRef);
}

void ARADocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept 
{
    // if any playback region changes would affect the sample content, prepare to
    // post a sample content update to any of our playback region listeners
    jassert(! currentPropertyUpdateAffectsContent);
    currentPropertyUpdateAffectsContent =
        ((playbackRegion->getStartInAudioModificationTime() != newProperties->startInModificationTime) ||
        (playbackRegion->getDurationInAudioModificationTime() != newProperties->durationInModificationTime) ||
        (playbackRegion->getStartInPlaybackTime() != newProperties->startInPlaybackTime) ||
        (playbackRegion->getDurationInPlaybackTime() != newProperties->durationInPlaybackTime)||
        (playbackRegion->isTimestretchEnabled() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretch) != 0)) ||
        (playbackRegion->isTimeStretchReflectingTempo() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretchReflectingTempo) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtHead() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtHead) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtTail() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtTail) != 0)));

    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araPlaybackRegion->notifyListeners ([araPlaybackRegion, newProperties](ARAPlaybackRegion::Listener& l) { l.willUpdatePlaybackRegionProperties (araPlaybackRegion, newProperties); });
}

void ARADocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept 
{
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araPlaybackRegion->notifyListeners ([araPlaybackRegion](ARAPlaybackRegion::Listener& l) { l.didUpdatePlaybackRegionProperties (araPlaybackRegion); });

    // post a content update if the updated properties affect the playback region sample content
    if (currentPropertyUpdateAffectsContent)
    {
        currentPropertyUpdateAffectsContent = false;
        auto scopes = ARAContentUpdateScopes::samplesAreAffected();
        JUCE_CONSTEXPR auto areNotesAnalyzable = (bool) (JucePlugin_ARAContentTypes & 1);
        if (areNotesAnalyzable)
            scopes += ARAContentUpdateScopes::notesAreAffected();
        // other content such as tempo or key signatures are not exported at playback region level
        // because this would simply mirror the musical context content.
        notifyPlaybackRegionContentChanged (araPlaybackRegion, scopes);
    }
}

void ARADocumentController::doGetPlaybackRegionHeadAndTailTime (const ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::ARATimeDuration* headTime, ARA::ARATimeDuration* tailTime) noexcept
{
    auto araPlaybackRegion = static_cast<const ARAPlaybackRegion*> (playbackRegion);
    *headTime = araPlaybackRegion->getHeadTime();
    *tailTime = araPlaybackRegion->getTailTime();
}

OVERRIDE_TO_NOTIFY_1 (willDestroyPlaybackRegion, PlaybackRegion*, playbackRegion)

//==============================================================================

#undef notify_listeners
#undef OVERRIDE_TO_NOTIFY_1
#undef OVERRIDE_TO_NOTIFY_2
#undef OVERRIDE_TO_NOTIFY_3

//==============================================================================

ARA::PlugIn::PlaybackRenderer* ARADocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARAPlaybackRenderer (this);
}

ARA::PlugIn::EditorRenderer* ARADocumentController::doCreateEditorRenderer() noexcept
{
    return new ARAEditorRenderer (this);
}

ARA::PlugIn::EditorView* ARADocumentController::doCreateEditorView() noexcept
{
    return new ARAEditorView (this);
}

//==============================================================================

// helper code for ARADocumentController::timerCallback() to rewire the host-related ARA SDK's progress tracker to our internal update mechanism
namespace ModelUpdateControllerProgressAdapter
{
    using namespace ARA;

    static void ARA_CALL notifyAudioSourceAnalysisProgress (ARAModelUpdateControllerHostRef controllerHostRef,
                                                            ARAAudioSourceHostRef audioSourceHostRef, ARAAnalysisProgressState state, float value) noexcept
    {
        auto audioSource = reinterpret_cast<ARAAudioSource*> (audioSourceHostRef);
        audioSource->notifyListeners ([&] (ARAAudioSource::Listener& l) { l.didUpdateAudioSourceAnalyisProgress (audioSource, state, value); });
    }

    static void ARA_CALL notifyAudioSourceContentChanged (ARAModelUpdateControllerHostRef controllerHostRef, ARAAudioSourceHostRef audioSourceHostRef,
                                                          const ARAContentTimeRange* range, ARAContentUpdateFlags flags) noexcept
    {
        jassert (false);    // not to be called - this adapter only forwards analysis progress
    }

    static void ARA_CALL notifyAudioModificationContentChanged (ARAModelUpdateControllerHostRef controllerHostRef, ARAAudioModificationHostRef audioModificationHostRef,
                                                                const ARAContentTimeRange* range, ARAContentUpdateFlags flags) noexcept
    {
        jassert (false);    // not to be called - this adapter only forwards analysis progress
    }

    static void ARA_CALL notifyPlaybackRegionContentChanged (ARAModelUpdateControllerHostRef controllerHostRef, ARAPlaybackRegionHostRef playbackRegionHostRef,
                                                             const ARAContentTimeRange* range, ARAContentUpdateFlags flags) noexcept
    {
        jassert (false);    // not to be called - this adapter only forwards analysis progress
    }

    ARA::PlugIn::HostModelUpdateController* get()
    {
        static const SizedStruct<ARA_MEMBER_PTR_ARGS (ARAModelUpdateControllerInterface, notifyPlaybackRegionContentChanged)> modelUpdateControllerInterface {
                ModelUpdateControllerProgressAdapter::notifyAudioSourceAnalysisProgress,
                ModelUpdateControllerProgressAdapter::notifyAudioSourceContentChanged,
                ModelUpdateControllerProgressAdapter::notifyAudioModificationContentChanged,
                ModelUpdateControllerProgressAdapter::notifyPlaybackRegionContentChanged
            };

        static const SizedStruct<ARA_MEMBER_PTR_ARGS (ARADocumentControllerHostInstance, playbackControllerInterface)> instance {
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &modelUpdateControllerInterface, nullptr, nullptr
        };

        static auto progressAdapter = ARA::PlugIn::HostModelUpdateController { &instance };
        return &progressAdapter;
    }
}

void ARADocumentController::timerCallback()
{
    if (! internalAnalysisProgressIsSynced.test_and_set (std::memory_order_release))
        for (auto audioSource : getDocument()->getAudioSources<ARAAudioSource>())
            audioSource->internalAnalysisProgressTracker.notifyProgress (ModelUpdateControllerProgressAdapter::get(), reinterpret_cast<ARA::ARAAudioSourceHostRef> (audioSource));
}

//==============================================================================

ARADocumentController::ArchiveReader::ArchiveReader (ARA::PlugIn::HostArchiveReader* reader)
: archiveReader (reader), 
  size (reader->getArchiveSize())
{}

int ARADocumentController::ArchiveReader::read (void* destBuffer, int maxBytesToRead)
{
    const int bytesToRead = std::min (maxBytesToRead, (int) (size - position));
    if (! archiveReader->readBytesFromArchive (position, bytesToRead, (ARA::ARAByte*) destBuffer))
        return 0;
    position += bytesToRead;
    return bytesToRead;
}

bool ARADocumentController::ArchiveReader::setPosition (int64 newPosition)
{
    if (newPosition >= (int64) size)
        return false;
    position = (size_t) newPosition;
    return true;
}

bool ARADocumentController::ArchiveReader::isExhausted()
{
    return position >= size;
}

ARADocumentController::ArchiveWriter::ArchiveWriter (ARA::PlugIn::HostArchiveWriter* writer)
{}

bool ARADocumentController::ArchiveWriter::write (const void* dataToWrite, size_t numberOfBytes)
{
    if (! archiveWriter->writeBytesToArchive (position, numberOfBytes, (const ARA::ARAByte*) dataToWrite))
        return false;
    position += numberOfBytes;
    return true;
}

bool ARADocumentController::ArchiveWriter::setPosition (int64 newPosition)
{
    if (newPosition > (int64) std::numeric_limits<size_t>::max())
        return false;
    position = (size_t) newPosition;
    return true;
}

} // namespace juce
