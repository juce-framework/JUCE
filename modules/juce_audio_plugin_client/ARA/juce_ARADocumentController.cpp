#include "juce_ARADocumentController.h"

const ARA::PlugIn::FactoryConfig* ARA::PlugIn::DocumentController::doCreateFactoryConfig() noexcept
{
    using namespace ARA;
    using namespace PlugIn;

    class JUCEARAFactoryConfig    : public FactoryConfig
    {
    public:
        JUCEARAFactoryConfig()
        {
            juce::String compatibleDocumentArchiveIDString = JucePlugin_ARACompatibleArchiveIDs;
            if (compatibleDocumentArchiveIDString.isNotEmpty())
            {
                compatibleDocumentArchiveIDStrings = juce::StringArray::fromLines (compatibleDocumentArchiveIDString);
                for (auto& compatibleID : compatibleDocumentArchiveIDStrings)
                    compatibleDocumentArchiveIDs.push_back (compatibleID.toRawUTF8());
            }
            
            // Update analyzeable content types
            static ARAContentType araContentVars[]{
                kARAContentTypeNotes,
                kARAContentTypeTempoEntries,
                kARAContentTypeBarSignatures,
                kARAContentTypeStaticTuning,
                kARAContentTypeKeySignatures,
                kARAContentTypeSheetChords
            };
            for (size_t i = 0; i < sizeof (araContentVars) / sizeof (ARAContentType); ++i)
                if (JucePlugin_ARAContentTypes & (1 << i))
                    analyzeableContentTypes.push_back (araContentVars[i]);

            // Update playback transformation flags
            const static ARAPlaybackTransformationFlags araPlaybackTransformations[]{
                kARAPlaybackTransformationTimestretch,
                kARAPlaybackTransformationTimestretchReflectingTempo,
                kARAPlaybackTransformationContentBasedFadeAtTail,
                kARAPlaybackTransformationContentBasedFadeAtHead
            };

            supportedPlaybackTransformationFlags = 0;
            for (size_t i = 0; i < sizeof (araPlaybackTransformations) / sizeof (ARAPlaybackTransformationFlags); ++i)
                if (JucePlugin_ARATransformationFlags & (1 << i))
                    supportedPlaybackTransformationFlags |= araPlaybackTransformations[i];
        }

        const char* getFactoryID() const noexcept override { return JucePlugin_ARAFactoryID; }
        const char* getPlugInName() const noexcept override { return JucePlugin_Name; }
        const char* getManufacturerName() const noexcept override { return JucePlugin_Manufacturer; }
        const char* getInformationURL() const noexcept override { return JucePlugin_ManufacturerWebsite; }
        const char* getVersion() const noexcept override { return JucePlugin_VersionString; }

        virtual const char* getDocumentArchiveID() const noexcept override { return JucePlugin_ARADocumentArchiveID; }
        virtual ARASize getCompatibleDocumentArchiveIDsCount() const noexcept override { return compatibleDocumentArchiveIDs.size(); }
        virtual const ARAPersistentID* getCompatibleDocumentArchiveIDs() const noexcept override { return compatibleDocumentArchiveIDs.empty() ? nullptr : compatibleDocumentArchiveIDs.data(); }

        virtual ARASize getAnalyzeableContentTypesCount() const noexcept override { return analyzeableContentTypes.size(); }
        virtual const ARAContentType* getAnalyzeableContentTypes() const noexcept override { return analyzeableContentTypes.empty() ? nullptr : analyzeableContentTypes.data(); }

        virtual ARAPlaybackTransformationFlags getSupportedPlaybackTransformationFlags() const noexcept override { return supportedPlaybackTransformationFlags; }

    private:
        juce::StringArray compatibleDocumentArchiveIDStrings;
        std::vector<ARAPersistentID> compatibleDocumentArchiveIDs;
        std::vector<ARAContentType> analyzeableContentTypes;
        ARAPlaybackTransformationFlags supportedPlaybackTransformationFlags;
    };

    return new JUCEARAFactoryConfig();
}

namespace juce
{

//==============================================================================

ARADocumentController::ARADocumentController (const ARA::ARADocumentControllerHostInstance* instance)
  : DocumentController (instance)
{
}

//==============================================================================

void ARADocumentController::internalNotifyAudioSourceAnalysisProgressStarted (ARAAudioSource* audioSource)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressStarted, 0.0f))
        internalAnalysisProgressIsSynced.clear (std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressStarted (audioSource);
}

void ARADocumentController::internalNotifyAudioSourceAnalysisProgressUpdated (ARAAudioSource* audioSource, float progress)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressUpdated,  progress))
        internalAnalysisProgressIsSynced.clear (std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressUpdated (audioSource, progress);
}

