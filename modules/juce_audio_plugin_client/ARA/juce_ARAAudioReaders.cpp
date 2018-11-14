#include "juce_ARAAudioReaders.h"

namespace juce
{

ARAAudioSourceReader::ARAAudioSourceReader (ARA::PlugIn::AudioSource* source, bool use64BitSamples)
    : AudioFormatReader (nullptr, "ARAAudioSourceReader")
{
    bitsPerSample = use64BitSamples ? 64 : 32;
    usesFloatingPointData = true;
    sampleRate = source->getSampleRate ();
    numChannels = source->getChannelCount ();
    lengthInSamples = source->getSampleCount ();
    tmpPtrs.resize (numChannels);
    audioSourceBeingRead = static_cast<ARAAudioSource*> (source);

    audioSourceBeingRead->addListener (this);

    if (audioSourceBeingRead->isSampleAccessEnabled ())
        recreate ();
}

ARAAudioSourceReader::~ARAAudioSourceReader ()
{
    // TODO JUCE_ARA
    // should we do this before the lock? after unlock?
    audioSourceBeingRead->removeListener (this);

    ScopedWriteLock l (lock);
    invalidate ();
}

void ARAAudioSourceReader::willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) noexcept
{
    if (audioSource != audioSourceBeingRead)
        return;

    // unlocked in didEnableAudioSourceSamplesAccess
    lock.enterWrite ();

    // invalidate our reader if sample access is disabled
    if (enable == false)
        invalidate ();
}

void ARAAudioSourceReader::didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) noexcept
{
    // following the invalidation above we can recreate any readers
    // we had before access was disabled
    if (audioSource != audioSourceBeingRead)
        return;

    // recreate our reader if sample access is enabled
    if (enable)
        recreate ();

    lock.exitWrite ();
}

void ARAAudioSourceReader::willDestroyAudioSource (ARAAudioSource* audioSource) noexcept
{
    if (audioSource != audioSourceBeingRead)
        return;

    // TODO JUCE_ARA
    // should this ever happen? ideally someone delete us instead...
    // jassertfalse;
    ScopedWriteLock scopedLock (lock);
    invalidate ();
    audioSourceBeingRead = nullptr;
}

void ARAAudioSourceReader::doUpdateAudioSourceContent (ARAAudioSource* audioSource, const ARA::ARAContentTimeRange* /*range*/, ARA::ARAContentUpdateFlags flags) noexcept
{
    if (audioSource != audioSourceBeingRead)
        return;

    // don't invalidate if the audio signal is unchanged
    if ((flags & ARA::kARAContentUpdateSignalScopeRemainsUnchanged) != 0)
        return;

    ScopedWriteLock scopedLock (lock);
    invalidate ();
}

void ARAAudioSourceReader::recreate ()
{
    // TODO JUCE_ARA
    // it shouldnt' be possible for araHostReader to contain data at this point, 
    // but should we assert that?
    jassert (audioSourceBeingRead->isSampleAccessEnabled ());
    araHostReader.reset (new ARA::PlugIn::HostAudioReader (audioSourceBeingRead));
}

void ARAAudioSourceReader::invalidate ()
{
    araHostReader.reset ();
}

bool ARAAudioSourceReader::readSamples (
    int** destSamples,
    int numDestChannels,
    int startOffsetInDestBuffer,
    int64 startSampleInFile,
    int numSamples)
{
    // If we can't enter the lock or we don't have a reader, zero samples and return false
    if (!lock.tryEnterRead () || araHostReader == nullptr)
    {
        for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
            FloatVectorOperations::clear ((float *) destSamples[chan_i], numSamples);
        return false;
    }

    jassert (audioSourceBeingRead != nullptr);

    for (int chan_i = 0; chan_i < (int) tmpPtrs.size (); ++chan_i)
        if (chan_i < numDestChannels && destSamples[chan_i] != nullptr)
            tmpPtrs[chan_i] = (void*) (destSamples[chan_i] + startOffsetInDestBuffer);
        else
        {
            if (numSamples > (int) dummyBuffer.size ())
                dummyBuffer.resize (numSamples);
            tmpPtrs[chan_i] = (void*) dummyBuffer.data ();
        }

    bool success = araHostReader->readAudioSamples (startSampleInFile, numSamples, tmpPtrs.data ());
    lock.exitRead ();
    return success;
}

}