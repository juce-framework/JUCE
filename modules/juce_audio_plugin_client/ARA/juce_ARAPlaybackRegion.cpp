#include "juce_ARAPlaybackRegion.h"

namespace juce
{

ARAPlaybackRegion::ARAPlaybackRegion (ARA::PlugIn::AudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef)
: ARA::PlugIn::PlaybackRegion (audioModification, hostRef)
{}

void ARAPlaybackRegion::willUpdatePlaybackRegionProperties (ARAPlaybackRegion::PropertiesPtr newProperties)
{
    // TODO JUCE_ARA same potential issues as in willDestroyPlaybackRegion(), but it's very unlikely that
    // listeners are going to add/remove themselves from this call.
    listeners.callExpectingUnregistration ([this, &newProperties] (Listener& l) { l.willUpdatePlaybackRegionProperties (this, newProperties); });
}

void ARAPlaybackRegion::didUpdatePlaybackRegionProperties()
{
    // TODO JUCE_ARA same potential issues as in willDestroyPlaybackRegion(), but it's very unlikely that
    // listeners are going to add/remove themselves from this call.
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.didUpdatePlaybackRegionProperties (this); });
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
