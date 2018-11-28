#include "juce_ARAAudioModification.h"

namespace juce
{

ARAAudioModification::ARAAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef)
: ARA::PlugIn::AudioModification (audioSource, hostRef)
{}

void ARAAudioModification::willUpdateAudioModificationProperties (PropertiesPtr newProperties)
{
    listeners.callExpectingUnregistration ([this, &newProperties] (Listener& l) { l.willUpdateAudioModificationProperties (this, newProperties); });
}

void ARAAudioModification::didUpdateAudioModificationProperties()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.didUpdateAudioModificationProperties (this); });
}

void ARAAudioModification::didUpdateAudioModificationContent (ARAContentUpdateScopes scopeFlags)
{
    listeners.callExpectingUnregistration ([this, scopeFlags] (Listener& l) { l.didUpdateAudioModificationContent (this, scopeFlags); });
}

void ARAAudioModification::doDeactivateAudioModificationForUndoHistory (bool deactivate)
{
    listeners.callExpectingUnregistration ([this, deactivate] (Listener& l) { l.doDeactivateAudioModificationForUndoHistory (this, deactivate); });
}

void ARAAudioModification::didAddPlaybackRegion (ARAPlaybackRegion* playbackRegion)
{
    listeners.callExpectingUnregistration ([this, playbackRegion] (Listener& l) { l.didAddPlaybackRegion(this, playbackRegion); });
}

void ARAAudioModification::willDestroyAudioModification()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.willDestroyAudioModification (this); });
}

void ARAAudioModification::addListener (Listener * l)
{
    listeners.add (l);
}

void ARAAudioModification::removeListener (Listener * l)
{
    listeners.remove (l);
}

} // namespace juce
