#pragma once

#include "JuceHeader.h"
#include "DocumentView.h"

//==============================================================================
/**
    RulersView
    JUCE component used to display rulers for song time (in seconds and musical beats) and chords
*/
class RulersView  : public Component,
                    private ARAEditorView::Listener,
                    private ARADocument::Listener,
                    private ARAMusicalContext::Listener,
                    private juce::Timer
{
public:
    RulersView (DocumentView& documentView);
    ~RulersView();

    // may return nullptr
    ARAMusicalContext* getCurrentMusicalContext() const { return musicalContext; }

    void paint (Graphics&) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARA::PlugIn::ViewSelection& viewSelection) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void willRemoveMusicalContextFromDocument (ARADocument* document, ARAMusicalContext* musicalContext) override;
    void didReorderMusicalContextsInDocument (ARADocument* document) override;
    void willDestroyDocument (ARADocument* document) override;

    // ARAMusicalContext::Listener overrides
    void didUpdateMusicalContextContent (ARAMusicalContext* musicalContext, ARAContentUpdateScopes scopeFlags) override;

    // MouseListener overrides
    void mouseDown (const MouseEvent& event) override;
    void mouseDoubleClick (const MouseEvent& event) override;

    // juce::Timer overrides
    void timerCallback() override;

private:
    void detachFromDocument();
    void detachFromMusicalContext();
    void findMusicalContext();

private:
    DocumentView& documentView;
    ARADocument* document;
    ARAMusicalContext* musicalContext;
    AudioPlayHead::CurrentPositionInfo lastPaintedPosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RulersView)
};
