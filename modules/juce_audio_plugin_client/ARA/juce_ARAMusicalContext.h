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
        virtual ~Listener() = default;

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void willUpdateMusicalContextProperties (ARAMusicalContext* musicalContext, ARAMusicalContext::PropertiesPtr newProperties) {}
        virtual void didUpdateMusicalContextProperties (ARAMusicalContext* musicalContext) {}
        virtual void doUpdateMusicalContextContent (ARAMusicalContext* musicalContext, ARAContentUpdateScopes scopeFlags) {}
        virtual void willDestroyMusicalContext (ARAMusicalContext* musicalContext) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    //==============================================================================
    JUCE_ARA_MODEL_OBJECT_LISTENERLIST

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAMusicalContext)
};

} // namespace juce
