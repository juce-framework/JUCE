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
    
    void willUpdateRegionSequenceProperties (ARARegionSequence::PropertiesPtr newProperties) noexcept;
    void didUpdateRegionSequenceProperties () noexcept;
    void willDestroyRegionSequence () noexcept;
    void willRemovePlaybackRegionFromRegionSequence (ARAPlaybackRegion* playbackRegion) noexcept;
    void didAddPlaybackRegionToRegionSequence (ARAPlaybackRegion* playbackRegion) noexcept;

    class Listener
    {
    public:
        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN

        virtual ~Listener () {}

        virtual void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) noexcept {}
        virtual void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) noexcept {}
        virtual void willDestroyRegionSequence (ARARegionSequence* regionSequence) noexcept {}
        virtual void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) noexcept {}
        virtual void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) noexcept {}

        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

private:
    ListenerList<Listener> listeners;
};

} // namespace juce
