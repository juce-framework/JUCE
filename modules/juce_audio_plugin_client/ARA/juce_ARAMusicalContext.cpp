#include "juce_ARAMusicalContext.h"

namespace juce
{

ARAMusicalContext::ARAMusicalContext (ARADocument* document, ARA::ARAMusicalContextHostRef hostRef)
: ARA::PlugIn::MusicalContext (document, hostRef)
{}

void ARAMusicalContext::willUpdateMusicalContextProperties (ARAMusicalContext::PropertiesPtr newProperties)
{
    listeners.callExpectingUnregistration ([this, &newProperties] (Listener& l) { l.willUpdateMusicalContextProperties (this, newProperties); });
}

void ARAMusicalContext::didUpdateMusicalContextProperties()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.didUpdateMusicalContextProperties (this); });
}

void ARAMusicalContext::didUpdateMusicalContextContent (ARAContentUpdateScopes scopeFlags)
{
    listeners.callExpectingUnregistration ([this, scopeFlags] (Listener& l) { l.didUpdateMusicalContextContent (this, scopeFlags); });
}

void ARAMusicalContext::willDestroyMusicalContext()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.willDestroyMusicalContext (this); });
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
