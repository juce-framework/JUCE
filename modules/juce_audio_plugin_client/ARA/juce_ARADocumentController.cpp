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
    audioSource->didUpdateAudioSourceContent (scopeFlags);

    if (notifyAllAudioModificationsAndPlaybackRegions)
    {
        for (auto audioModification : audioSource->getAudioModifications())
            notifyAudioModificationContentChanged (static_cast<ARAAudioModification*> (audioModification), scopeFlags, true);
    }

    audioSourceUpdates[audioSource] += scopeFlags;
}

void ARADocumentController::notifyAudioModificationContentChanged (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags, bool notifyAllPlaybackRegions)
{
    audioModification->didUpdateAudioModificationContent (scopeFlags);
    
    if (notifyAllPlaybackRegions)
    {
        for (auto playbackRegion : audioModification->getPlaybackRegions())
            notifyPlaybackRegionContentChanged (static_cast<ARAPlaybackRegion*> (playbackRegion), scopeFlags);
    }

    audioModificationUpdates[audioModification] += scopeFlags;
}

void ARADocumentController::notifyPlaybackRegionContentChanged (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags)
{
    playbackRegion->didUpdatePlaybackRegionContent (scopeFlags);

    playbackRegionUpdates[playbackRegion] += scopeFlags;
}

//==============================================================================
    
ARA::PlugIn::Document* ARADocumentController::doCreateDocument (ARA::PlugIn::DocumentController* documentController) noexcept
{
    return new ARADocument (static_cast<ARADocumentController*> (documentController));
}

void ARADocumentController::doBeginEditing() noexcept
{
    static_cast<ARADocument*>(getDocument ())->doBeginEditing ();
}

void ARADocumentController::doEndEditing() noexcept
{
    static_cast<ARADocument*>(getDocument ())->doEndEditing ();
}

void ARADocumentController::doNotifyModelUpdates () noexcept
{
    auto modelUpdateController = getHostInstance ()->getModelUpdateController ();
    if (modelUpdateController != nullptr)
    {
        for (auto& audioSourceUpdate : audioSourceUpdates)
            modelUpdateController->notifyAudioSourceContentChanged (audioSourceUpdate.first->getHostRef (), nullptr, audioSourceUpdate.second);

        for (auto& audioModificationUpdate : audioModificationUpdates)
            modelUpdateController->notifyAudioModificationContentChanged (audioModificationUpdate.first->getHostRef (), nullptr, audioModificationUpdate.second);

        for (auto& playbackRegionUpdate : playbackRegionUpdates)
            modelUpdateController->notifyPlaybackRegionContentChanged (playbackRegionUpdate.first->getHostRef (), nullptr, playbackRegionUpdate.second);
    }
 
    audioSourceUpdates.clear ();
    audioModificationUpdates.clear ();
    playbackRegionUpdates.clear ();
}

void ARADocumentController::willUpdateDocumentProperties (ARA::PlugIn::Document* document, ARA::PlugIn::Document::PropertiesPtr newProperties) noexcept
{
    static_cast<ARADocument*>(document)->willUpdateDocumentProperties (newProperties);
}

void ARADocumentController::didUpdateDocumentProperties (ARA::PlugIn::Document* document) noexcept
{
    static_cast<ARADocument*>(document)->didUpdateDocumentProperties ();
}

void ARADocumentController::willDestroyDocument (ARA::PlugIn::Document* document) noexcept
{
    static_cast<ARADocument*>(document)->willDestroyDocument ();
}

//==============================================================================

ARA::PlugIn::MusicalContext* ARADocumentController::doCreateMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef) noexcept
{
    return new ARAMusicalContext (static_cast<ARADocument*> (document), hostRef);
}

void ARADocumentController::willUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept
{
    static_cast<ARAMusicalContext*> (musicalContext)->willUpdateMusicalContextProperties (newProperties);
}

void ARADocumentController::didUpdateMusicalContextProperties (ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    static_cast<ARAMusicalContext*> (musicalContext)->didUpdateMusicalContextProperties();
}

void ARADocumentController::doUpdateMusicalContextContent (ARA::PlugIn::MusicalContext* musicalContext, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes scopeFlags) noexcept
{
    static_cast<ARAMusicalContext*> (musicalContext)->didUpdateMusicalContextContent (scopeFlags);
}

void ARADocumentController::willDestroyMusicalContext (ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    static_cast<ARAMusicalContext*> (musicalContext)->willDestroyMusicalContext();
}

//==============================================================================

ARA::PlugIn::RegionSequence* ARADocumentController::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return new ARARegionSequence (static_cast<ARADocument*> (document), hostRef);
}