void ARADocumentController::internalNotifyAudioSourceAnalysisProgressCompleted (ARAAudioSource* audioSource)
{
    if (audioSource->internalAnalysisProgressTracker.updateProgress (ARA::kARAAnalysisProgressCompleted, 1.0f))
        internalAnalysisProgressIsSynced.clear (std::memory_order_release);

    DocumentController::notifyAudioSourceAnalysisProgressCompleted (audioSource);
}

void ARADocumentController::internalDidUpdateAudioSourceAnalysisProgress (ARAAudioSource* audioSource, ARAAudioSource::ARAAnalysisProgressState state, float progress)
{
    // helper to forward listener callbacks from our ModelUpdateControllerProgressAdapter
    didUpdateAudioSourceAnalyisProgress (audioSource, state, progress);
}

//==============================================================================

// some helper macros to ease repeated declaration & implementation of notification functions below:

#define notify_listeners(function, ModelObjectPtrType, modelObject,  ...) \
    static_cast<std::remove_pointer<ModelObjectPtrType>::type::Listener*> (this)->function  (static_cast<ModelObjectPtrType> (modelObject), ##__VA_ARGS__); \
    static_cast<ModelObjectPtrType> (modelObject)->notifyListeners ([&] (std::remove_pointer<ModelObjectPtrType>::type::Listener& l) { try { l.function (static_cast<ModelObjectPtrType> (modelObject), ##__VA_ARGS__); } catch (...) {} })

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

ARA::PlugIn::Document* ARADocumentController::doCreateDocument() noexcept
{
    return new ARADocument (static_cast<ARADocumentController*> (this));
}

void ARADocumentController::willBeginEditing() noexcept
{
    notify_listeners (willBeginEditing, ARADocument*, getDocument());
}

void ARADocumentController::didEndEditing() noexcept
{
    notify_listeners (didEndEditing, ARADocument*, getDocument());

    if (isTimerRunning() && (activeAudioSourcesCount == 0))
        stopTimer();
    else if (! isTimerRunning() && (activeAudioSourcesCount > 0))
        startTimerHz (20);
}

void ARADocumentController::willNotifyModelUpdates() noexcept
{
    notify_listeners (willNotifyModelUpdates, ARADocument*, getDocument());
}

void ARADocumentController::didNotifyModelUpdates() noexcept
{
    notify_listeners (didNotifyModelUpdates, ARADocument*, getDocument());
}

//==============================================================================

bool ARADocumentController::doRestoreObjectsFromArchive (ARA::PlugIn::HostArchiveReader* archiveReader, const ARA::PlugIn::RestoreObjectsFilter* filter) noexcept
{
    ARAInputStream reader (archiveReader);
    return doRestoreObjectsFromStream (reader, filter);
}

bool ARADocumentController::doStoreObjectsToArchive (ARA::PlugIn::HostArchiveWriter* archiveWriter, const ARA::PlugIn::StoreObjectsFilter* filter) noexcept
{
    ARAOutputStream writer (archiveWriter);
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

void ARADocumentController::doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes flags) noexcept
{
    notify_listeners (doUpdateMusicalContextContent, ARAMusicalContext*, musicalContext, flags);
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
    ++activeAudioSourcesCount;
    return new ARAAudioSource (static_cast<ARADocument*> (document), hostRef);
}

void ARADocumentController::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes flags) noexcept
{
    notify_listeners (doUpdateAudioSourceContent, ARAAudioSource*, audioSource, flags);
}

OVERRIDE_TO_NOTIFY_3 (willUpdateAudioSourceProperties, AudioSource*, audioSource, ARAAudioSource::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdateAudioSourceProperties, AudioSource*, audioSource)
OVERRIDE_TO_NOTIFY_3 (willEnableAudioSourceSamplesAccess, AudioSource*, audioSource, bool, enable)
OVERRIDE_TO_NOTIFY_3 (didEnableAudioSourceSamplesAccess, AudioSource*, audioSource, bool, enable)
OVERRIDE_TO_NOTIFY_2 (didAddAudioModificationToAudioSource, AudioSource*, audioSource, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_2 (willRemoveAudioModificationFromAudioSource, AudioSource*, audioSource, AudioModification*, audioModification)
OVERRIDE_TO_NOTIFY_3 (willDeactivateAudioSourceForUndoHistory, AudioSource*, audioSource, bool, deactivate)

void ARADocumentController::didDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept
{
    activeAudioSourcesCount += (deactivate ? -1 : 1);
    notify_listeners (didDeactivateAudioSourceForUndoHistory, ARAAudioSource*, audioSource, deactivate);
}

void ARADocumentController::willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    if (! audioSource->isDeactivatedForUndoHistory())
        --activeAudioSourcesCount;
    notify_listeners (willDestroyAudioSource, ARAAudioSource*, audioSource);
}
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

OVERRIDE_TO_NOTIFY_3 (willUpdatePlaybackRegionProperties, PlaybackRegion*, playbackRegion, ARAPlaybackRegion::PropertiesPtr, newProperties)
OVERRIDE_TO_NOTIFY_1 (didUpdatePlaybackRegionProperties, PlaybackRegion*, playbackRegion)
OVERRIDE_TO_NOTIFY_1 (willDestroyPlaybackRegion, PlaybackRegion*, playbackRegion)

//==============================================================================

void ARADocumentController::internalNotifyAudioSourceContentChanged (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyAudioSourceContentChanged (audioSource, scopeFlags);
    notify_listeners (doUpdateAudioSourceContent, ARAAudioSource*, audioSource, scopeFlags);
}

void ARADocumentController::internalNotifyAudioModificationContentChanged (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyAudioModificationContentChanged (audioModification, scopeFlags);
    notify_listeners (didUpdateAudioModificationContent, ARAAudioModification*, audioModification, scopeFlags);
}

void ARADocumentController::internalNotifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    if (notifyARAHost)
        DocumentController::notifyPlaybackRegionContentChanged (playbackRegion, scopeFlags);
    notify_listeners (didUpdatePlaybackRegionContent, ARAPlaybackRegion*, playbackRegion, scopeFlags);
}

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

    static void ARA_CALL notifyAudioSourceAnalysisProgress (ARAModelUpdateControllerHostRef /*controllerHostRef*/,
                                                            ARAAudioSourceHostRef audioSourceHostRef, ARAAnalysisProgressState state, float value) noexcept
    {
        auto audioSource = reinterpret_cast<ARAAudioSource*> (audioSourceHostRef);
        audioSource->getDocumentController<ARADocumentController>()->internalDidUpdateAudioSourceAnalysisProgress (audioSource, state, value);
        audioSource->notifyListeners ([&] (ARAAudioSource::Listener& l) { l.didUpdateAudioSourceAnalyisProgress (audioSource, state, value); });
    }

    static void ARA_CALL notifyAudioSourceContentChanged (ARAModelUpdateControllerHostRef, ARAAudioSourceHostRef,
                                                          const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassert (false);    // not to be called - this adapter only forwards analysis progress
    }

    static void ARA_CALL notifyAudioModificationContentChanged (ARAModelUpdateControllerHostRef, ARAAudioModificationHostRef,
                                                                const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassert (false);    // not to be called - this adapter only forwards analysis progress
    }

    static void ARA_CALL notifyPlaybackRegionContentChanged (ARAModelUpdateControllerHostRef, ARAPlaybackRegionHostRef,
                                                             const ARAContentTimeRange*, ARAContentUpdateFlags) noexcept
    {
        jassert (false);    // not to be called - this adapter only forwards analysis progress
    }

    ARA::PlugIn::HostModelUpdateController* get()
    {
        static const SizedStruct<ARA_STRUCT_MEMBER (ARAModelUpdateControllerInterface, notifyPlaybackRegionContentChanged)> modelUpdateControllerInterface {
            ModelUpdateControllerProgressAdapter::notifyAudioSourceAnalysisProgress,
            ModelUpdateControllerProgressAdapter::notifyAudioSourceContentChanged,
            ModelUpdateControllerProgressAdapter::notifyAudioModificationContentChanged,
            ModelUpdateControllerProgressAdapter::notifyPlaybackRegionContentChanged
        };

        static const SizedStruct<ARA_STRUCT_MEMBER (ARADocumentControllerHostInstance, playbackControllerInterface)> instance {
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

ARAInputStream::ARAInputStream (ARA::PlugIn::HostArchiveReader* reader)
: archiveReader (reader), 
  size (reader->getArchiveSize())
{}

int ARAInputStream::read (void* destBuffer, int maxBytesToRead)
{
    const int bytesToRead = std::min (maxBytesToRead, (int) (size - position));
    if (! archiveReader->readBytesFromArchive (position, bytesToRead, (ARA::ARAByte*) destBuffer))
    {
        failure = true;
        return 0;
    }

    position += bytesToRead;
    return bytesToRead;
}

bool ARAInputStream::setPosition (int64 newPosition)
{
    if (newPosition >= (int64) size)
        return false;
    position = (size_t) newPosition;
    return true;
}

bool ARAInputStream::isExhausted()
{
    return position >= size;
}

ARAOutputStream::ARAOutputStream (ARA::PlugIn::HostArchiveWriter* writer)
: archiveWriter (writer)
{}

bool ARAOutputStream::write (const void* dataToWrite, size_t numberOfBytes)
{
    if (!archiveWriter->writeBytesToArchive (position, numberOfBytes, (const ARA::ARAByte*) dataToWrite))
        return false;

    position += numberOfBytes;
    return true;
}

bool ARAOutputStream::setPosition (int64 newPosition)
{
    if (newPosition > (int64) std::numeric_limits<size_t>::max())
        return false;
    position = (size_t) newPosition;
    return true;
}

} // namespace juce
