#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{
class ARADocumentController;

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
        virtual void willUpdateDocumentProperties (Document* document, Document::PropertiesPtr newProperties) {}
        virtual void didUpdateDocumentProperties (Document* document) {}
        virtual void willDestroyDocument (Document* document) {}
       ARA_DISABLE_UNREFERENCED_PARAMETER_WARNING_END
    };

    void addListener (Listener* l);
    void removeListener (Listener* l);

public:         // to be called by ARADocumentController only
    void doBeginEditing();
    void doEndEditing();
    void willUpdateDocumentProperties (Document::PropertiesPtr newProperties);
    void didUpdateDocumentProperties();
    void willDestroyDocument();

private:
    ListenerList<Listener> listeners;
};

} // namespace juce
