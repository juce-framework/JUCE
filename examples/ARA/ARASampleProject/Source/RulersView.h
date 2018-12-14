#pragma once

#include "JuceHeader.h"

//==============================================================================
/**
    RulersView
    JUCE component used to display rulers for song time (in seconds and musical beats) and chords
*/
class RulersView  : public Component,
                    private ARADocument::Listener,
                    private ARAMusicalContext::Listener
{
public:
    RulersView (ARADocument* document);
    ~RulersView();

    void paint (Graphics&) override;

    // ARADocument::Listener overrides
    void willRemoveMusicalContextFromDocument (ARADocument* document, ARAMusicalContext* musicalContext) override;
    void didReorderMusicalContextsInDocument (ARADocument* document) override;
    void willDestroyDocument (ARADocument* document) override;

    // ARAMusicalContext::Listener overrides
    void doUpdateMusicalContextContent (ARAMusicalContext* context, ARAContentUpdateScopes scopeFlags) override;

private:
    void detachFromDocument();
    void detachFromMusicalContext();

private:
    ARADocument* document;
    ARAMusicalContext* musicalContext;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RulersView)
};
