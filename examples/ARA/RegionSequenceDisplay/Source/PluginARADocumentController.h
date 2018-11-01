/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA document controller implementation. 

  ==============================================================================
*/

#pragma once

#include "AudioView.h"

class ARASampleProjectPlaybackRenderer: public ARA::PlugIn::PlaybackRenderer
{
    std::map<ARAAudioSource*, std::unique_ptr<BufferingAudioSource>> _audioSourceMap;
    std::unique_ptr<TimeSliceThread> _audioReadingThread;
protected:
    virtual void didAddPlaybackRegion (ARA::PlugIn::PlaybackRegion* playbackRegion) noexcept override;

public:
    using PlaybackRenderer::PlaybackRenderer;

    void renderPlaybackRegions (AudioBuffer<float>& buffer, ARA::ARASampleRate sampleRate, ARA::ARASamplePosition samplePosition, bool isPlayingBack);
};

class ARASampleProjectDocumentController : public juce::ARADocumentController
{
public:
    ARASampleProjectDocumentController() noexcept : juce::ARADocumentController() {}
    ARA::PlugIn::EditorView* doCreateEditorView() noexcept override;
    ARA::PlugIn::PlaybackRenderer* doCreatePlaybackRenderer() noexcept override;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectDocumentController)
};

/** Naive Editor class that visualize current ARA Document RegionSequences state */
class ARASampleProjectEditor : public juce::Component, public ARA::PlugIn::EditorView
{
public:
    ARASampleProjectEditor (ARA::PlugIn::DocumentController*) noexcept;
    void doNotifySelection (const ARA::PlugIn::ViewSelection*) noexcept override;
    void resized () override;

private:
    double maxRegionSequenceLength;
    juce::CriticalSection selectionLock;
    juce::OwnedArray <AudioView> regionSequenceViews;
};
