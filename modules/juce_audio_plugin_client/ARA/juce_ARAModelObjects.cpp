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
        auto regionTimeRange = playbackRegion->getTimeRange (includeHeadAndTail);
        startTime = jmin (startTime, regionTimeRange.getStart());
        endTime = jmax (endTime, regionTimeRange.getEnd());
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
    : ARA::PlugIn::AudioSource (document, hostRef)
{}

void ARAAudioSource::notifyAnalysisProgressStarted()
{
    getDocumentController()->notifyAudioSourceAnalysisProgressStarted (this);
}

void ARAAudioSource::notifyAnalysisProgressUpdated (float progress)
{
    getDocumentController()->notifyAudioSourceAnalysisProgressUpdated (this, progress);
}

void ARAAudioSource::notifyAnalysisProgressCompleted()
{
    getDocumentController()->notifyAudioSourceAnalysisProgressCompleted (this);
}

void ARAAudioSource::notifyContentChanged (ARAContentUpdateScopes scopeFlags)
{
    getDocumentController()->notifyAudioSourceContentChanged (this, scopeFlags);

    notifyListeners ([&] (Listener& l) { l.didUpdateAudioSourceContent (this, scopeFlags); });
}

//==============================================================================

ARAAudioModification::ARAAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARAAudioModification* optionalModificationToClone)
    : ARA::PlugIn::AudioModification (audioSource, hostRef, optionalModificationToClone)
{}

void ARAAudioModification::notifyContentChanged (ARAContentUpdateScopes scopeFlags)
{
    getDocumentController()->notifyAudioModificationContentChanged (this, scopeFlags);

    notifyListeners ([&] (Listener& l) { l.didUpdateAudioModificationContent (this, scopeFlags); });
}

//==============================================================================

ARAPlaybackRegion::ARAPlaybackRegion (ARAAudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef)
    : ARA::PlugIn::PlaybackRegion (audioModification, hostRef)
{}

Range<double> ARAPlaybackRegion::getTimeRange (bool includeHeadAndTail) const
{
    auto startTime = getStartInPlaybackTime();
    auto endTime = getEndInPlaybackTime();
    
    if (includeHeadAndTail)
    {
        ARA::ARATimeDuration headTime {}, tailTime {};
        getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
        startTime -= headTime;
        endTime += tailTime;
    }
    
    return { startTime, endTime };
}

void ARAPlaybackRegion::notifyContentChanged (ARAContentUpdateScopes scopeFlags)
{
    getDocumentController()->notifyPlaybackRegionContentChanged (this, scopeFlags);

    notifyListeners ([&] (Listener& l) { l.didUpdatePlaybackRegionContent (this, scopeFlags); });
}

} // namespace juce
