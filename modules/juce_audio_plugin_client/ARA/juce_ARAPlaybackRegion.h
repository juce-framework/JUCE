#pragma once

#include "juce_ARA_audio_plugin.h"
#include "juce_ARADocumentController.h"

namespace juce
{

class ARAPlaybackRegion : public ARA::PlugIn::PlaybackRegion
{
public:
    ARAPlaybackRegion (ARA::PlugIn::AudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef);

    class Listener
    {
    public:
        virtual ~Listener()  {}

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) {}
        virtual void didUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion) {}
        virtual void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

public:         // to be called by ARADocumentController only
    void willUpdatePlaybackRegionProperties (ARAPlaybackRegion::PropertiesPtr newProperties);
    void didUpdatePlaybackRegionProperties();
    void willDestroyPlaybackRegion();

private:
    ListenerList<Listener> listeners;
};

} // namespace juce
