#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{
class ARAPlaybackRegion;

class ARAAudioModification : public ARA::PlugIn::AudioModification
{
public:
    ARAAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef);

    class Listener
    {
    public:
        virtual ~Listener() {}

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willUpdateAudioModificationProperties (ARAAudioModification* audioModification, PropertiesPtr newProperties) {}
        virtual void didUpdateAudioModificationProperties (ARAAudioModification* audioModification) {}
        virtual void didUpdateAudioModificationContent (ARAAudioModification* audioModification, ARAContentUpdateScopes scopeFlags) {}
        virtual void doDeactivateAudioModificationForUndoHistory (ARAAudioModification* audioModification, bool deactivate) {}
        virtual void didAddPlaybackRegion (ARAAudioModification* audioModification, ARAPlaybackRegion* playbackRegion) {}
        virtual void willDestroyAudioModification (ARAAudioModification* audioModification) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    //==============================================================================
    JUCE_ARA_MODEL_OBJECT_LISTENERLIST

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAAudioModification)
};

} // namespace juce
