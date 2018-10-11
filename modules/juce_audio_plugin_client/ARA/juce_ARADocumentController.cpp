#pragma once

#include "juce_ARADocumentController.h"
#include "juce_ARAAudioSource.h"
#include "juce_ARARegionSequence.h"

const ARA::ARAFactory* ARA::PlugIn::DocumentController::getARAFactory ()
{
    using namespace ARA;

    static ARAFactory* factory = nullptr;
    if (factory == nullptr)
    {
        factory = new SizedStruct<ARA_MEMBER_PTR_ARGS (ARAFactory, supportedPlaybackTransformationFlags)>;

        // Supported API generations (forced, I guess...)
        factory->lowestSupportedApiGeneration = kARAAPIGeneration_2_0_Draft;
        factory->highestSupportedApiGeneration = kARAAPIGeneration_2_0_Final;

        // Factory ID
        factory->factoryID = JucePlugin_ARAFactoryID;

        // ARA lifetime management functions
        factory->initializeARAWithConfiguration = ARAInitialize;
        factory->uninitializeARA = ARAUninitialize;

        // Plugin Name
        factory->plugInName = JucePlugin_Name;

        // Manufacturer Name
        factory->manufacturerName = JucePlugin_Manufacturer;

        // Info URL
        factory->informationURL = JucePlugin_ManufacturerWebsite;

        // Version
        factory->version = JucePlugin_VersionString;

        // Document Controller factory function
        factory->createDocumentControllerWithDocument = ARACreateDocumentControllerWithDocumentInstance;

        // Document archive ID
        factory->documentArchiveID = JucePlugin_ARADocumentArchiveID;

        // TODO Compatible archive IDs and count

        // Content types
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
        for (int i = 0; i < sizeof(araContentVars) / sizeof(ARAContentType); i++)
        {
            if (JucePlugin_ARAContentTypes & (1 << i))
                contentTypes.push_back(araContentVars[i]);
        }

        delete factory->analyzeableContentTypes;
        factory->analyzeableContentTypesCount = (int) contentTypes.size();
        factory->analyzeableContentTypes = contentTypes.data();

        // Playback Transformation Flags
        static ARAPlaybackTransformationFlags araPlaybackTransformations[]{
            kARAPlaybackTransformationTimestretch,
            kARAPlaybackTransformationTimestretchReflectingTempo,
            kARAPlaybackTransformationContentBasedFadeAtTail,
            kARAPlaybackTransformationContentBasedFadeAtHead
        };

        factory->supportedPlaybackTransformationFlags = 0;
        for (int i = 0; i < sizeof(araPlaybackTransformations) / sizeof(ARAPlaybackTransformationFlags); i++)
        {
            if (JucePlugin_ARATransformationFlags & (1 << i))
                factory->supportedPlaybackTransformationFlags |= araPlaybackTransformations[i];
        }

        // TODO Algorithms
    }

    return factory;
}

namespace juce
{

ARADocumentController::ARADocumentController()
{
}

ARADocumentController::~ARADocumentController()
{
}

ARA::PlugIn::AudioSource* ARADocumentController::doCreateAudioSource (ARA::PlugIn::Document *document, ARA::ARAAudioSourceHostRef hostRef)
{
    return new ARAAudioSource (document, hostRef);
}

ARA::PlugIn::RegionSequence* ARADocumentController::doCreateRegionSequence (ARA::PlugIn::Document *document, ARA::ARARegionSequenceHostRef hostRef)
{
    return new ARARegionSequence (document, hostRef);
}

void ARADocumentController::willEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable)
{
    auto source = static_cast<ARAAudioSource*> (audioSource);
    source->willEnableSamplesAccess (enable);
}

void ARADocumentController::didEnableAudioSourceSamplesAccess (ARA::PlugIn::AudioSource* audioSource, bool enable)
{
    auto source = static_cast<ARAAudioSource*> (audioSource);
    source->didEnableSamplesAccess (enable);
}

void ARADocumentController::willUpdateAudioSourceProperties (
    ARA::PlugIn::AudioSource* audioSource,
    ARA::PlugIn::PropertiesPtr<ARA::ARAAudioSourceProperties>)
{
    static_cast<ARAAudioSource*> (audioSource)->willUpdateProperties();
}

void ARADocumentController::didUpdateAudioSourceProperties (ARA::PlugIn::AudioSource* audioSource)
{
    static_cast<ARAAudioSource*> (audioSource)->didUpdateProperties();
}

void ARADocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties)
{
    ARARegionSequence::willUpdatePlaybackRegionProperties (playbackRegion, newProperties);
}

void ARADocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion)
{
    ARARegionSequence::didUpdatePlaybackRegionProperties (playbackRegion);
}

} // namespace juce