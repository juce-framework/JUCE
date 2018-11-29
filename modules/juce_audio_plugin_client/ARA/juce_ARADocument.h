#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{
class ARADocumentController;
class ARARegionSequence;
class ARAMusicalContext;
class ARAAudioSource;

class ARADocument : public ARA::PlugIn::Document
{
public:
    ARADocument (ARADocumentController* documentController);
    
    class Listener
    {
    public:
        virtual ~Listener() {}

       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_BEGIN
        virtual void doBeginEditing (ARADocument* document) {}
        virtual void doEndEditing (ARADocument* document) {}
        virtual void willUpdateDocumentProperties (ARADocument* document, ARADocument::PropertiesPtr newProperties) {}
        virtual void didUpdateDocumentProperties (ARADocument* document) {}
        virtual void didReorderRegionSequencesInDocument (ARADocument* document) {}
        virtual void didAddMusicalContext (ARADocument* document, ARAMusicalContext* musicalContext) {}
        virtual void didAddRegionSequence (ARADocument* document, ARARegionSequence* regionSequence) {}
        virtual void didAddAudioSource (ARADocument* document, ARAAudioSource* audioSource) {}
        virtual void willDestroyDocument (ARADocument* document) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    //==============================================================================
    JUCE_ARA_MODEL_OBJECT_LISTENERLIST

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARADocument)
};

} // namespace juce