void ARADocumentController::willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept
{
    static_cast<ARARegionSequence*> (regionSequence)->willUpdateRegionSequenceProperties (newProperties);
}

void ARADocumentController::didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    static_cast<ARARegionSequence*> (regionSequence)->didUpdateRegionSequenceProperties();
}

void ARADocumentController::willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    static_cast<ARARegionSequence*> (regionSequence)->willDestroyRegionSequence();
}

void ARADocumentController::willRemovePlaybackRegionFromRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    static_cast<ARARegionSequence*> (regionSequence)->
        willRemovePlaybackRegionFromRegionSequence (static_cast<ARAPlaybackRegion*>(playbackRegion));
}

void ARADocumentController::didAddPlaybackRegionToRegionSequence (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    static_cast<ARARegionSequence*> (regionSequence)->
        didAddPlaybackRegionToRegionSequence (static_cast<ARAPlaybackRegion*>(playbackRegion));
}

//==============================================================================

ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource (ARA::PlugIn::Document *document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    return new ARAAudioSource (document, hostRef);
}

void ARADocumentController::willUpdateAudioSourceProperties (
    ARA::PlugIn::AudioSource* audioSource,
    ARA::PlugIn::AudioSource::PropertiesPtr newProperties) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->willUpdateAudioSourceProperties (newProperties);
}

void ARADocumentController::didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->didUpdateAudioSourceProperties();
}

void ARADocumentController::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* /*range*/, ARA::ContentUpdateScopes scopeFlags) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->didUpdateAudioSourceContent (scopeFlags);
}

void ARADocumentController::willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->willEnableAudioSourceSamplesAccess (enable);
}

void ARADocumentController::didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->didEnableAudioSourceSamplesAccess (enable);
}

void ARADocumentController::doDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->doDeactivateAudioSourceForUndoHistory (deactivate);
}

void ARADocumentController::willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->willDestroyAudioSource();
}

AudioFormatReader* ARADocumentController::createAudioSourceReader (ARAAudioSource* audioSource)
{
    return new ARAAudioSourceReader (audioSource);
}

ARAPlaybackRegionReader* ARADocumentController::createPlaybackRegionReader (std::vector<ARAPlaybackRegion*> playbackRegions, bool nonRealtime)
{
    return new ARAPlaybackRegionReader (static_cast<ARAPlaybackRenderer*>(doCreatePlaybackRenderer()), playbackRegions, nonRealtime);
}

ARARegionSequenceReader* ARADocumentController::createRegionSequenceReader (ARARegionSequence* regionSequence, bool nonRealtime)
{
    return new ARARegionSequenceReader (static_cast<ARAPlaybackRenderer*>(doCreatePlaybackRenderer()), regionSequence, nonRealtime);
}

//==============================================================================

ARA::PlugIn::AudioModification* ARADocumentController::doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef) noexcept
{
    return new ARAAudioModification (audioSource, hostRef);
}

void ARADocumentController::willUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification, ARA::PlugIn::AudioModification::PropertiesPtr newProperties) noexcept
{
    static_cast<ARAAudioModification*> (audioModification)->willUpdateAudioModificationProperties (newProperties);
}

void ARADocumentController::didUpdateAudioModificationProperties (ARA::PlugIn::AudioModification* audioModification) noexcept
{
    static_cast<ARAAudioModification*> (audioModification)->didUpdateAudioModificationProperties();
}

void ARADocumentController::doDeactivateAudioModificationForUndoHistory (ARA::PlugIn::AudioModification* audioModification, bool deactivate) noexcept
{
    static_cast<ARAAudioModification*> (audioModification)->doDeactivateAudioModificationForUndoHistory (deactivate);
}

void ARADocumentController::willDestroyAudioModification (ARA::PlugIn::AudioModification* audioModification) noexcept
{
    static_cast<ARAAudioModification*> (audioModification)->willDestroyAudioModification();
}

//==============================================================================

ARA::PlugIn::PlaybackRegion* ARADocumentController::doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept
{
    return new ARAPlaybackRegion (modification, hostRef);
}

void ARADocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PlaybackRegion::PropertiesPtr newProperties) noexcept
{
    static_cast<ARAPlaybackRegion*> (playbackRegion)->willUpdatePlaybackRegionProperties (newProperties);
}

void ARADocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    static_cast<ARAPlaybackRegion*> (playbackRegion)->didUpdatePlaybackRegionProperties();
}

void ARADocumentController::willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    static_cast<ARAPlaybackRegion*> (playbackRegion)->willDestroyPlaybackRegion();
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
