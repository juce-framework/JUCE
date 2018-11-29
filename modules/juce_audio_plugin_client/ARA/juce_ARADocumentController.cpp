#include "juce_ARADocumentController.h"

#define notify_listeners(className, function, classInstance,  ...) \
    static_cast<className*> (classInstance)->notifyListeners ([&] (className::Listener& l) { l.function (static_cast<className*> (classInstance), ##__VA_ARGS__); })

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
    notify_listeners (ARAPlaybackRegion, didUpdatePlaybackRegionContent, playbackRegion, scopeFlags);
}

//==============================================================================

ARA::PlugIn::Document* ARADocumentController::doCreateDocument (ARA::PlugIn::DocumentController* documentController) noexcept
{
    return new ARADocument (static_cast<ARADocumentController*> (documentController));
}

void ARADocumentController::doBeginEditing() noexcept
{
    notify_listeners (ARADocument, doBeginEditing, getDocument());
}

void ARADocumentController::doEndEditing() noexcept
{
    notify_listeners (ARADocument, doEndEditing, getDocument());
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
    notify_listeners (ARADocument, willUpdateDocumentProperties, document, newProperties);
}

void ARADocumentController::didUpdateDocumentProperties (ARA::PlugIn::Document* document) noexcept
{
    notify_listeners (ARADocument, didUpdateDocumentProperties, document);
}

void ARADocumentController::didReorderRegionSequencesInDocument (ARA::PlugIn::Document* document) noexcept
{
    notify_listeners (ARADocument, didReorderRegionSequencesInDocument, document);
}

void ARADocumentController::didAddMusicalContextToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    notify_listeners (ARADocument, didAddMusicalContext, document, static_cast<ARAMusicalContext*> (musicalContext));
}

void ARADocumentController::willRemoveMusicalContextFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    notify_listeners (ARADocument, willRemoveMusicalContext, document, static_cast<ARAMusicalContext*> (musicalContext));
}

void ARADocumentController::didAddRegionSequenceToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    notify_listeners (ARADocument, didAddRegionSequence, document, static_cast<ARARegionSequence*> (regionSequence));
}

void ARADocumentController::willRemoveRegionSequenceFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    notify_listeners (ARADocument, willRemoveRegionSequence, document, static_cast<ARARegionSequence*> (regionSequence));
}

void ARADocumentController::didAddAudioSourceToDocument (ARA::PlugIn::Document* document, ARA::PlugIn::AudioSource* audioSource) noexcept
{
    notify_listeners (ARADocument, didAddAudioSource, document, static_cast<ARAAudioSource*> (audioSource));
}

void ARADocumentController::willRemoveAudioSourceFromDocument (ARA::PlugIn::Document* document, ARA::PlugIn::AudioSource* audioSource) noexcept
{
    notify_listeners (ARADocument, willRemoveAudioSource, document, static_cast<ARAAudioSource*> (audioSource));
}

void ARADocumentController::willDestroyDocument (ARA::PlugIn::Document* document) noexcept
{
    notify_listeners (ARADocument, willDestroyDocument, document);
}

//==============================================================================

ARA::PlugIn::MusicalContext* ARADocumentController::doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept
{
    return new ARAMusicalContext (static_cast<ARADocument*>(document), hostRef);
}

void ARADocumentController::willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept
{
    notify_listeners (ARAMusicalContext, willUpdateMusicalContextProperties, musicalContext, newProperties);
}

void ARADocumentController::didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    notify_listeners (ARAMusicalContext, didUpdateMusicalContextProperties, musicalContext);
}

void ARADocumentController::doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes scopeFlags) noexcept
{
    notify_listeners (ARAMusicalContext, didUpdateMusicalContextContent, musicalContext, scopeFlags);
}

void ARADocumentController::willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    notify_listeners (ARAMusicalContext, willDestroyMusicalContext, musicalContext);
}

//==============================================================================

ARA::PlugIn::RegionSequence* ARADocumentController::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return new ARARegionSequence (static_cast<ARADocument*>(document), hostRef);
}

void ARADocumentController::willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept
{
    notify_listeners (ARARegionSequence, willUpdateRegionSequenceProperties, regionSequence, newProperties);
}

void ARADocumentController::didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    notify_listeners (ARARegionSequence, didUpdateRegionSequenceProperties, regionSequence);
}

void ARADocumentController::willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    notify_listeners (ARARegionSequence, willDestroyRegionSequence, regionSequence);
}

void ARADocumentController::willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    notify_listeners (ARARegionSequence, willRemovePlaybackRegionFromRegionSequence, regionSequence, static_cast<ARAPlaybackRegion*> (playbackRegion));
}

