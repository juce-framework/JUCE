#include "juce_ARARegionSequence.h"

namespace juce
{

#if JUCE_DEBUG
 bool ARARegionSequence::stateUpdatePlaybackRegionProperties = false;
#endif

class ARARegionSequence::Reader : public AudioFormatReader
{
    friend class ARARegionSequence;

public:
    Reader (ARARegionSequence*, double sampleRate);
    virtual ~Reader();

    bool readSamples (
        int** destSamples,
        int numDestChannels,
        int startOffsetInDestBuffer,
        int64 startSampleInFile,
        int numSamples) override;

private:
    Ref::Ptr ref;
    std::map<ARA::PlugIn::AudioSource*, AudioFormatReader*> sourceReaders;
    AudioSampleBuffer sampleBuffer;
};

ARARegionSequence::ARARegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef)
    : ARA::PlugIn::RegionSequence (document, hostRef)
{
    ref = new Ref (this);
    prevSequenceForNewPlaybackRegion = nullptr;
}

ARARegionSequence::~ARARegionSequence()
{
    ref->reset();
}

AudioFormatReader* ARARegionSequence::newReader (double sampleRate)
{
    return new Reader (this, sampleRate);
}

/*static*/ void ARARegionSequence::willUpdatePlaybackRegionProperties (
    ARA::PlugIn::PlaybackRegion* region,
    ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> properties)
{
   #if JUCE_DEBUG
    jassert (! stateUpdatePlaybackRegionProperties);
    stateUpdatePlaybackRegionProperties = true;
   #endif

    ARARegionSequence* oldSequence = static_cast<ARARegionSequence*> (region->getRegionSequence());
    ARARegionSequence* newSequence = static_cast<ARARegionSequence*> (ARA::PlugIn::fromRef (properties->regionSequenceRef));
    jassert (newSequence->prevSequenceForNewPlaybackRegion == nullptr);

    newSequence->ref->reset();
    newSequence->prevSequenceForNewPlaybackRegion = oldSequence;

    if (oldSequence != nullptr && oldSequence != newSequence)
    {
        oldSequence->ref->reset();
        auto it = oldSequence->sourceRefCount.find (region->getAudioModification()->getAudioSource());
        --it->second;
        if (it->second == 0)
            oldSequence->sourceRefCount.erase (it);
    }
}

/*static*/ void ARARegionSequence::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* region)
{
   #if JUCE_DEBUG
    jassert (stateUpdatePlaybackRegionProperties);
    stateUpdatePlaybackRegionProperties = false;
   #endif

    ARARegionSequence* newSequence = static_cast<ARARegionSequence*> (region->getRegionSequence());
    ARARegionSequence* oldSequence = newSequence->prevSequenceForNewPlaybackRegion;
    newSequence->prevSequenceForNewPlaybackRegion = nullptr;

    auto* source = region->getAudioModification()->getAudioSource();
    jassert (source != nullptr);

    if (newSequence != oldSequence)
    {
        if (oldSequence != nullptr)
            oldSequence->ref = new Ref (oldSequence);
        ++newSequence->sourceRefCount[source];
    }

    newSequence->ref = new Ref (newSequence);
}

bool ARARegionSequence::isSampleAccessEnabled() const
{
    Ref::ScopedAccess access (ref);
    for (auto& x : sourceRefCount)
        if (! x.first->isSampleAccessEnabled())
            return false;
    return true;
}

ARARegionSequence::Reader::Reader (ARARegionSequence* sequence, double sampleRate_)
    : AudioFormatReader (nullptr, "ARARegionSequenceReader")
    , ref (sequence->ref)
{
    bitsPerSample = 32;
    usesFloatingPointData = true;
    numChannels = 0;
    lengthInSamples = 0;
    sampleRate = sampleRate_;

    Ref::ScopedAccess access (ref);
    jassert (access);
    for (ARA::PlugIn::PlaybackRegion* region : sequence->getPlaybackRegions())
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

        if (sourceReaders.find (source) == sourceReaders.end())
        {
            numChannels = std::max (numChannels, (unsigned int) source->getChannelCount());
            sourceReaders[source] = source->newReader();
        }

        lengthInSamples = std::max (lengthInSamples, region->getEndInPlaybackSamples (sampleRate));
    }
}

ARARegionSequence::Reader::~Reader()
{
    for (auto& x : sourceReaders)
        delete x.second;
}

bool ARARegionSequence::Reader::readSamples (
    int** destSamples,
    int numDestChannels,
    int startOffsetInDestBuffer,
    int64 startSampleInFile,
    int numSamples)
{
    // Clear buffers
    for (int i = 0; i < numDestChannels; ++i)
        if (float* destBuf = (float*) destSamples[i])
            FloatVectorOperations::clear (destBuf + startOffsetInDestBuffer, numSamples);

    Ref::ScopedAccess sequence (ref, true);
    if (! sequence)
        return false;

    if (sampleBuffer.getNumSamples() < numSamples || sampleBuffer.getNumChannels() < numDestChannels)
        sampleBuffer.setSize (numDestChannels, numSamples, false, false, true);

    const double start = (double) startSampleInFile / sampleRate;
    const double stop = (double) (startSampleInFile + (int64) numSamples) / sampleRate;

    // Fill in content from relevant regions
    for (ARA::PlugIn::PlaybackRegion* region : sequence->getPlaybackRegions())
    {
        if (region->getEndInPlaybackTime() <= start || region->getStartInPlaybackTime() >= stop)
            continue;

        const int64 regionStartSample = region->getStartInPlaybackSamples (sampleRate);

        AudioFormatReader* sourceReader = sourceReaders[region->getAudioModification()->getAudioSource()];
        jassert (sourceReader != nullptr);

        const int64 startSampleInRegion = std::max ((int64) 0, startSampleInFile - regionStartSample);
        const int destOffest = (int) std::max ((int64) 0, regionStartSample - startSampleInFile);
        const int numRegionSamples = std::min (
            (int) (region->getDurationInPlaybackSamples (sampleRate) - startSampleInRegion),
            numSamples - destOffest);
        if (! sourceReader->read (
            (int**) sampleBuffer.getArrayOfWritePointers(),
            numDestChannels,
            region->getStartInAudioModificationSamples() + startSampleInRegion,
            numRegionSamples,
            false))
            return false;
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            if (float* destBuf = (float*) destSamples[chan_i])
                FloatVectorOperations::add (
                    destBuf + startOffsetInDestBuffer + destOffest,
                    sampleBuffer.getReadPointer (chan_i), numRegionSamples);
    }

    return true;
}

} // namespace juce
