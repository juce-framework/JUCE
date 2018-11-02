#pragma once

#include "juce_ARADocumentController.h"
#include "juce_ARAAudioSource.h"
#include "juce_ARARegionSequence.h"

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

ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource (ARA::PlugIn::Document *document, ARA::ARAAudioSourceHostRef hostRef) noexcept
{
    return new ARAAudioSource (document, hostRef);
}

ARA::PlugIn::RegionSequence* ARADocumentController::doCreateRegionSequence (ARA::PlugIn::Document *document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return new ARARegionSequence (document, hostRef);
}

void ARADocumentController::willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    auto source = static_cast<ARAAudioSource*> (audioSource);
    source->willEnableSamplesAccess (enable);
}

void ARADocumentController::didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable) noexcept
{
    auto source = static_cast<ARAAudioSource*> (audioSource);
    source->didEnableSamplesAccess (enable);
}

void ARADocumentController::willUpdateAudioSourceProperties (
    ARA::PlugIn::AudioSource* audioSource,
    ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties>) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->willUpdateProperties();
}

void ARADocumentController::didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource) noexcept
{
    static_cast<ARAAudioSource*> (audioSource)->didUpdateProperties();
}

void ARADocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept
{
    ARARegionSequence::willUpdatePlaybackRegionProperties (playbackRegion, newProperties);
}

void ARADocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    ARARegionSequence::didUpdatePlaybackRegionProperties (playbackRegion);
}

} // namespace juce
