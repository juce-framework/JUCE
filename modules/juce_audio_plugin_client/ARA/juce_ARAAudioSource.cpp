#include "juce_ARAAudioSource.h"

namespace juce
{

ARAAudioSource::ARAAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef)
: ARA::PlugIn::AudioSource(document, hostRef)
{}

void ARAAudioSource::willUpdateAudioSourceProperties (PropertiesPtr newProperties)
{
    listeners.callExpectingUnregistration ([this, &newProperties] (Listener& l) { l.willUpdateAudioSourceProperties (this, newProperties); });
}

void ARAAudioSource::didUpdateAudioSourceProperties()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.didUpdateAudioSourceProperties (this); });
}

void ARAAudioSource::doUpdateAudioSourceContent (const ARA::ARAContentTimeRange* range, ARAContentUpdateScopes scopeFlags)
{
    listeners.callExpectingUnregistration ([this, range, scopeFlags] (Listener& l) { l.doUpdateAudioSourceContent (this, range, scopeFlags); });
}

void ARAAudioSource::willEnableAudioSourceSamplesAccess (bool enable)
{
    listeners.callExpectingUnregistration ([this, enable] (Listener& l) { l.willEnableAudioSourceSamplesAccess (this, enable); });
}

void ARAAudioSource::didEnableAudioSourceSamplesAccess (bool enable)
{
    listeners.callExpectingUnregistration ([this, enable] (Listener& l) { l.didEnableAudioSourceSamplesAccess (this, enable); });
}

void ARAAudioSource::doDeactivateAudioSourceForUndoHistory (bool deactivate)
{
    listeners.callExpectingUnregistration ([this, deactivate] (Listener& l) { l.doDeactivateAudioSourceForUndoHistory (this, deactivate); });
}

void ARAAudioSource::willDestroyAudioSource()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.willDestroyAudioSource (this); });
}

void ARAAudioSource::addListener (Listener * l)
{
    listeners.add (l);
}

void ARAAudioSource::removeListener (Listener * l)
{
    listeners.remove (l);
}

} // namespace juce
