#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAMusicalContext : public ARA::PlugIn::MusicalContext
{
public:
    ARAMusicalContext (ARA::PlugIn::Document* document, ARA::ARAMusicalContextHostRef hostRef);
    
    void willUpdateMusicalContextProperties (ARAMusicalContext::PropertiesPtr newProperties) noexcept;
    void didUpdateMusicalContextProperties () noexcept;
    void doUpdateMusicalContextContent (const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept;
    void willDestroyMusicalContext () noexcept;

    class Listener
    {
    public:
        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN

        virtual ~Listener () {}

        void willUpdateMusicalContextProperties (ARAMusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) noexcept {}
        void didUpdateMusicalContextProperties (ARAMusicalContext* musicalContext) noexcept {}
        void doUpdateMusicalContextContent (ARAMusicalContext* musicalContext, const ARA::ARAContentTimeRange* range, ARA::ARAContentUpdateFlags flags) noexcept {}
        void willDestroyMusicalContext (ARAMusicalContext* musicalContext) noexcept {}

        ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

private:
    ListenerList<Listener> listeners;
};

} // namespace juce
