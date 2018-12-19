#pragma once

#include "JuceHeader.h"

class ARASampleProjectAudioProcessorEditor;

//==============================================================================
/**
    RulersView
    JUCE component used to display rulers for song time (in seconds and musical beats) and chords
*/
class RulersView  : public Component,
                    private ARAEditorView::Listener,
                    private ARADocument::Listener,
                    private ARAMusicalContext::Listener
{
public:
    RulersView (ARASampleProjectAudioProcessorEditor& owner);
    ~RulersView();

    void paint (Graphics&) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void willRemoveMusicalContextFromDocument (ARADocument* document, ARAMusicalContext* musicalContext) override;
    void didReorderMusicalContextsInDocument (ARADocument* document) override;
    void willDestroyDocument (ARADocument* document) override;

    // ARAMusicalContext::Listener overrides
    void doUpdateMusicalContextContent (ARAMusicalContext* context, ARAContentUpdateScopes scopeFlags) override;

private:
    void detachFromDocument();
    void detachFromMusicalContext();
    void findMusicalContext ();

private:
    ARASampleProjectAudioProcessorEditor& owner;
    ARADocument* document;
    ARAMusicalContext* musicalContext;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RulersView)
};
