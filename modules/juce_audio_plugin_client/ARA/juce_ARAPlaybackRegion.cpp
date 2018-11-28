#include "juce_ARAPlaybackRegion.h"

namespace juce
{

ARAPlaybackRegion::ARAPlaybackRegion (ARAAudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef)
: ARA::PlugIn::PlaybackRegion (audioModification, hostRef)
{}

void ARAPlaybackRegion::willUpdatePlaybackRegionProperties (ARAPlaybackRegion::PropertiesPtr newProperties)
{
    listeners.callExpectingUnregistration ([this, &newProperties] (Listener& l) { l.willUpdatePlaybackRegionProperties (this, newProperties); });
}

void ARAPlaybackRegion::didUpdatePlaybackRegionProperties()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.didUpdatePlaybackRegionProperties (this); });
}

void ARAPlaybackRegion::didUpdatePlaybackRegionContent (ARAContentUpdateScopes scopeFlags)
{
    listeners.callExpectingUnregistration ([this, scopeFlags] (Listener& l) { l.didUpdatePlaybackRegionContent (this, scopeFlags); });
}

void ARAPlaybackRegion::willDestroyPlaybackRegion()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.willDestroyPlaybackRegion (this); });
}

void ARAPlaybackRegion::addListener (Listener * l) 
{ 
    listeners.add (l);
}

void ARAPlaybackRegion::removeListener (Listener * l) 
{ 
    listeners.remove (l);
}

} // namespace juce
