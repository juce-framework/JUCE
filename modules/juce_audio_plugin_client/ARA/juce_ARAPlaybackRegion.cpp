#include "juce_ARAPlaybackRegion.h"

namespace juce
{

ARAPlaybackRegion::ARAPlaybackRegion (ARA::PlugIn::AudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef)
: ARA::PlugIn::PlaybackRegion (audioModification, hostRef)
{}

void ARAPlaybackRegion::willUpdatePlaybackRegionProperties (ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept
{
    for (Listener* l : listeners)
        l->willUpdatePlaybackRegionProperties (this, newProperties);
}

void ARAPlaybackRegion::didUpdatePlaybackRegionProperties () noexcept
{
    for (Listener* l : listeners)
        l->didUpdatePlaybackRegionProperties (this);
}

void ARAPlaybackRegion::willDestroyPlaybackRegion () noexcept
{
    for (Listener* l : listeners)
        l->willDestroyPlaybackRegion (this);
}

void ARAPlaybackRegion::addListener (Listener * l) 
{ 
    listeners.push_back (l); 
}

void ARAPlaybackRegion::removeListener (Listener * l) 
{ 
    ::find_erase (listeners, l); 
}

} // namespace juce
