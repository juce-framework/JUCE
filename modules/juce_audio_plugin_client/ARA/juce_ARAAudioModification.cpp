#include "juce_ARAAudioModification.h"

namespace juce
{

ARAAudioModification::ARAAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef)
: ARA::PlugIn::AudioModification (audioSource, hostRef)
{}


} // namespace juce
