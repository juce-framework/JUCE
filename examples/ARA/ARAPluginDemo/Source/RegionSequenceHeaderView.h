#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

//==============================================================================
/**
    RegionSequenceHeaderView
    JUCE component used to display ARA region sequence name, color, and selection state
*/
class RegionSequenceHeaderView    : public juce::Component,
                                    private juce::ARAEditorView::Listener,
                                    private juce::ARARegionSequence::Listener
{
public:
    RegionSequenceHeaderView (juce::ARAEditorView* editorView, juce::ARARegionSequence* regionSequence);
    ~RegionSequenceHeaderView() override;

    void paint (juce::Graphics&) override;

    // ARAEditorView::Listener overrides
    void onNewSelection (const juce::ARAViewSelection& viewSelection) override;

    // ARARegionSequence::Listener overrides
    void didUpdateRegionSequenceProperties (juce::ARARegionSequence* regionSequence) override;
    void willDestroyRegionSequence (juce::ARARegionSequence* regionSequence) override;

private:
    void detachFromRegionSequence();

private:
    juce::ARAEditorView* editorView;
    juce::ARARegionSequence* regionSequence;
    bool isSelected { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegionSequenceHeaderView)
};
