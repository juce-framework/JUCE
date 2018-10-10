/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARA_Library/PlugIn/ARAPlug.h"
#include "juce_audio_plugin_client/ARA/ARADocumentController.h"
#include "AudioView.h"

class ARASampleProjectDocumentController : public juce::ARADocumentController
{
public:
    ARASampleProjectDocumentController() : juce::ARADocumentController() {}
    ARA::PlugIn::EditorView* doCreateEditorView() override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectDocumentController)
};

/** Naive Editor class that visualize current ARA Document RegionSequences state */
class ARASampleProjectEditor : public juce::Component, public ARA::PlugIn::EditorView
{
public:
    ARASampleProjectEditor (ARA::PlugIn::DocumentController*);
    void doNotifySelection (const ARA::PlugIn::ViewSelection*) override;
    void resized () override;

private:
    double _maxRegionSequenceLength;
    juce::CriticalSection selectionLock;
    juce::OwnedArray <AudioView> _regionSequenceViews;
};
