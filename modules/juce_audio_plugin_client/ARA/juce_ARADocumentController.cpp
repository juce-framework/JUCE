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

#define notify_listeners(function, ModelObjectPtrType, modelObject,  ...) \
    static_cast<ModelObjectPtrType> (modelObject)->notifyListeners ([&] (std::remove_pointer<ModelObjectPtrType>::type::Listener& l) { l.function (static_cast<ModelObjectPtrType> (modelObject), ##__VA_ARGS__); })

//==============================================================================

void ARADocumentController::notifyAudioSourceContentChanged (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags, bool notifyAllAudioModificationsAndPlaybackRegions)
{
    audioSourceUpdates[audioSource] += scopeFlags;

    notify_listeners (doUpdateAudioSourceContent, ARAAudioSource*, audioSource, scopeFlags);

    if (notifyAllAudioModificationsAndPlaybackRegions)
    {
        for (auto audioModification : audioSource->getAudioModifications<ARAAudioModification>())
            notifyAudioModificationContentChanged (audioModification, scopeFlags, true);
    }
}

void ARADocumentController::notifyAudioModificationContentChanged (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags, bool notifyAllPlaybackRegions)
{
    audioModificationUpdates[audioModification] += scopeFlags;

    notify_listeners (doUpdateAudioModificationContent, ARAAudioModification*, audioModification, scopeFlags);

    if (notifyAllPlaybackRegions)
    {
        for (auto playbackRegion : audioModification->getPlaybackRegions<ARAPlaybackRegion>())
            notifyPlaybackRegionContentChanged (playbackRegion, scopeFlags);
    }
}

void ARADocumentController::notifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags)
{
    playbackRegionUpdates[playbackRegion] += scopeFlags;

    notify_listeners (didUpdatePlaybackRegionContent, ARAPlaybackRegion*, playbackRegion, scopeFlags);
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

void ARADocumentController::doNotifyModelUpdates() noexcept
{
    auto modelUpdateController = getHostInstance()->getModelUpdateController();
    if (modelUpdateController != nullptr)
    {
        for (auto& audioSourceUpdate : audioSourceUpdates)
            modelUpdateController->notifyAudioSourceContentChanged (audioSourceUpdate.first->getHostRef(), nullptr, audioSourceUpdate.second);

        for (auto& audioModificationUpdate : audioModificationUpdates)
            modelUpdateController->notifyAudioModificationContentChanged (audioModificationUpdate.first->getHostRef(), nullptr, audioModificationUpdate.second);

        for (auto& playbackRegionUpdate : playbackRegionUpdates)
            modelUpdateController->notifyPlaybackRegionContentChanged (playbackRegionUpdate.first->getHostRef(), nullptr, playbackRegionUpdate.second);
    }

    audioSourceUpdates.clear();
    audioModificationUpdates.clear();
    playbackRegionUpdates.clear();
}

//==============================================================================

bool ARADocumentController::doRestoreObjectsFromStream (InputStream& /*input*/, ARA::PlugIn::RestoreObjectsFilter* /*filter*/) noexcept
{
    return true;
}

bool ARADocumentController::doStoreObjectsToStream (OutputStream& /*output*/, ARA::PlugIn::StoreObjectsFilter* /*filter*/) noexcept
{
    return true;
}

bool ARADocumentController::doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, ARA::PlugIn::RestoreObjectsFilter* filter) noexcept
{
    ArchiveReader reader (archiveReader);
    return doRestoreObjectsFromStream (reader, filter);
}

bool ARADocumentController::doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, ARA::PlugIn::StoreObjectsFilter* filter) noexcept
{
    ArchiveWriter writer (archiveWriter);
    return doStoreObjectsToStream(writer, filter);
}

//==============================================================================

ARA::PlugIn::MusicalContext* ARADocumentController::doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept
{
    return new ARAMusicalContext (static_cast<ARADocument*>(document), hostRef);
}

//==============================================================================

ARA::PlugIn::RegionSequence* ARADocumentController::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return new ARARegionSequence (static_cast<ARADocument*>(document), hostRef);
}

