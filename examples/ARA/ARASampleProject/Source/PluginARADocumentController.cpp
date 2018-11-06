/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#include "PluginARADocumentController.h"
#include "PluginARAPlaybackRenderer.h"
#include "PluginARAEditorView.h"
#include "PluginARARegionSequence.h"

ARASampleProjectDocumentController::ARASampleProjectDocumentController() noexcept
: juce::ARADocumentController()
{
    araAudioSourceReadingThread.reset (new TimeSliceThread (String (JucePlugin_Name) + " ARA Sample Reading Thread"));
    araAudioSourceReadingThread->startThread();
}

ARA::PlugIn::EditorView* ARASampleProjectDocumentController::doCreateEditorView() noexcept
{
    return new ARASampleProjectEditorView (this);
}

ARA::PlugIn::PlaybackRenderer* ARASampleProjectDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARASampleProjectPlaybackRenderer (this, *araAudioSourceReadingThread.get(), (1 << 16));
}

ARA::PlugIn::RegionSequence* ARASampleProjectDocumentController::doCreateRegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef) noexcept
{
    return new ARASampleProjectRegionSequence (document, hostRef);
}

void ARASampleProjectDocumentController::willUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept
{
    ARASampleProjectRegionSequence::willUpdatePlaybackRegionProperties (playbackRegion, newProperties);
}

void ARASampleProjectDocumentController::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept
{
    ARASampleProjectRegionSequence::didUpdatePlaybackRegionProperties (playbackRegion);
}

//==============================================================================
// This creates new instances of the document controller..
ARA::PlugIn::DocumentController* ARA::PlugIn::DocumentController::doCreateDocumentController() noexcept
{
    return new ARASampleProjectDocumentController();
};


