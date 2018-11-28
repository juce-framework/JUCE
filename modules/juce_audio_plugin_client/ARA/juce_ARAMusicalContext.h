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
        virtual void willUpdateMusicalContextProperties (ARAMusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) {}
        virtual void didUpdateMusicalContextProperties (ARAMusicalContext* musicalContext) {}
        virtual void didUpdateMusicalContextContent (ARAMusicalContext* musicalContext, ARAContentUpdateScopes scopeFlags) {}
        virtual void willDestroyMusicalContext (ARAMusicalContext* musicalContext) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

public:         // to be called by ARADocumentController only
    void willUpdateMusicalContextProperties (ARAMusicalContext::PropertiesPtr newProperties);
    void didUpdateMusicalContextProperties();
    void didUpdateMusicalContextContent (ARAContentUpdateScopes scopeFlags);
    void willDestroyMusicalContext();

private:
    ListenerList<Listener> listeners;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAMusicalContext)
};

} // namespace juce
