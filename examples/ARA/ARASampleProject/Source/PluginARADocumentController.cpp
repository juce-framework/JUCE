/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#include "PluginARADocumentController.h"
#include "PluginARAPlaybackRenderer.h"

ARASampleProjectDocumentController::ARASampleProjectDocumentController() noexcept
: juce::ARADocumentController()
{
    araAudioSourceReadingThread.reset (new TimeSliceThread (String (JucePlugin_Name) + " ARA Sample Reading Thread"));
    araAudioSourceReadingThread->startThread();
}

// return an instance of our playback renderer implementation
ARA::PlugIn::PlaybackRenderer* ARASampleProjectDocumentController::doCreatePlaybackRenderer() noexcept
{
    return new ARASampleProjectPlaybackRenderer (this, *araAudioSourceReadingThread.get(), (1 << 16));
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
    return new ARARegionSequenceReader (regionSequence, static_cast<ARASampleProjectPlaybackRenderer*> (doCreatePlaybackRenderer()));
}

ARARegionSequenceReader::ARARegionSequenceReader (ARA::PlugIn::RegionSequence* regionSequence, ARA::PlugIn::PlaybackRenderer* playbackRenderer)
: AudioFormatReader (nullptr, "ARAAudioSourceReader"),
  regionSequence (regionSequence),
  playbackRenderer (playbackRenderer)
{
    // TODO JUCE_ARA
    // deal with single and double precision floats
    bitsPerSample = 32;
    usesFloatingPointData = true;
    numChannels = 0;
    lengthInSamples = 0;
    sampleRate = 0;

    for (ARA::PlugIn::PlaybackRegion* region : regionSequence->getPlaybackRegions())
    {
        ARA::PlugIn::AudioModification* modification = region->getAudioModification();
        ARAAudioSource* source = static_cast<ARAAudioSource*> (modification->getAudioSource());

        if (sampleRate == 0.0)
            sampleRate = source->getSampleRate();

        if (sampleRate != source->getSampleRate())
        {
            // Skip regions with mis-matching sample-rates!
            continue;
        }

        numChannels = std::max (numChannels, (unsigned int) source->getChannelCount());
        lengthInSamples = std::max (lengthInSamples, region->getEndInPlaybackSamples (sampleRate));

        playbackRenderer->addPlaybackRegion (ARA::PlugIn::toRef (region));
        static_cast<ARAPlaybackRegion*>(region)->addListener (this);
    }
}

ARARegionSequenceReader::~ARARegionSequenceReader()
{
    ScopedWriteLock scopedWrite (lock);
    for (ARA::PlugIn::PlaybackRegion* region : regionSequence->getPlaybackRegions())
        static_cast<ARAPlaybackRegion*>(region)->removeListener (this);
    delete playbackRenderer;
}

bool ARARegionSequenceReader::readSamples (
    int** destSamples,
    int numDestChannels,
    int startOffsetInDestBuffer,
    int64 startSampleInFile,
    int numSamples)
{
    // render our ARA playback regions for this time duration using the ARA playback renderer instance
    if (! lock.tryEnterRead())
    {
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            FloatVectorOperations::clear ((float *) destSamples[chan_i], numSamples);
        return false;
    }

    AudioBuffer<float> buffer ((float **) destSamples, numDestChannels, startOffsetInDestBuffer, numSamples);
    static_cast<ARASampleProjectPlaybackRenderer*>(playbackRenderer)->renderSamples (buffer, sampleRate, startSampleInFile, true);
    lock.exitRead();
    return true;
}


void ARARegionSequenceReader::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept
{
    if (contains (playbackRenderer->getPlaybackRegions(), static_cast<ARA::PlugIn::PlaybackRegion*> (playbackRegion)))
    {
        if (newProperties->regionSequenceRef != ARA::PlugIn::toRef (regionSequence))
        {
            ScopedWriteLock scopedWrite (lock);
            playbackRegion->removeListener (this);
            playbackRenderer->removePlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
        }
    }
    else if (newProperties->regionSequenceRef == ARA::PlugIn::toRef (regionSequence))
    {
        ScopedWriteLock scopedWrite (lock);
        playbackRegion->addListener (this);
        playbackRenderer->addPlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
    }
}

void ARARegionSequenceReader::willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept
{
    if (contains (playbackRenderer->getPlaybackRegions(), static_cast<ARA::PlugIn::PlaybackRegion*> (playbackRegion)))
    {
        ScopedWriteLock scopedWrite (lock);
        playbackRegion->removeListener (this);
        playbackRenderer->removePlaybackRegion (ARA::PlugIn::toRef (playbackRegion));
    }
}
