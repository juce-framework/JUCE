#include "juce_ARADocument.h"

namespace juce
{

ARADocument::ARADocument (ARADocumentController* documentController)
: ARA::PlugIn::Document (documentController)
{}

void ARADocument::doBeginEditing()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.doBeginEditing (this); });
}

void ARADocument::doEndEditing()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.doEndEditing (this); });
}

void ARADocument::willUpdateDocumentProperties (ARADocument::PropertiesPtr newProperties)
{
    listeners.callExpectingUnregistration ([this, &newProperties] (Listener& l) { l.willUpdateDocumentProperties(this, newProperties); });
}

void ARADocument::didUpdateDocumentProperties()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.didUpdateDocumentProperties (this); });
}

void ARADocument::willDestroyDocument()
{
    listeners.callExpectingUnregistration ([this] (Listener& l) { l.willDestroyDocument (this); });
}

void ARADocument::addListener (Listener * l)
{
    listeners.add (l);
}

void ARADocument::removeListener (Listener * l)
{
    listeners.remove (l);
}

} // namespace juce
