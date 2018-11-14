#include "juce_ARADocumentController.h"
#include "juce_ARARegionSequence.h"
#include "juce_ARAAudioSource.h"
#include "juce_ARAPlaybackRegion.h"

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

ARA::PlugIn::PlaybackRegion* ARADocumentController::doCreatePlaybackRegion (ARA::PlugIn::AudioModification* modification, ARA::ARAPlaybackRegionHostRef hostRef) noexcept 
{
    return new ARAPlaybackRegion (modification, hostRef);
}

void ARADocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept
{
    // TODO JUCE_ARA
    // replace these static functions with listener callbacks
    ARARegionSequence::willUpdatePlaybackRegionProperties (playbackRegion, newProperties);
    static_cast<ARAPlaybackRegion*> (playbackRegion)->willUpdatePlaybackRegionProperties (newProperties);
}

void ARADocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    // TODO JUCE_ARA
    // replace these static functions with listener callbacks
    ARARegionSequence::didUpdatePlaybackRegionProperties (playbackRegion);
    static_cast<ARAPlaybackRegion*> (playbackRegion)->didUpdatePlaybackRegionProperties ();
}

void ARADocumentController::willDestroyPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    static_cast<ARAPlaybackRegion*> (playbackRegion)->willDestroyPlaybackRegion ();
}
//==============================================================================

ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource (ARA::PlugIn::Document *document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    return new ARAAudioSource (document, hostRef);
}

void ARADocumentController::willUpdateAudioSourceProperties (
    ARA::PlugIn::AudioSource* audioSource,
    ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties> properties) noexcept
{
    for (ARAAudioSourceUpdateListener* updateListener : audioSourceUpdateListeners)
        updateListener->willUpdateAudioSourceProperties (audioSource, properties);
}

void ARADocumentController::didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    for (ARAAudioSourceUpdateListener* updateListener : audioSourceUpdateListeners)
        updateListener->didUpdateAudioSourceProperties (audioSource);
}

void ARADocumentController::doUpdateAudioSourceContent (ARA::PlugIn::AudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept
{
    for (ARAAudioSourceUpdateListener* updateListener : audioSourceUpdateListeners)
        updateListener->doUpdateAudioSourceContent (audioSource, range, flags);
}

void ARADocumentController::willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    for (ARAAudioSourceUpdateListener* updateListener : audioSourceUpdateListeners)
        updateListener->willEnableAudioSourceSamplesAccess (audioSource, enable);
}

void ARADocumentController::didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    for (ARAAudioSourceUpdateListener* updateListener : audioSourceUpdateListeners)
        updateListener->didEnableAudioSourceSamplesAccess (audioSource, enable);
}

void ARADocumentController::doDeactivateAudioSourceForUndoHistory (ARA::PlugIn::AudioSource* audioSource, bool deactivate) noexcept
{
    for (ARAAudioSourceUpdateListener* updateListener : audioSourceUpdateListeners)
        updateListener->doDeactivateAudioSourceForUndoHistory (audioSource, deactivate);
}

void ARADocumentController::willDestroyAudioSource (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    for (ARAAudioSourceUpdateListener* updateListener : audioSourceUpdateListeners)
        updateListener->willDestroyAudioSource (audioSource);
}

void ARADocumentController::addAudioSourceUpdateListener (ARAAudioSourceUpdateListener* updateListener)
{
    audioSourceUpdateListeners.push_back (updateListener);
}

void ARADocumentController::removeAudioSourceUpdateListener (ARAAudioSourceUpdateListener* updateListener)
{
    find_erase (audioSourceUpdateListeners, updateListener);
}

//==============================================================================

ARA::PlugIn::RegionSequence* ARADocumentController::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return new ARARegionSequence (document, hostRef);
}

void ARADocumentController::willUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties) noexcept
{
    for (ARARegionSequenceUpdateListener* updateListener : regionSequenceUpdateListeners)
        updateListener->willUpdateRegionSequenceProperties (regionSequence, newProperties);
}

void ARADocumentController::didUpdateRegionSequenceProperties (ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    for (ARARegionSequenceUpdateListener* updateListener : regionSequenceUpdateListeners)
        updateListener->didUpdateRegionSequenceProperties (regionSequence);
}

void ARADocumentController::willDestroyRegionSequence (ARA::PlugIn::RegionSequence* regionSequence) noexcept
{
    for (ARARegionSequenceUpdateListener* updateListener : regionSequenceUpdateListeners)
        updateListener->willDestroyRegionSequence (regionSequence);
}

void ARADocumentController::willReorderRegionSequencesInMusicalContext (const ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    for (ARARegionSequenceUpdateListener* updateListener : regionSequenceUpdateListeners)
        updateListener->willReorderRegionSequencesInMusicalContext (musicalContext);
}

void ARADocumentController::didReorderRegionSequencesInMusicalContext (const ARA::PlugIn::MusicalContext* musicalContext) noexcept
{
    for (ARARegionSequenceUpdateListener* updateListener : regionSequenceUpdateListeners)
        updateListener->didReorderRegionSequencesInMusicalContext (musicalContext);
}

void ARADocumentController::willReorderRegionSequences () noexcept
{
    for (ARARegionSequenceUpdateListener* updateListener : regionSequenceUpdateListeners)
        updateListener->willReorderRegionSequences ();
}

void ARADocumentController::didReorderRegionSequences () noexcept
{
    for (ARARegionSequenceUpdateListener* updateListener : regionSequenceUpdateListeners)
        updateListener->didReorderRegionSequences ();
}

void ARADocumentController::addRegionSequenceUpdateListener (ARARegionSequenceUpdateListener* updateListener)
{
    regionSequenceUpdateListeners.push_back (updateListener);
}

void ARADocumentController::removeRegionSequenceUpdateListener (ARARegionSequenceUpdateListener* updateListener)
{
    find_erase (regionSequenceUpdateListeners, updateListener);
}

ARARegionSequenceUpdateListener::ARARegionSequenceUpdateListener (ARA::PlugIn::DocumentController *documentController)
: araDocumentController (static_cast<ARADocumentController*> (documentController))
{
    if (araDocumentController)
        araDocumentController->addRegionSequenceUpdateListener (this);
}

ARARegionSequenceUpdateListener::~ARARegionSequenceUpdateListener ()
{
    if (araDocumentController)
        araDocumentController->removeRegionSequenceUpdateListener (this);
}

ARAAudioSourceUpdateListener::ARAAudioSourceUpdateListener (ARA::PlugIn::DocumentController *documentController)
    : araDocumentController (static_cast<ARADocumentController*> (documentController))
{
    if (araDocumentController)
        araDocumentController->addAudioSourceUpdateListener (this);
}

ARAAudioSourceUpdateListener::~ARAAudioSourceUpdateListener ()
{
    if (araDocumentController)
        araDocumentController->removeAudioSourceUpdateListener (this);
}

} // namespace juce
