#pragma once

#include "juce_ARAAudioSource.h"

namespace juce
{

class ARAAudioSource::Reader : public AudioFormatReader
{
public:
    Reader (ARAAudioSource*);
    ~Reader();

    void createHostAudioReaderForSource(ARA::PlugIn::AudioSource* audioSource);
    void invalidate();

    bool readSamples (
        int** destSamples,
        int numDestChannels,
        int startOffsetInDestBuffer,
        int64 startSampleInFile,
        int numSamples) override;

private:
    Ref::Ptr ref;
    std::vector<void*> tmpPtrs;

    // When readSamples is not reading all channels,
    // we still need to provide pointers to all channels to the ARA read call.
    // So we'll read the other channels into this dummy buffer.
    std::vector<float> dummyBuffer;

    std::unique_ptr<ARA::PlugIn::HostAudioReader> araHostReader;
};

ARAAudioSource::ARAAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef)
    : ARA::PlugIn::AudioSource (document, hostRef)
    , ref (new Ref (this))
{
}

ARAAudioSource::~ARAAudioSource()
{
    invalidateReaders();
}

void ARAAudioSource::invalidateReaders()
{
    ScopedWriteLock l (ref->lock);
    for (auto& reader : readers)
        reader->invalidate();
    readers.clear();
    ref->reset();
}

AudioFormatReader* ARAAudioSource::newReader()
{
    return new Reader (this);
}

void ARAAudioSource::willUpdateProperties()
{
#if JUCE_DEBUG
    jassert (! stateUpdateProperties);
    stateUpdateProperties = true;
#endif

    invalidateReaders();
}

void ARAAudioSource::didUpdateProperties()
{
#if JUCE_DEBUG
    jassert (stateUpdateProperties);
    stateUpdateProperties = false;
#endif

    ref = new Ref (this);
}

void ARAAudioSource::willEnableSamplesAccess (bool enable)
{
#if JUCE_DEBUG
    jassert (! stateEnableSamplesAccess);
    stateEnableSamplesAccess = true;
#endif

    ref->lock.enterWrite();
    if (! enable)
        for (auto& reader : readers)
            reader->invalidate();
}

void ARAAudioSource::didEnableSamplesAccess (bool enable)
{
#if JUCE_DEBUG
    jassert (stateEnableSamplesAccess);
    stateEnableSamplesAccess = false;
#endif

    if (enable)
        for (auto& reader : readers)
            reader->createHostAudioReaderForSource(this);
    ref->lock.exitWrite();
}

ARAAudioSource::Reader::Reader (ARAAudioSource* source)
    : AudioFormatReader (nullptr, "ARAAudioSourceReader")
    , ref (source->ref)
{
    if (source->isSampleAccessEnabled())
        araHostReader.reset (new ARA::PlugIn::HostAudioReader (source));
    bitsPerSample = 32;
    usesFloatingPointData = true;
    sampleRate = source->getSampleRate();
    numChannels = source->getChannelCount();
    lengthInSamples = source->getSampleCount();
    tmpPtrs.resize (numChannels);
    ScopedWriteLock l (ref->lock);
    source->readers.push_back (this);
}

ARAAudioSource::Reader::~Reader()
{
    if (Ref::ScopedAccess source{ ref })
    {
        ScopedWriteLock l (ref->lock);
        source->readers.erase (std::find (source->readers.begin(), source->readers.end(), this));
    }
}

void ARAAudioSource::Reader::createHostAudioReaderForSource(ARA::PlugIn::AudioSource* audioSource)
{
    // TODO should we assert these conditions instead of treating them as a case for invalidation?
    if (audioSource == nullptr || audioSource->isSampleAccessEnabled() == false)
        invalidate();
    else
        araHostReader.reset(new ARA::PlugIn::HostAudioReader(audioSource));
}

void ARAAudioSource::Reader::invalidate()
{
    araHostReader.reset();
}

bool ARAAudioSource::Reader::readSamples (
    int** destSamples,
    int numDestChannels,
    int startOffsetInDestBuffer,
    int64 startSampleInFile,
    int numSamples)
{
    Ref::ScopedAccess source (ref, true);
    if (! source || araHostReader == nullptr)
        return false;
    for (int chan_i = 0; chan_i < (int) tmpPtrs.size(); ++chan_i)
        if (chan_i < numDestChannels && destSamples[chan_i] != nullptr)
            tmpPtrs[chan_i] = (void*) (destSamples[chan_i] + startOffsetInDestBuffer);
        else
        {
            if (numSamples > (int) dummyBuffer.size())
                dummyBuffer.resize (numSamples);
            tmpPtrs[chan_i] = (void*) dummyBuffer.data();
        }
    return araHostReader->readAudioSamples (startSampleInFile, numSamples, tmpPtrs.data());
}

} // namespace juce
