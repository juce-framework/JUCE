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

    // MouseListener overrides
    void mouseDown (const MouseEvent& event) override;
    void mouseDoubleClick (const MouseEvent& event) override;

private:
    void detachFromDocument();
    void detachFromMusicalContext();
    void findMusicalContext ();

    // useful typedefse
    using HostTempoEntryReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries>;
    using HostBarSignatureReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures>;
    using HostChordReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeSheetChords>;

    // helper functions for finding data in the ARAMusicalContext
    double getTempoForTime (ARA::ARATimePosition timeInSeconds, ARA::ARAContentTempoEntry& leftEntry, ARA::ARAContentTempoEntry&rightEntry) const;
    ARA::ARAQuarterPosition getQuarterForTime (ARA::ARATimePosition timePosition) const;
    ARA::ARAQuarterPosition getQuarterForBeat (double beatPosition) const;
    double getBeatForQuarter (ARA::ARAQuarterPosition quarterPosition) const;
    ARA::ARATimePosition getTimeForQuarter (ARA::ARAQuarterPosition quarterPosition) const;
    ARA::ARAContentBarSignature getBarSignatureForQuarter (ARA::ARAQuarterPosition quarterPos) const;

private:
    ARASampleProjectAudioProcessorEditor& owner;
    ARADocument* document;
    ARAMusicalContext* musicalContext;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RulersView)
};
