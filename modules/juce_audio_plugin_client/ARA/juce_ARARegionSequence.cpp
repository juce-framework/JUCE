#include "juce_ARARegionSequence.h"

namespace juce
{

ARARegionSequence::ARARegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef)
: ARA::PlugIn::RegionSequence (document, hostRef)
{}

void ARARegionSequence::willUpdateRegionSequenceProperties (ARARegionSequence::PropertiesPtr newProperties)
{
    listeners.call ([this, &newProperties] (Listener& l) { l.willUpdateRegionSequenceProperties (this, newProperties); });
}

void ARARegionSequence::didUpdateRegionSequenceProperties()
{
    listeners.call ([this] (Listener& l) { l.didUpdateRegionSequenceProperties (this); });
}

void ARARegionSequence::willDestroyRegionSequence()
{
    // TODO JUCE_ARA 
    // same concerns involving removal as with other listeners
    auto listenersCopy (listeners.getListeners());
    for (auto listener : listenersCopy)
    {
        if (listeners.contains (listener))
            listener->willDestroyRegionSequence (this);
    }
}

void ARARegionSequence::didAddPlaybackRegionToRegionSequence (ARAPlaybackRegion* playbackRegion)
{
    listeners.call ([this, playbackRegion] (Listener& l) { l.didAddPlaybackRegionToRegionSequence (this, playbackRegion); });
}

void ARARegionSequence::willRemovePlaybackRegionFromRegionSequence (ARAPlaybackRegion* playbackRegion)
{
    listeners.call ([this, playbackRegion] (Listener& l) { l.willRemovePlaybackRegionFromRegionSequence (this, playbackRegion); });
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
