#pragma once

#include "ARARegionSequence.h"
#include "ARAAudioSource.h"

#if JUCE_DEBUG
bool ARARegionSequence::stateUpdatePlaybackRegionProperties = false;
#endif

namespace juce
{
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
        Ref::Ptr ref_;
        std::map<ARA::PlugIn::AudioSource*, AudioFormatReader*> sourceReaders_;
        AudioSampleBuffer buf_;
    };

    ARARegionSequence::ARARegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef)
        : ARA::PlugIn::RegionSequence (document, hostRef)
    {
        ref_ = new Ref (this);
        prevSequenceForNewPlaybackRegion_ = nullptr;
    }

    ARARegionSequence::~ARARegionSequence()
    {
        ref_->reset();
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
        jassert (!stateUpdatePlaybackRegionProperties);
        stateUpdatePlaybackRegionProperties = true;
#endif

        ARARegionSequence* oldSequence = dynamic_cast<ARARegionSequence*> (region->getRegionSequence());
        ARARegionSequence* newSequence = dynamic_cast<ARARegionSequence*> (ARA::PlugIn::fromRef (properties->regionSequenceRef));
        jassert (newSequence != nullptr);
        jassert (newSequence->prevSequenceForNewPlaybackRegion_ == nullptr);

        newSequence->ref_->reset();
        newSequence->prevSequenceForNewPlaybackRegion_ = oldSequence;

        if (oldSequence != nullptr && oldSequence != newSequence)
        {
            oldSequence->ref_->reset();
            auto it = oldSequence->sourceRefCount_.find (region->getAudioModification()->getAudioSource());
            --it->second;
            if (it->second == 0)
                oldSequence->sourceRefCount_.erase (it);
        }
    }

    /*static*/ void ARARegionSequence::didUpdatePlaybackRegionProperties (ARA::PlugIn::PlaybackRegion* region)
    {
#if JUCE_DEBUG
        jassert (stateUpdatePlaybackRegionProperties);
        stateUpdatePlaybackRegionProperties = false;
#endif

        ARARegionSequence* newSequence = dynamic_cast<ARARegionSequence*> (region->getRegionSequence());
        jassert (newSequence != nullptr);

        ARARegionSequence* oldSequence = newSequence->prevSequenceForNewPlaybackRegion_;
        newSequence->prevSequenceForNewPlaybackRegion_ = nullptr;

        auto* source = region->getAudioModification()->getAudioSource();
        jassert (source != nullptr);

        if (newSequence != oldSequence)
        {
            if (oldSequence != nullptr)
                oldSequence->ref_ = new Ref (oldSequence);
            ++newSequence->sourceRefCount_[source];
        }

        newSequence->ref_ = new Ref (newSequence);
    }

    bool ARARegionSequence::isSampleAccessEnabled() const
    {
        Ref::ScopedAccess access (ref_);
        for (auto& x : sourceRefCount_)
            if (!x.first->isSampleAccessEnabled())
                return false;
        return true;
    }

    ARARegionSequence::Reader::Reader (ARARegionSequence* sequence, double sampleRate_)
        : AudioFormatReader (nullptr, "ARARegionSequenceReader")
        , ref_ (sequence->ref_)
    {
        bitsPerSample = 32;
        usesFloatingPointData = true;
        numChannels = 0;
        lengthInSamples = 0;
        sampleRate = sampleRate_;

        Ref::ScopedAccess access (ref_);
        jassert (access);
        for (ARA::PlugIn::PlaybackRegion* region : sequence->getPlaybackRegions())
        {
            ARA::PlugIn::AudioModification* modification = region->getAudioModification();
            jassert (modification != nullptr);
            ARAAudioSource* source = dynamic_cast<ARAAudioSource*> (modification->getAudioSource());
            jassert (source != nullptr);

            if (sampleRate == 0.0)
                sampleRate = source->getSampleRate();

            if (sampleRate != source->getSampleRate())
            {
                // Skip regions with mis-matching sample-rates!
                continue;
            }

            if (sourceReaders_.find (source) == sourceReaders_.end())
            {
                numChannels = std::max (numChannels, (unsigned int) source->getChannelCount());
                sourceReaders_[source] = source->newReader();
            }

            lengthInSamples = std::max (lengthInSamples, region->getEndInPlaybackSamples (sampleRate));
        }
    }

    ARARegionSequence::Reader::~Reader()
    {
        for (auto& x : sourceReaders_)
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
            if (float* buf = (float*) destSamples[i])
                FloatVectorOperations::clear (buf + startOffsetInDestBuffer, numSamples);

        Ref::ScopedAccess sequence (ref_, true);
        if (!sequence)
            return false;

        if (buf_.getNumSamples() < numSamples || buf_.getNumChannels() < numDestChannels)
            buf_.setSize (numDestChannels, numSamples, false, false, true);

        const double start = (double) startSampleInFile / sampleRate;
        const double stop = (double) (startSampleInFile + (int64) numSamples) / sampleRate;

        // Fill in content from relevant regions
        for (ARA::PlugIn::PlaybackRegion* region : sequence->getPlaybackRegions())
        {
            if (region->getEndInPlaybackTime() <= start || region->getStartInPlaybackTime() >= stop)
                continue;

            const int64 regionStartSample = region->getStartInPlaybackSamples (sampleRate);

            AudioFormatReader* sourceReader = sourceReaders_[region->getAudioModification()->getAudioSource()];
            jassert (sourceReader != nullptr);

            const int64 startSampleInRegion = std::max ((int64) 0, startSampleInFile - regionStartSample);
            const int destOffest = (int) std::max ((int64) 0, regionStartSample - startSampleInFile);
            const int numRegionSamples = std::min (
                (int) (region->getDurationInPlaybackSamples (sampleRate) - startSampleInRegion),
                numSamples - destOffest);
            if (!sourceReader->read (
                (int**) buf_.getArrayOfWritePointers(),
                numDestChannels,
                region->getStartInAudioModificationSamples() + startSampleInRegion,
                numRegionSamples,
                false))
                return false;
            for (int chan_i = 0; chan_i < numDestChannels; ++chan_i)
                if (float* buf = (float*) destSamples[chan_i])
                    FloatVectorOperations::add (
                        buf + startOffsetInDestBuffer + destOffest,
                        buf_.getReadPointer (chan_i), numRegionSamples);
        }

        return true;
    }
}
