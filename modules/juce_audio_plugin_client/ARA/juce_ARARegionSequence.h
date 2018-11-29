#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAPlaybackRegion;

class ARARegionSequence : public ARA::PlugIn::RegionSequence
{
public:
    ARARegionSequence (ARADocument* document, ARA::ARARegionSequenceHostRef hostRef);

    // If all audio sources used by the playback regions in this region sequence have the
    // same sample rate, this rate is returned here, otherwise 0.0 is returned.
    // If the region sequence has no playback regions, this also returns 0.0.
    double getCommonSampleRate();

    class Listener
    {
    public:
        virtual ~Listener() = default;

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willUpdateRegionSequenceProperties (ARARegionSequence* regionSequence, ARARegionSequence::PropertiesPtr newProperties) {}
        virtual void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) {}
        virtual void willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) {}
        virtual void didAddPlaybackRegionToRegionSequence (ARARegionSequence* regionSequence, ARAPlaybackRegion* playbackRegion) {}
        virtual void willDestroyRegionSequence (ARARegionSequence* regionSequence) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    //==============================================================================
    JUCE_ARA_MODEL_OBJECT_LISTENERLIST

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARARegionSequence)
};

} // namespace juce
