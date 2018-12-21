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

    using HostTempoEntryReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries>;
    using HostBarSignatureReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures>;
    using HostChordReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeSheetChords>;

    double findTempoForTime (ARA::ARATimePosition timeInSeconds, ARA::ARAContentTempoEntry& leftEntry, ARA::ARAContentTempoEntry&rightEntry) const;
    ARA::ARAContentBarSignature findBarSignatureForTime (ARA::ARATimePosition timeInSeconds) const;
    double findTimeOfBeat (ARA::ARAContentBarSignature barSignature, double beat) const;
    ARA::ARAContentBarSignature findBarSignatureForQuarter (ARA::ARAQuarterPosition quarterPos) const;

    static inline double quartersToSeconds (ARA::ARAQuarterPosition position, double tempo)
    {
        return 60. * position / tempo;
    }
    static inline double secondsToQuarters (ARA::ARATimePosition seconds, double tempo)
    {
        return tempo * seconds / 60.;
    }
    static inline double quartersToBeats (ARA::ARAContentBarSignature barSignature, ARA::ARAQuarterPosition quarterPosition)
    {
        return (barSignature.denominator / 4.) * quarterPosition;
    };
    static inline double beatsToQuarters (ARA::ARAContentBarSignature barSignature, ARA::ARAQuarterPosition beatPosition)
    {
        return (4. / barSignature.denominator) * beatPosition;
    };

private:
    ARASampleProjectAudioProcessorEditor& owner;
    ARADocument* document;
    ARAMusicalContext* musicalContext;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RulersView)
};
