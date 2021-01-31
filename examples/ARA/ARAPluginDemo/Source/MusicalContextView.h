#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

class DocumentView;

//==============================================================================
/**
    MusicalContextView
    JUCE component used to display musical context data: rulers for song time (in seconds and musical beats) and chords
*/
class MusicalContextView  : public juce::Component,
                            public juce::SettableTooltipClient,
                            private juce::ARAEditorView::Listener,
                            private juce::ARADocument::Listener,
                            private juce::ARAMusicalContext::Listener,
                            private juce::Timer
{
public:
    MusicalContextView (DocumentView& documentView);
    ~MusicalContextView() override;

    // may return nullptr
    juce::ARAMusicalContext* getCurrentMusicalContext() const { return musicalContext; }

    void paint (juce::Graphics&) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const juce::ARAViewSelection& viewSelection) override;

    // ARADocument::Listener overrides
    void didEndEditing (juce::ARADocument* document) override;
    void willRemoveMusicalContextFromDocument (juce::ARADocument* document, juce::ARAMusicalContext* musicalContext) override;
    void didReorderMusicalContextsInDocument (juce::ARADocument* document) override;
    void willDestroyDocument (juce::ARADocument* document) override;

    // ARAMusicalContext::Listener overrides
    void doUpdateMusicalContextContent (juce::ARAMusicalContext* musicalContext, juce::ARAContentUpdateScopes scopeFlags) override;

    // MouseListener overrides
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDoubleClick (const juce::MouseEvent& event) override;

    // juce::Timer overrides
    void timerCallback() override;

private:
    void detachFromDocument();
    void detachFromMusicalContext();
    void findMusicalContext();

private:
    DocumentView& documentView;
    juce::ARADocument* document;
    juce::ARAMusicalContext* musicalContext;
    juce::AudioPlayHead::CurrentPositionInfo lastPaintedPosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MusicalContextView)
};
