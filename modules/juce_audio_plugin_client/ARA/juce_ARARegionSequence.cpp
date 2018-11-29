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

} // namespace juce
