#pragma once

#include "juce_ARA_audio_plugin.h"
#include "juce_ARADocumentController.h"
#include "juce_SafeRef.h"

namespace juce
{

class ARAPlaybackRegion : public ARA::PlugIn::PlaybackRegion
{
public:
    ARAPlaybackRegion (ARA::PlugIn::AudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef);

    void willUpdatePlaybackRegionProperties (ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept;
    void didUpdatePlaybackRegionProperties () noexcept;
    void willDestroyPlaybackRegion () noexcept;

    class Listener
    {
    public:
        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN

        virtual void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARA::PlugIn::PropertiesPtr<ARA::ARAPlaybackRegionProperties> newProperties) noexcept {}
        virtual void didUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion) noexcept {}
        virtual void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept {}

        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

private:
    std::vector<Listener*> listeners;
};

} // namespace juce
