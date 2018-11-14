#include "juce_ARAPlaybackRegion.h"

namespace juce
{

ARAPlaybackRegion::ARAPlaybackRegion (ARA::PlugIn::AudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef)
: ARA::PlugIn::PlaybackRegion (audioModification, hostRef)
{}

void ARAPlaybackRegion::willUpdatePlaybackRegionProperties (ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept
{
    // TODO JUCE_ARA same potential issues as in willDestroyPlaybackRegion(), but it's very unlikely that
    // listeners are going to add/remove themselves from this call.
    listeners.call ([this, &newProperties] (Listener& l) { l.willUpdatePlaybackRegionProperties (this, newProperties); });
}

void ARAPlaybackRegion::didUpdatePlaybackRegionProperties() noexcept
{
    // TODO JUCE_ARA same potential issues as in willDestroyPlaybackRegion(), but it's very unlikely that
    // listeners are going to add/remove themselves from this call.
    listeners.call ([this] (Listener& l) { l.didUpdatePlaybackRegionProperties (this); });
}

void ARAPlaybackRegion::willDestroyPlaybackRegion() noexcept
{
// TODO JUCE_ARA listeners will typically remove themself from this call. In that case, ListenerList may not
// notify some listeners, or call some twice, which seems not acceptable for this use case.
// So we have to roll a custom version, which is not entirely possible due to the lock not being accesible.
// Started a thread in the JUCE forum to figure out what to do:
// https://forum.juce.com/t/listenerlist-issue-need-reliable-notifications-if-adding-removing-listeners-from-within-a-callback/30361

// This is not a generic implementation. It'll work well here due to the context,
// but is not thread safe, and potentially no great performance either...
//  auto lock (listeners.getLock());
    auto listenersCopy (listeners.getListeners());
    for (auto listener : listenersCopy)
    {
        if (listeners.contains (listener))
            listener->willDestroyPlaybackRegion (this);
    }
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
