#include "juce_ARAAudioModification.h"

namespace juce
{

ARAAudioModification::ARAAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef)
: ARA::PlugIn::AudioModification (audioSource, hostRef)
{}

void ARAAudioModification::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyAllPlaybackRegions)
{
    static_cast<ARADocumentController*> (getAudioSource()->getDocument()->getDocumentController())->
                                notifyAudioModificationContentChanged (this, scopeFlags, notifyAllPlaybackRegions);
}

} // namespace juce
