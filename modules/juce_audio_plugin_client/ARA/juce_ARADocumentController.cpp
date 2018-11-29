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
                                                        // TODO JUCE_ARA add a way to update compatible archive IDs and count if needed!
                                                        JucePlugin_ARADocumentArchiveID, 0U, nullptr,
                                                        // Analyzeable content types - will be updated below
                                                        0U, nullptr,
                                                        // Playback transformation flags - will be updated below
                                                        0);

        // Update analyzeable content types
        static std::vector<ARAContentType> contentTypes;
        static ARAContentType araContentVars[]{
            kARAContentTypeNotes,
            kARAContentTypeTempoEntries,
            kARAContentTypeBarSignatures,
            kARAContentTypeSignatures,
            kARAContentTypeStaticTuning,
            kARAContentTypeDynamicTuningOffsets,
            kARAContentTypeKeySignatures,
            kARAContentTypeSheetChords
        };
        for (size_t i = 0; i < sizeof (araContentVars) / sizeof (ARAContentType); i++)
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
        for (size_t i = 0; i < sizeof (araPlaybackTransformations) / sizeof (ARAPlaybackTransformationFlags); i++)
        {
            if (JucePlugin_ARATransformationFlags & (1 << i))
                factory->supportedPlaybackTransformationFlags |= araPlaybackTransformations[i];
        }

        // TODO JUCE_ARA
        // Any other factory fields? Algorithm selection?
    }

    return factory;
}

namespace juce
{
//==============================================================================

void ARADocumentController::notifyAudioSourceContentChanged (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags, bool notifyAllAudioModificationsAndPlaybackRegions)
{
    audioSourceUpdates[audioSource] += scopeFlags;

    audioSource->notifyListeners ([audioSource, scopeFlags] (ARAAudioSource::Listener& l) { l.didUpdateAudioSourceContent (audioSource, scopeFlags); });

    if (notifyAllAudioModificationsAndPlaybackRegions)
    {
        for (auto audioModification : audioSource->getAudioModifications())
            notifyAudioModificationContentChanged (static_cast<ARAAudioModification*> (audioModification), scopeFlags, true);
    }
}

void ARADocumentController::notifyAudioModificationContentChanged (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags, bool notifyAllPlaybackRegions)
{
    audioModificationUpdates[audioModification] += scopeFlags;

    audioModification->notifyListeners ([audioModification, scopeFlags] (ARAAudioModification::Listener& l) { l.didUpdateAudioModificationContent (audioModification, scopeFlags); });

    if (notifyAllPlaybackRegions)
    {
        for (auto playbackRegion : audioModification->getPlaybackRegions())
            notifyPlaybackRegionContentChanged (static_cast<ARAPlaybackRegion*> (playbackRegion), scopeFlags);
    }
}

void ARADocumentController::notifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags)
{
    playbackRegionUpdates[playbackRegion] += scopeFlags;

    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araPlaybackRegion->notifyListeners ([araPlaybackRegion, scopeFlags] (ARAPlaybackRegion::Listener& l) { l.didUpdatePlaybackRegionContent (araPlaybackRegion, scopeFlags); });
}

//==============================================================================

ARA::PlugIn::Document* ARADocumentController::doCreateDocument (ARA::PlugIn::DocumentController* documentController) noexcept
{
    return new ARADocument (static_cast<ARADocumentController*> (documentController));
}

void ARADocumentController::doBeginEditing() noexcept
{
    auto document = static_cast<ARADocument*> (getDocument());
    document->notifyListeners ([document] (ARADocument::Listener& l) { l.doBeginEditing (document); });
}

void ARADocumentController::doEndEditing() noexcept
{
    auto document = static_cast<ARADocument*> (getDocument ());
    document->notifyListeners ([document] (ARADocument::Listener& l) { l.doEndEditing (document); });
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

void ARADocumentController::willUpdateDocumentProperties (ARA::PlugIn::Document* document, ARA::PlugIn::Document::PropertiesPtr newProperties) noexcept
{
    auto araDocument = static_cast<ARADocument*> (document);
    araDocument->notifyListeners ([araDocument, newProperties] (ARADocument::Listener& l) { l.willUpdateDocumentProperties (araDocument, newProperties); });
}

void ARADocumentController::didUpdateDocumentProperties (ARA::PlugIn::Document* document) noexcept
{
    auto araDocument = static_cast<ARADocument*> (document);
    araDocument->notifyListeners ([araDocument] (ARADocument::Listener& l) { l.didUpdateDocumentProperties (araDocument); });
}

void ARADocumentController::didReorderRegionSequencesInDocument (ARA::PlugIn::Document* document) noexcept
{
    auto araDocument = static_cast<ARADocument*> (document);
    araDocument->notifyListeners ([araDocument] (ARADocument::Listener& l) { l.didReorderRegionSequencesInDocument (araDocument); });
}

void ARADocumentController::willDestroyDocument (ARA::PlugIn::Document* document) noexcept
{
    auto araDocument = static_cast<ARADocument*> (document);
    araDocument->notifyListeners ([araDocument] (ARADocument::Listener& l) { l.willDestroyDocument (araDocument); });
}

//==============================================================================

ARA::PlugIn::MusicalContext* ARADocumentController::doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept
{
    auto araDocument = static_cast<ARADocument*> (document);
    auto musicalContext = new ARAMusicalContext (araDocument, hostRef);
    araDocument->notifyListeners ([araDocument, musicalContext] (ARADocument::Listener& l) { l.didAddMusicalContext (araDocument, musicalContext); });
    return musicalContext;
}

void ARADocumentController::willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept
{
    auto araMusicalContext = static_cast<ARAMusicalContext*> (musicalContext);
    araMusicalContext->notifyListeners ([araMusicalContext, newProperties] (ARAMusicalContext::Listener& l) { l.willUpdateMusicalContextProperties (araMusicalContext, newProperties); });
}

void ARADocumentController::didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    auto araMusicalContext = static_cast<ARAMusicalContext*> (musicalContext);
    araMusicalContext->notifyListeners ([araMusicalContext] (ARAMusicalContext::Listener& l) { l.didUpdateMusicalContextProperties (araMusicalContext); });
}