void ARADocumentController::didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    notify_listeners (ARARegionSequence, didAddPlaybackRegionToRegionSequence, regionSequence, static_cast<ARAPlaybackRegion*> (playbackRegion));
}

//==============================================================================

ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource (ARA::PlugIn::Document *document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    return new ARAAudioSource (static_cast<ARADocument*>(document), hostRef);\
}

void ARADocumentController::willUpdateAudioSourceProperties (
    ARA::PlugIn::AudioSource* audioSource,
    ARA::PlugIn::AudioSource::PropertiesPtr newProperties) noexcept
{
    notify_listeners (ARAAudioSource, willUpdateAudioSourceProperties, audioSource, newProperties);
}

void ARADocumentController::didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    notify_listeners (ARAAudioSource, didUpdateAudioSourceProperties, audioSource);
}

void ARADocumentController::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes scopeFlags) noexcept
{
    notify_listeners (ARAAudioSource, didUpdateAudioSourceContent, audioSource, scopeFlags);
}

void ARADocumentController::willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    notify_listeners (ARAAudioSource, willEnableAudioSourceSamplesAccess, audioSource, enable);
}

void ARADocumentController::didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    notify_listeners (ARAAudioSource, didEnableAudioSourceSamplesAccess, audioSource, enable);
}

void ARADocumentController::doDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept
{
    notify_listeners (ARAAudioSource, doDeactivateAudioSourceForUndoHistory, audioSource, deactivate);
}

void ARADocumentController::didAddAudioModificationToAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept
{
    notify_listeners (ARAAudioSource, didAddAudioModification, audioSource, static_cast<ARAAudioModification*> (audioModification));
}

void ARADocumentController::willRemoveAudioModificationFromAudioSource (ARA::PlugIn::AudioSource* audioSource, ARA::PlugIn::AudioModification* audioModification) noexcept
{
    notify_listeners (ARAAudioSource, willRemoveAudioModification, audioSource, static_cast<ARAAudioModification*> (audioModification));
}

void ARADocumentController::willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    notify_listeners (ARAAudioSource, willDestroyAudioSource, audioSource);
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
    return new ARAAudioModification (static_cast<ARAAudioSource*> (audioSource), hostRef);
}

void ARADocumentController::willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::AudioModification::PropertiesPtr newProperties) noexcept
{
    notify_listeners (ARAAudioModification, willUpdateAudioModificationProperties, audioModification, newProperties);
}

void ARADocumentController::didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept
{
    notify_listeners (ARAAudioModification, didUpdateAudioModificationProperties, audioModification);
}

void ARADocumentController::doDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept
{
    notify_listeners (ARAAudioModification, doDeactivateAudioModificationForUndoHistory, audioModification, deactivate);
}

void ARADocumentController::didAddPlaybackRegionToAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    notify_listeners (ARAAudioModification, didAddPlaybackRegion, audioModification, static_cast<ARAPlaybackRegion*> (playbackRegion));
}

void ARADocumentController::willRemovePlaybackRegionFromAudioModification (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    notify_listeners (ARAAudioModification, willRemovePlaybackRegion, audioModification, static_cast<ARAPlaybackRegion*> (playbackRegion));
}

void ARADocumentController::willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept
{
    notify_listeners (ARAAudioModification, willDestroyAudioModification, audioModification);
}

//==============================================================================

ARA::PlugIn::PlaybackRegion* ARADocumentController::doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept
{
    return new ARAPlaybackRegion (static_cast<ARAAudioModification*>(modification), hostRef);
}

void ARADocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PlaybackRegion::PropertiesPtr newProperties) noexcept
{
    notify_listeners (ARAPlaybackRegion, willUpdatePlaybackRegionProperties, playbackRegion, newProperties);
}

void ARADocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept   
{
    notify_listeners (ARAPlaybackRegion, didUpdatePlaybackRegionProperties, playbackRegion);
}

void ARADocumentController::doGetPlaybackRegionHeadAndTailTime (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::ARATimeDuration* headTime, ARA::ARATimeDuration* tailTime) noexcept
{
    auto araPlaybackRegion = static_cast<ARAPlaybackRegion*> (playbackRegion);
    *headTime = araPlaybackRegion->getHeadTime();
    *tailTime = araPlaybackRegion->getTailTime();
}

void ARADocumentController::willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    notify_listeners (ARAPlaybackRegion, willDestroyPlaybackRegion, playbackRegion);
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

#undef notify_listeners
