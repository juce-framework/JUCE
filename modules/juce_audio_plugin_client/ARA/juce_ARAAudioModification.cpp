#include "juce_ARAAudioModification.h"

namespace juce
{

ARAAudioModification::ARAAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef)
: ARA::PlugIn::AudioModification (audioSource, hostRef)
{}

void ARAAudioModification::willUpdateAudioModificationProperties (PropertiesPtr newProperties) noexcept
{
    listeners.call ([this, &newProperties] (Listener& l) { l.willUpdateAudioModificationProperties (this, newProperties); });
}

void ARAAudioModification::didUpdateAudioModificationProperties () noexcept
{
    listeners.call ([this] (Listener& l) { l.didUpdateAudioModificationProperties (this); });
}

void ARAAudioModification::doDeactivateAudioModificationForUndoHistory (bool deactivate) noexcept
{
    listeners.call ([this, deactivate] (Listener& l) { l.doDeactivateAudioModificationForUndoHistory (this, deactivate); });
}

void ARAAudioModification::willDestroyAudioModification () noexcept
{
    // TODO JUCE_ARA 
    // same concerns involving removal as with other listeners
    auto listenersCopy (listeners.getListeners ());
    for (auto listener : listenersCopy)
    {
        if (listeners.contains (listener))
            listener->willDestroyAudioModification (this);
    }
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