void ARADocumentController::doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes scopeFlags) noexcept
{
    auto araMusicalContext = static_cast<ARAMusicalContext*> (musicalContext);
    araMusicalContext->notifyListeners ([araMusicalContext, scopeFlags] (ARAMusicalContext::Listener& l) { l.didUpdateMusicalContextContent (araMusicalContext, scopeFlags); });
}

void ARADocumentController::willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    auto araMusicalContext = static_cast<ARAMusicalContext*> (musicalContext);
    araMusicalContext->notifyListeners ([araMusicalContext] (ARAMusicalContext::Listener& l) { l.willDestroyMusicalContext (araMusicalContext); });
}

//==============================================================================

ARA::PlugIn::RegionSequence* ARADocumentController::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    auto araDocument = static_cast<ARADocument*> (document);
    auto regionSequence = new ARARegionSequence (araDocument, hostRef);
    araDocument->notifyListeners ([araDocument, regionSequence] (ARADocument::Listener& l) { l.didAddRegionSequence (araDocument, regionSequence); });
    return regionSequence;
}

void ARADocumentController::willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept
{
    auto araRegionSequence = static_cast<ARARegionSequence*> (regionSequence);
    araRegionSequence->notifyListeners ([araRegionSequence, newProperties] (ARARegionSequence::Listener& l) { l.willUpdateRegionSequenceProperties (araRegionSequence, newProperties); });
}

void ARADocumentController::didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    auto araRegionSequence = static_cast<ARARegionSequence*> (regionSequence);
    araRegionSequence->notifyListeners ([araRegionSequence] (ARARegionSequence::Listener& l) { l.didUpdateRegionSequenceProperties (araRegionSequence); });
}

void ARADocumentController::willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    auto araRegionSequence = static_cast<ARARegionSequence*> (regionSequence);
    araRegionSequence->notifyListeners ([araRegionSequence] (ARARegionSequence::Listener& l) { l.willDestroyRegionSequence (araRegionSequence); });
}

void ARADocumentController::willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    auto araRegionSequence = static_cast<ARARegionSequence*> (regionSequence);
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araRegionSequence->notifyListeners ([araRegionSequence, araPlaybackRegion] (ARARegionSequence::Listener& l) { l.willRemovePlaybackRegionFromRegionSequence (araRegionSequence, araPlaybackRegion); });
}

void ARADocumentController::didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    auto araRegionSequence = static_cast<ARARegionSequence*> (regionSequence);
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araRegionSequence->notifyListeners ([araRegionSequence, araPlaybackRegion] (ARARegionSequence::Listener& l) { l.didAddPlaybackRegionToRegionSequence (araRegionSequence, araPlaybackRegion); });
}

//==============================================================================

ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource (ARA::PlugIn::Document *document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    auto araDocument = static_cast<ARADocument*> (document);
    auto audioSource = new ARAAudioSource (araDocument, hostRef);
    araDocument->notifyListeners ([araDocument, audioSource] (ARADocument::Listener& l) { l.didAddAudioSource (araDocument, audioSource); });
    return audioSource;
}

void ARADocumentController::willUpdateAudioSourceProperties (
    ARA::PlugIn::AudioSource* audioSource,
    ARA::PlugIn::AudioSource::PropertiesPtr newProperties) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*> (audioSource);
    araAudioSource->notifyListeners ([araAudioSource, newProperties] (ARAAudioSource::Listener& l) { l.willUpdateAudioSourceProperties(araAudioSource, newProperties); });
}

void ARADocumentController::didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*> (audioSource);
    araAudioSource->notifyListeners ([araAudioSource] (ARAAudioSource::Listener& l) { l.didUpdateAudioSourceProperties (araAudioSource); });
}

void ARADocumentController::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes scopeFlags) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*> (audioSource);
    araAudioSource->notifyListeners ([araAudioSource, scopeFlags] (ARAAudioSource::Listener& l) { l.didUpdateAudioSourceContent (araAudioSource, scopeFlags); });
}

