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

// return an instance of our editor view implementation
ARA::PlugIn::EditorView* ARASampleProjectDocumentController::doCreateEditorView() noexcept
{
    return new ARASampleProjectEditorView (this);
}

// return an instance of our playback renderer implementation
ARA::PlugIn::PlaybackRenderer* ARASampleProjectDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARASampleProjectPlaybackRenderer (this, *araAudioSourceReadingThread.get (), (1 << 16));
}

// return an instance of our region sequence class, which ties in with a special region sequence audio reader
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


//==============================================================================

AudioFormatReader* ARASampleProjectDocumentController::createRegionSequenceReader (ARA::PlugIn::RegionSequence* regionSequence)
{
    return new ARARegionSequenceReader (regionSequence, static_cast<ARASampleProjectPlaybackRenderer*> (doCreatePlaybackRenderer ()));
}

ARARegionSequenceReader::ARARegionSequenceReader (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRenderer* playbackRenderer)
: AudioFormatReader (nullptr, "ARAAudioSourceReader"),
  _playbackRenderer (playbackRenderer)
{
    // TODO JUCE_ARA
    // deal with single and double precision floats
    bitsPerSample = 32;
    usesFloatingPointData = true;
    numChannels = 0;
    lengthInSamples = 0;
    sampleRate = 0;

    for (ARA::PlugIn::PlaybackRegion* region : regionSequence->getPlaybackRegions ())
    {
        ARA::PlugIn::AudioModification* modification = region->getAudioModification ();
        ARAAudioSource* source = static_cast<ARAAudioSource*> (modification->getAudioSource ());

        if (sampleRate == 0.0)
            sampleRate = source->getSampleRate ();

        if (sampleRate != source->getSampleRate ())
        {
            // Skip regions with mis-matching sample-rates!
            continue;
        }

        numChannels = std::max (numChannels, (unsigned int) source->getChannelCount ());
        lengthInSamples = std::max (lengthInSamples, region->getEndInPlaybackSamples (sampleRate));

        _playbackRenderer->addPlaybackRegion (ARA::PlugIn::toRef (region));
    }
}

ARARegionSequenceReader::~ARARegionSequenceReader ()
{
    // TODO JUCE_ARA
    // do we have to remove all playback before destroying the renderer?
    delete _playbackRenderer;
}

bool ARARegionSequenceReader::readSamples (
    int** destSamples,
    int numDestChannels,
    int startOffsetInDestBuffer,
    int64 startSampleInFile,
    int numSamples)
{
    // render our ARA playback regions for this time duration using the ARA playback renderer instance
    AudioBuffer<float> buffer ((float **) destSamples, numDestChannels, startOffsetInDestBuffer, numSamples);
    static_cast<ARASampleProjectPlaybackRenderer*>(_playbackRenderer)->renderPlaybackRegions (buffer, sampleRate, startSampleInFile, true);
    return true;
}
