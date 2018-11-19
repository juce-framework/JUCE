#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAMusicalContext : public ARA::PlugIn::MusicalContext
{
public:
    ARAMusicalContext (ARADocument* document, ARA::ARAMusicalContextHostRef hostRef);

    class Listener
    {
    public:
        virtual ~Listener() {}

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        void willUpdateMusicalContextProperties (ARAMusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) {}
        void didUpdateMusicalContextProperties (ARAMusicalContext* musicalContext) {}
        void doUpdateMusicalContextContent (ARAMusicalContext* musicalContext, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) {}
        void willDestroyMusicalContext (ARAMusicalContext* musicalContext) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

public:         // to be called by ARADocumentController only
    void willUpdateMusicalContextProperties (ARAMusicalContext::PropertiesPtr newProperties);
    void didUpdateMusicalContextProperties();
    void doUpdateMusicalContextContent (const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags);
    void willDestroyMusicalContext();

private:
    ListenerList<Listener> listeners;
};

} // namespace juce