void ARADocumentController::willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*> (audioSource);
    araAudioSource->notifyListeners ([araAudioSource, enable] (ARAAudioSource::Listener& l) { l.willEnableAudioSourceSamplesAccess(araAudioSource, enable); });
}

void ARADocumentController::didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*> (audioSource);
    araAudioSource->notifyListeners ([araAudioSource, enable] (ARAAudioSource::Listener& l) { l.didEnableAudioSourceSamplesAccess(araAudioSource, enable); });
}

void ARADocumentController::doDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*> (audioSource);
    araAudioSource->notifyListeners ([araAudioSource, deactivate] (ARAAudioSource::Listener& l) { l.doDeactivateAudioSourceForUndoHistory (araAudioSource, deactivate); });
}

void ARADocumentController::willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*> (audioSource);
    araAudioSource->notifyListeners ([araAudioSource] (ARAAudioSource::Listener& l) { l.willDestroyAudioSource (araAudioSource); });
}

AudioFormatReader* ARADocumentController::createAudioSourceReader (ARAAudioSource* audioSource)
{
    return new ARAAudioSourceReader (audioSource);
}

ARAPlaybackRegionReader* ARADocumentController::createPlaybackRegionReader (std::vector<ARAPlaybackRegion*> playbackRegions, bool nonRealtime)
{
    return new ARAPlaybackRegionReader (static_cast<ARAPlaybackRenderer*> (doCreatePlaybackRenderer()), playbackRegions, nonRealtime);
}

ARARegionSequenceReader* ARADocumentController::createRegionSequenceReader (ARARegionSequence* regionSequence, bool nonRealtime)
{
    return new ARARegionSequenceReader (static_cast<ARAPlaybackRenderer*> (doCreatePlaybackRenderer()), regionSequence, nonRealtime);
}

//==============================================================================

ARA::PlugIn::AudioModification* ARADocumentController::doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef) noexcept
{
    auto araAudioSource = static_cast<ARAAudioSource*>(audioSource);
    auto audioModification = new ARAAudioModification (araAudioSource, hostRef);
    araAudioSource->notifyListeners ([araAudioSource, audioModification] (ARAAudioSource::Listener& l) { l.didAddAudioModification (araAudioSource, audioModification); });
    return audioModification;
}

void ARADocumentController::willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::AudioModification::PropertiesPtr newProperties) noexcept
{
    auto araAudioModification = static_cast<ARAAudioModification*> (audioModification);
    araAudioModification->notifyListeners ([araAudioModification, newProperties] (ARAAudioModification::Listener& l) { l.willUpdateAudioModificationProperties (araAudioModification, newProperties); });
}

void ARADocumentController::didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept
{
    auto araAudioModification = static_cast<ARAAudioModification*> (audioModification);
    araAudioModification->notifyListeners ([araAudioModification] (ARAAudioModification::Listener& l) { l.didUpdateAudioModificationProperties (araAudioModification); });
}

void ARADocumentController::doDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept
{
    auto araAudioModification = static_cast<ARAAudioModification*> (audioModification);
    araAudioModification->notifyListeners ([araAudioModification, deactivate] (ARAAudioModification::Listener& l) { l.doDeactivateAudioModificationForUndoHistory (araAudioModification, deactivate); });
}

void ARADocumentController::willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept
{
    auto araAudioModification = static_cast<ARAAudioModification*> (audioModification);
    araAudioModification->notifyListeners ([araAudioModification] (ARAAudioModification::Listener& l) { l.willDestroyAudioModification (araAudioModification); });
}

//==============================================================================

ARA::PlugIn::PlaybackRegion* ARADocumentController::doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept
{
    auto audioModification = static_cast<ARAAudioModification*>(modification);
    auto playbackRegion = new ARAPlaybackRegion (audioModification, hostRef);
    audioModification->notifyListeners ([audioModification, playbackRegion] (ARAAudioModification::Listener& l) { l.didAddPlaybackRegion (audioModification, playbackRegion); });
    return playbackRegion;
}

void ARADocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PlaybackRegion::PropertiesPtr newProperties) noexcept
{
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araPlaybackRegion->notifyListeners ([araPlaybackRegion, newProperties] (ARAPlaybackRegion::Listener& l) { l.willUpdatePlaybackRegionProperties (araPlaybackRegion, newProperties); });
}

void ARADocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araPlaybackRegion->notifyListeners ([araPlaybackRegion] (ARAPlaybackRegion::Listener& l) { l.didUpdatePlaybackRegionProperties (araPlaybackRegion); });
}

void ARADocumentController::doGetPlaybackRegionHeadAndTailTime (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::ARATimeDuration* headTime, ARA::ARATimeDuration* tailTime) noexcept
{
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    *headTime = araPlaybackRegion->getHeadTime();
    *tailTime = araPlaybackRegion->getTailTime();
}

void ARADocumentController::willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    araPlaybackRegion->notifyListeners ([araPlaybackRegion] (ARAPlaybackRegion::Listener& l) { l.willDestroyPlaybackRegion (araPlaybackRegion); });
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

} // namespace juce
