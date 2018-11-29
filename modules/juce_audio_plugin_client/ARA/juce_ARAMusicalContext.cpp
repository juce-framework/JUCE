#include "juce_ARAMusicalContext.h"

namespace juce
{

ARAMusicalContext::ARAMusicalContext (ARADocument* document, ARA::ARAMusicalContextHostRef hostRef)
: ARA::PlugIn::MusicalContext (document, hostRef)
{}

} // namespace juce
