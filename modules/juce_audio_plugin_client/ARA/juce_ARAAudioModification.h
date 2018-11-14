#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAAudioModification : public ARA::PlugIn::AudioModification
{
public:
    ARAAudioModification (ARA::PlugIn::AudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef);
    
    void willUpdateAudioModificationProperties (ARA::PlugIn::PropertiesPtr<ARA::ARAAudioModificationProperties> newProperties) noexcept;
    void didUpdateAudioModificationProperties () noexcept;
    void doDeactivateAudioModificationForUndoHistory (bool deactivate) noexcept;
    void willDestroyAudioModification () noexcept;

    class Listener
    {
    public:
        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN

        virtual ~Listener () {}

        virtual void willUpdateAudioModificationProperties (ARAAudioModification* audioModification, ARA::PlugIn::PropertiesPtr<ARA::ARAAudioModificationProperties> newProperties) noexcept {}
        virtual void didUpdateAudioModificationProperties (ARAAudioModification* audioModification) noexcept {}
        virtual void doDeactivateAudioModificationForUndoHistory (ARAAudioModification* audioModification, bool deactivate) noexcept {}
        virtual void willDestroyAudioModification (ARAAudioModification* audioModification) noexcept {}

        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

private:
    ListenerList<Listener> listeners;
};

} // namespace juce
