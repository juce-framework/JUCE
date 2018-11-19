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
    listeners.call ([this, &newProperties] (Listener& l) { l.willUpdatePlaybackRegionProperties (this, newProperties); });
}

void ARAPlaybackRegion::didUpdatePlaybackRegionProperties()
{
    // TODO JUCE_ARA same potential issues as in willDestroyPlaybackRegion(), but it's very unlikely that
    // listeners are going to add/remove themselves from this call.
    listeners.call ([this] (Listener& l) { l.didUpdatePlaybackRegionProperties (this); });
}

void ARAPlaybackRegion::willDestroyPlaybackRegion()
{
// TODO JUCE_ARA listeners will typically remove themself from this call. In that case, ListenerList may not
// notify some listeners, or call some twice, which seems not acceptable for this use case.
// So we have to roll a custom version, which is not entirely possible due to the lock not being accesible.
// Started a thread in the JUCE forum to figure out what to do:
// https://forum.juce.com/t/listenerlist-issue-need-reliable-notifications-if-adding-removing-listeners-from-within-a-callback/30361
// @jj for the time being, I suggest we edit the ListenerList class and add some
//     callExpectingUnregistration () variant which we use in all the willDestroy... ()
//     Unsure about the other calls, I tend to keeping call () there for performance reasons...?

    if (listeners.isEmpty())
        return;

    auto& listenersArray = listeners.getListeners();

// not thread safe without the lock, but currently there's no outside acceess to it...!
//  auto lock (listenersArray.getLock());

	if (listenersArray.size() == 1)
	{
        // if there's but one listener, we can skip copying the array
		listenersArray.getFirst()->willDestroyPlaybackRegion (this);
	}
	else
	{
		auto listenersCopy (listenersArray);
		for (auto listener : listenersCopy)
		{
			if (listeners.contains (listener))
				listener->willDestroyPlaybackRegion (this);
		}
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
