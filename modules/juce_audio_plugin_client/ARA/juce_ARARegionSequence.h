#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAPlaybackRegion;

class ARARegionSequence : public ARA::PlugIn::RegionSequence
{
public:
    ARARegionSequence (ARA::PlugIn::Document* document, ARA::ARARegionSequenceHostRef hostRef);

    class Listener
    {
    public:
        virtual ~Listener() {}

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) {}
        virtual void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) {}
        virtual void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) {}
        virtual void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) {}
        virtual void willDestroyRegionSequence (ARARegionSequence* regionSequence) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

public:         // to be called by ARADocumentController only
    void willUpdateRegionSequenceProperties (ARARegionSequence::PropertiesPtr newProperties);
    void didUpdateRegionSequenceProperties();
    void didAddPlaybackRegionToRegionSequence (ARAPlaybackRegion* playbackRegion);
    void willRemovePlaybackRegionFromRegionSequence (ARAPlaybackRegion* playbackRegion);
    void willDestroyRegionSequence();

private:
    ListenerList<Listener> listeners;
};

} // namespace juce
