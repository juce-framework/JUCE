#include "juce_ARAPlaybackRegion.h"

namespace juce
{

ARAPlaybackRegion::ARAPlaybackRegion (ARAAudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef)
: ARA::PlugIn::PlaybackRegion (audioModification, hostRef)
{}

} // namespace juce
