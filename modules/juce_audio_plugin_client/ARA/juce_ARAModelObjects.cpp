#include "juce_ARAModelObjects.h"

namespace juce
{

//==============================================================================

ARADocument::ARADocument (ARADocumentController* documentController)
    : ARA::PlugIn::Document (documentController)
{}

//==============================================================================

ARAMusicalContext::ARAMusicalContext (ARADocument* document, ARA::ARAMusicalContextHostRef hostRef)
    : ARA::PlugIn::MusicalContext (document, hostRef)
{}

//==============================================================================

ARARegionSequence::ARARegionSequence (ARADocument* document, ARA::ARARegionSequenceHostRef hostRef)
    : ARA::PlugIn::RegionSequence (document, hostRef)
{}

Range<double> ARARegionSequence::getTimeRange (bool includeHeadAndTail) const
{
    if (getPlaybackRegions().empty())
        return {};

    double startTime = std::numeric_limits<double>::max();
    double endTime = std::numeric_limits<double>::lowest();
    for (auto playbackRegion : getPlaybackRegions<ARAPlaybackRegion>())
    {
        double regionStartTime = playbackRegion->getStartInPlaybackTime();
        double regionEndTime = playbackRegion->getEndInPlaybackTime();

        if (includeHeadAndTail)
        {
            regionStartTime -= playbackRegion->getHeadTime();
            regionEndTime += playbackRegion->getTailTime();
        }

        startTime = jmin (startTime, regionStartTime);
        endTime = jmax (endTime, regionEndTime);
    }

    return { startTime, endTime };
}

double ARARegionSequence::getCommonSampleRate() const
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

//==============================================================================

ARAAudioSource::ARAAudioSource (ARADocument* document, ARA::ARAAudioSourceHostRef hostRef)
    : ARA::PlugIn::AudioSource(document, hostRef)
{}

void ARAAudioSource::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyAllAudioModificationsAndPlaybackRegions)
{
    getDocument()->getDocumentController<ARADocumentController>()->
                                notifyAudioSourceContentChanged (this, scopeFlags, notifyAllAudioModificationsAndPlaybackRegions);
}

//==============================================================================

ARAAudioModification::ARAAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef)
    : ARA::PlugIn::AudioModification (audioSource, hostRef)
{}

void ARAAudioModification::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyAllPlaybackRegions)
{
    getAudioSource()->getDocument()->getDocumentController<ARADocumentController>()->
                                notifyAudioModificationContentChanged (this, scopeFlags, notifyAllPlaybackRegions);
}

//==============================================================================

ARAPlaybackRegion::ARAPlaybackRegion (ARAAudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef)
    : ARA::PlugIn::PlaybackRegion (audioModification, hostRef)
{}

void ARAPlaybackRegion::setHeadTime (double newHeadTime)
{
    headTime = newHeadTime;
    notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected());
}

void ARAPlaybackRegion::setTailTime (double newTailTime)
{
    tailTime = newTailTime;
    notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected());
}

void ARAPlaybackRegion::setHeadAndTailTime (double newHeadTime, double newTailTime)
{
    headTime = newHeadTime;
    tailTime = newTailTime;
    notifyContentChanged (ARAContentUpdateScopes::samplesAreAffected());
}

void ARAPlaybackRegion::notifyContentChanged (ARAContentUpdateScopes scopeFlags)
{
    getAudioModification()->getAudioSource()->getDocument()->getDocumentController<ARADocumentController>()->
                            notifyPlaybackRegionContentChanged (this, scopeFlags);
}

} // namespace juce