//==============================================================================

ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource (ARA::PlugIn::Document *document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    return new ARAAudioSource (static_cast<ARADocument*>(document), hostRef);
}

//==============================================================================

ARA::PlugIn::AudioModification* ARADocumentController::doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
{
    return new ARAAudioModification (static_cast<ARAAudioSource*> (audioSource), hostRef, static_cast<ARAAudioModification*> (optionalModificationToClone));
}

//==============================================================================

ARA::PlugIn::PlaybackRegion* ARADocumentController::doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept
{
    return new ARAPlaybackRegion (static_cast<ARAAudioModification*>(modification), hostRef);
}

void ARADocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) noexcept 
{
    // if any playback region changes would affect the sample content, prepare to
    // post a sample content update to any of our playback region listeners
    _playbackRegionSamplesAffected =
        ((playbackRegion->getStartInAudioModificationTime () != newProperties->startInModificationTime) ||
        (playbackRegion->getDurationInAudioModificationTime () != newProperties->durationInModificationTime) ||
        (playbackRegion->getStartInPlaybackTime () != newProperties->startInPlaybackTime) ||
        (playbackRegion->getDurationInPlaybackTime () != newProperties->durationInPlaybackTime)||
        (playbackRegion->isTimestretchEnabled() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretch) != 0)) ||
        (playbackRegion->isTimeStretchReflectingTempo() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationTimestretchReflectingTempo) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtHead() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtHead) != 0)) ||
        (playbackRegion->hasContentBasedFadeAtTail() != ((newProperties->transformationFlags & ARA::kARAPlaybackTransformationContentBasedFadeAtTail) != 0)));

    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araPlaybackRegion->notifyListeners ([araPlaybackRegion, newProperties](ARAPlaybackRegion::Listener& l) { l.willUpdatePlaybackRegionProperties (araPlaybackRegion, newProperties); });
}

void ARADocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept 
{
    // post a content update if the updated properties affect the playback region sample content
    if (_playbackRegionSamplesAffected)
    {
        notify_listeners (didUpdatePlaybackRegionContent, ARAPlaybackRegion*, playbackRegion, ARAContentUpdateScopes::samplesAreAffected());
        _playbackRegionSamplesAffected = false;
    }

    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araPlaybackRegion->notifyListeners ([araPlaybackRegion](ARAPlaybackRegion::Listener& l) { l.didUpdatePlaybackRegionProperties (araPlaybackRegion); });
}

void ARADocumentController::doGetPlaybackRegionHeadAndTailTime (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::ARATimeDuration* headTime, ARA::ARATimeDuration* tailTime) noexcept
{
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    *headTime = araPlaybackRegion->getHeadTime();
    *tailTime = araPlaybackRegion->getTailTime();
}

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

ARADocumentController::ArchiveReader::ArchiveReader (ARA::PlugIn::HostArchiveReader* reader)
: archiveReader (reader), 
  position (0), 
  size (reader->getArchiveSize())
{}

int ARADocumentController::ArchiveReader::read (void* destBuffer, int maxBytesToRead)
{
    const int bytesToRead = std::min (maxBytesToRead, (int) (size - position));
    const int result =
        archiveReader->readBytesFromArchive (
            position,
            bytesToRead,
            (ARA::ARAByte*) destBuffer)
        ? bytesToRead
        : 0;
    position += result;
    return result;
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
: archiveWriter (writer), 
  position (0)
{}

bool ARADocumentController::ArchiveWriter::write (const void* dataToWrite, size_t numberOfBytes)
{
    if (! archiveWriter->writeBytesToArchive (
        position,
        numberOfBytes,
        (const ARA::ARAByte*) dataToWrite))
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

//==============================================================================

#undef notify_listeners

} // namespace juce
