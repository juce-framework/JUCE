#include "juce_ARAAudioSource.h"

namespace juce
{

ARAAudioSource::ARAAudioSource (ARADocument* document, ARA::ARAAudioSourceHostRef hostRef)
: ARA::PlugIn::AudioSource(document, hostRef)
{}

} // namespace juce
