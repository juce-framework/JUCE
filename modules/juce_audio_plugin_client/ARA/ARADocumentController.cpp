#pragma once

#include "ARADocumentController.h"
#include "ARAAudioSource.h"
#include "ARARegionSequence.h"

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
