#include "juce_ARAPlaybackRegion.h"

namespace juce
{

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
    static_cast<ARADocumentController*> (getAudioModification()->getAudioSource()->getDocument()->getDocumentController())->
                            notifyPlaybackRegionContentChanged (this, scopeFlags);
}

} // namespace juce
