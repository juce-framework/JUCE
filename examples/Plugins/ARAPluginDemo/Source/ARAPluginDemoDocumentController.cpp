#include "ARAPluginDemoDocumentController.h"
#include "ARAPluginDemoAudioModification.h"
#include "ARAPluginDemoPlaybackRenderer.h"

//==============================================================================
ARA::PlugIn::AudioModification* ARAPluginDemoDocumentController::doCreateAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARA::PlugIn::AudioModification* optionalModificationToClone) noexcept
{
    return new ARAPluginDemoAudioModification (static_cast<juce::ARAAudioSource*> (audioSource), hostRef, static_cast<const juce::ARAAudioModification*> (optionalModificationToClone));
}

ARA::PlugIn::PlaybackRenderer* ARAPluginDemoDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new PluginDemoPlaybackRenderer (this);
}

//==============================================================================
bool ARAPluginDemoDocumentController::doRestoreObjectsFromStream (juce::ARAInputStream& input, const juce::ARARestoreObjectsFilter* filter) noexcept
{
    // start reading data from the archive, starting with the number of audio modifications in the archive
    const auto numAudioModifications = input.readInt64();

    // loop over stored audio modification data
    for (juce::int64 i = 0; i < numAudioModifications; ++i)
    {
        const float progressVal = static_cast<float> (i) / static_cast<float> (numAudioModifications);
        getHostArchivingController()->notifyDocumentUnarchivingProgress (progressVal);

        // read audio modification persistent ID and analysis result from archive
        const juce::String persistentID = input.readString();
        const bool dimmed = input.readBool();

        // find audio modification to restore the state to (drop state if not to be loaded)
        auto audioModification = filter->getAudioModificationToRestoreStateWithID<ARAPluginDemoAudioModification> (persistentID.getCharPointer());
        if (! audioModification)
            continue;

        bool dimChanged = (dimmed != audioModification->isDimmed());
        audioModification->setDimmed (dimmed);

        // if the dim state changed, send a sample content change notification without notifying the host
        if (dimChanged)
        {
            audioModification->notifyContentChanged (juce::ARAContentUpdateScopes::samplesAreAffected(), false);
            for (auto playbackRegion : audioModification->getPlaybackRegions())
                playbackRegion->notifyContentChanged (juce::ARAContentUpdateScopes::samplesAreAffected(), false);
        }
    }

    getHostArchivingController()->notifyDocumentUnarchivingProgress (1.0f);

    return ! input.failed();
}

bool ARAPluginDemoDocumentController::doStoreObjectsToStream (juce::ARAOutputStream& output, const juce::ARAStoreObjectsFilter* filter) noexcept
{
    // this example implementation only deals with audio modification states
    const auto& audioModificationsToPersist{ filter->getAudioModificationsToStore<ARAPluginDemoAudioModification>() };

    // write the number of audio modifications we are persisting
    const size_t numAudioModifications = audioModificationsToPersist.size();
    bool success = output.writeInt64 ((juce::int64)numAudioModifications);

    // for each audio modification to persist, persist its ID followed by whether or not it's dimmed
    for (size_t i = 0; i < numAudioModifications; ++i)
    {
        // write persistent ID and dim state
        success = success && output.writeString (audioModificationsToPersist[i]->getPersistentID());
        success = success && output.writeBool (audioModificationsToPersist[i]->isDimmed());

        const float progressVal = static_cast<float> (i) / static_cast<float> (numAudioModifications);
        getHostArchivingController()->notifyDocumentArchivingProgress (progressVal);
    }

    getHostArchivingController()->notifyDocumentArchivingProgress (1.0);

    return success;
}

bool ARAPluginDemoDocumentController::doIsAudioModificationPreservingAudioSourceSignal (ARA::PlugIn::AudioModification* audioModification) noexcept
{
    return ! static_cast<ARAPluginDemoAudioModification*>(audioModification)->isDimmed();
}

//==============================================================================
// This creates the static ARAFactory instances for the plugin.
const ARA::ARAFactory* JUCE_CALLTYPE createARAFactory()
{
    return juce::ARADocumentController::createARAFactory<ARAPluginDemoDocumentController>();
}
