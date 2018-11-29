#include "juce_ARAAudioSource.h"

namespace juce
{

ARAAudioSource::ARAAudioSource (ARADocument* document, ARA::ARAAudioSourceHostRef hostRef)
: ARA::PlugIn::AudioSource(document, hostRef)
{}

void ARAAudioSource::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyAllAudioModificationsAndPlaybackRegions)
{
    static_cast<ARADocumentController*> (getDocument()->getDocumentController())->
                                notifyAudioSourceContentChanged (this, scopeFlags, notifyAllAudioModificationsAndPlaybackRegions);
}

} // namespace juce
