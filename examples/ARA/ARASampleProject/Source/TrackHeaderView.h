#pragma once

#include "JuceHeader.h"

//==============================================================================
/**
    TrackHeaderView
    JUCE component used to display ARA region sequence name, color, and selection state
*/
class TrackHeaderView   : public Component,
                          private ARAEditorView::Listener,
                          private ARARegionSequence::Listener
{
public:
    TrackHeaderView (ARAEditorView* editorView, ARARegionSequence* regionSequence);
    ~TrackHeaderView();

    void paint (Graphics&) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const ARAViewSelection& viewSelection) override;

    // ARARegionSequence::Listener overrides
    void didUpdateRegionSequenceProperties (ARARegionSequence* regionSequence) override;
    void willDestroyRegionSequence (ARARegionSequence* regionSequence) override;

private:
    void detachFromRegionSequence();

private:
    ARAEditorView* editorView;
    ARARegionSequence* regionSequence;
    bool isSelected { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackHeaderView)
};
