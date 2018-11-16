#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAAudioSource : public ARA::PlugIn::AudioSource
{
public:
    ARAAudioSource (ARA::PlugIn::Document* document, ARA::ARAAudioSourceHostRef hostRef);

    class Listener
    {
    public:
        virtual ~Listener()  {}

        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willUpdateAudioSourceProperties (ARAAudioSource* audioSource, PropertiesPtr newProperties) noexcept {}
        virtual void didUpdateAudioSourceProperties (ARAAudioSource* audioSource) noexcept {}
        virtual void doUpdateAudioSourceContent (ARAAudioSource* audioSource, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept {}
        virtual void willEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) noexcept {}
        virtual void didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable) noexcept {}
        virtual void doDeactivateAudioSourceForUndoHistory (ARAAudioSource* audioSource, bool deactivate) noexcept {}
        virtual void willDestroyAudioSource (ARAAudioSource* audioSource) noexcept {}
        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

public:         // to be called by ARADocumentController only
    void willUpdateAudioSourceProperties (PropertiesPtr newProperties) noexcept;
    void didUpdateAudioSourceProperties() noexcept;
    void doUpdateAudioSourceContent (const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept;
    void willEnableAudioSourceSamplesAccess (bool enable) noexcept;
    void didEnableAudioSourceSamplesAccess (bool enable) noexcept;
    void doDeactivateAudioSourceForUndoHistory (bool deactivate) noexcept;
    void willDestroyAudioSource() noexcept;

private:
    ListenerList<Listener> listeners;
};

}
