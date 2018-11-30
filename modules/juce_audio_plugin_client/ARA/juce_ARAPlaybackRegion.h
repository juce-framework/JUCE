#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAPlaybackRegion : public ARA::PlugIn::PlaybackRegion
{
public:
    ARAPlaybackRegion (ARAAudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef);

    class Listener
    {
    public:
        virtual ~Listener() = default;

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties) {}
        virtual void didUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion) {}
        virtual void didUpdatePlaybackRegionContent (ARAPlaybackRegion* playbackRegion, ARAContentUpdateScopes scopeFlags) {}
        virtual void willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    double getHeadTime() const { return headTime; }
    double getTailTime() const { return tailTime; }
    void setHeadTime (double newHeadTime);
    void setTailTime (double newTailTime);
    void setHeadAndTailTime (double newHeadTime, double newTailTime);

    void notifyContentChanged (ARAContentUpdateScopes scopeFlags);

private:
    double headTime = 0.0;
    double tailTime = 0.0;

    //==============================================================================
    JUCE_ARA_MODEL_OBJECT_LISTENERLIST

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPlaybackRegion)
};

} // namespace juce
