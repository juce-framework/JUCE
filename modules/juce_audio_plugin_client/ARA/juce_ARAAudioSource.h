#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{
class ARAAudioModification;

class ARAAudioSource : public ARA::PlugIn::AudioSource
{
public:
    ARAAudioSource (ARADocument* document, ARA::ARAAudioSourceHostRef hostRef);

    class Listener
    {
    public:
        virtual ~Listener()  {}

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willUpdateAudioSourceProperties (ARAAudioSource* audioSource, PropertiesPtr newProperties) {}
        virtual void didUpdateAudioSourceProperties (ARAAudioSource* audioSource) {}
        virtual void didUpdateAudioSourceContent (ARAAudioSource* audioSource, ARAContentUpdateScopes scopeFlags) {}
        virtual void willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) {}
        virtual void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) {}
        virtual void doDeactivateAudioSourceForUndoHistory (ARAAudioSource* audioSource, bool deactivate) {}
        virtual void didAddAudioModification (ARAAudioSource* audioSource, ARAAudioModification* audioModification) {}
        virtual void willDestroyAudioSource (ARAAudioSource* audioSource) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

public:         // to be called by ARADocumentController only
    void willUpdateAudioSourceProperties (PropertiesPtr newProperties);
    void didUpdateAudioSourceProperties();
    void didUpdateAudioSourceContent (ARAContentUpdateScopes scopeFlags);
    void willEnableAudioSourceSamplesAccess (bool enable);
    void didEnableAudioSourceSamplesAccess (bool enable);
    void doDeactivateAudioSourceForUndoHistory (bool deactivate);
    void didAddAudioModification (ARAAudioModification* audioModification);
    void willDestroyAudioSource();

private:
    ListenerList<Listener> listeners;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAAudioSource)
};

}
