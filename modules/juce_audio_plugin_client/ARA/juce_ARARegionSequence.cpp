#include "juce_ARARegionSequence.h"

namespace juce
{

ARARegionSequence::ARARegionSequence (ARADocument* document, ARA::ARARegionSequenceHostRef hostRef)
: ARA::PlugIn::RegionSequence (document, hostRef)
{}

double ARARegionSequence::getCommonSampleRate()
{
    double commonSampleRate = 0.0;
    for (auto playbackRegion : getPlaybackRegions())
    {
        double sampleRate = playbackRegion->getAudioModification()->getAudioSource()->getSampleRate();
        if (commonSampleRate == 0.0)
            commonSampleRate = sampleRate;
        if (commonSampleRate != sampleRate)
            return 0.0;
    }
    return commonSampleRate;
}

void ARARegionSequence::willUpdateRegionSequenceProperties (ARARegionSequence::PropertiesPtr newProperties)
{
    listeners.callExpectingUnregistration ([this, &newProperties] (Listener& l) { l.willUpdateRegionSequenceProperties (this, newProperties); });
}

void ARARegionSequence::didUpdateRegionSequenceProperties()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.didUpdateRegionSequenceProperties (this); });
}

void ARARegionSequence::willDestroyRegionSequence()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.willDestroyRegionSequence (this); });
}

void ARARegionSequence::didAddPlaybackRegionToRegionSequence (ARAPlaybackRegion* playbackRegion)
{
    listeners.callExpectingUnregistration ([this, playbackRegion] (Listener& l) { l.didAddPlaybackRegionToRegionSequence (this, playbackRegion); });
}

void ARARegionSequence::willRemovePlaybackRegionFromRegionSequence (ARAPlaybackRegion* playbackRegion)
{
    listeners.callExpectingUnregistration ([this, playbackRegion] (Listener& l) { l.willRemovePlaybackRegionFromRegionSequence (this, playbackRegion); });
}

void ARARegionSequence::addListener (Listener * l)
{
    listeners.add (l);
}

void ARARegionSequence::removeListener (Listener * l)
{
    listeners.remove (l);
}

} // namespace juce
