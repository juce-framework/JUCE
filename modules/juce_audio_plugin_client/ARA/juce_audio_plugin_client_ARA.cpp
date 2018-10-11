// TODO formalize ARA includes

#include <ARA_Library/PlugIn/ARAPlug.cpp>
#include <ARA_Library/Debug/ARADebug.c>
#include <ARA_Library/Dispatch/ARAPlugInDispatch.cpp>

// Include these source files directly for now
#include "juce_ARADocumentController.cpp"
#include "juce_ARARegionSequence.cpp"
#include "juce_ARAAudioSource.cpp"

#include "JuceHeader.h"

const ARA::ARAFactory* ARA::PlugIn::DocumentController::getARAFactory ()
{
    using namespace ARA;

    static ARAFactory* factory = nullptr;
    if (!factory)
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