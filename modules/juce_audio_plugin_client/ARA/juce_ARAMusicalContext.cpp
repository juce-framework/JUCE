#include "juce_ARAMusicalContext.h"

namespace juce
{

ARAMusicalContext::ARAMusicalContext (ARADocument* document, ARA::ARAMusicalContextHostRef hostRef)
: ARA::PlugIn::MusicalContext (document, hostRef)
{}

void ARAMusicalContext::willUpdateMusicalContextProperties (ARAMusicalContext::PropertiesPtr newProperties)
{
    listeners.call ([this, &newProperties] (Listener& l) { l.willUpdateMusicalContextProperties (this, newProperties); });
}

void ARAMusicalContext::didUpdateMusicalContextProperties()
{
    listeners.call ([this] (Listener& l) { l.didUpdateMusicalContextProperties (this); });
}

void ARAMusicalContext::doUpdateMusicalContextContent (const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags)
{
    listeners.call ([this, range, flags] (Listener& l) { l.doUpdateMusicalContextContent (this, range, flags); });
}

void ARAMusicalContext::willDestroyMusicalContext()
{
    // TODO JUCE_ARA 
    // same concerns involving removal as with other listeners
    auto listenersCopy (listeners.getListeners());
    for (auto listener : listenersCopy)
    {
        if (listeners.contains (listener))
            listener->willDestroyMusicalContext (this);
    }
}

void ARAMusicalContext::addListener (Listener * l)
{
    listeners.add (l);
}

void ARAMusicalContext::removeListener (Listener * l)
{
    listeners.remove (l);
}

} // namespace juce
