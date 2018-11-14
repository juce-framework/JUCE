#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAPlaybackRegion;

class ARARegionSequence : public ARA::PlugIn::RegionSequence
{
public:
    ARARegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef);

    // Is sample access enabled in all audio sources in sequence?
    bool isSampleAccessEnabled() const;
    
    void willUpdateRegionSequenceProperties (ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties) noexcept;
    void didUpdateRegionSequenceProperties () noexcept;
    void willAddPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void didAddPlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void willRemovePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void didRemovePlaybackRegion (ARAPlaybackRegion* playbackRegion) noexcept;
    void willDestroyRegionSequence () noexcept;

    class Listener
    {
    public:
        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN

        virtual ~Listener () {}

        virtual void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence, ARA::PlugIn::PropertiesPtr<ARA::ARARegionSequenceProperties> newProperties) noexcept {}
        virtual void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) noexcept {}
        virtual void willDestroyRegionSequence (ARARegionSequence* regionSequence) noexcept {}
        virtual void willAddPlaybackRegion (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) noexcept {}
        virtual void didAddPlaybackRegion (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) noexcept {}
        virtual void willRemovePlaybackRegion (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) noexcept {}
        virtual void didRemovePlaybackRegion (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) noexcept {}

        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

private:
    ListenerList<Listener> listeners;
};

} // namespace juce
