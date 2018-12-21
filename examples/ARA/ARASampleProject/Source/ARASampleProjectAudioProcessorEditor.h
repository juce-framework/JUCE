#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "ARASampleProjectAudioProcessor.h"

class RegionSequenceView;
class RulersView;

//==============================================================================
/**
    Editor class for ARA sample project
    This class manages the UI we use to display region sequences in the
    ARA document as well as their current selection state
*/
class ARASampleProjectAudioProcessorEditor  : public AudioProcessorEditor,
                                              public AudioProcessorEditorARAExtension,
                                              private ARAEditorView::Listener,
                                              private ARADocument::Listener,
                                              private juce::Timer
{
public:
    ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor&);
    ~ARASampleProjectAudioProcessorEditor();

    // total time range
    Range<double> getTimeRange () const { return Range<double> (startTime, endTime); }

    // visible time range
    Range<double> getVisibleTimeRange () const;
// TODO JUCE_ARA if we want to make this into a reusable view, then zooming should use this primitive:
//  void setVisibleTimeRange (double start, double end);
//  It would limit the new visibile range to getTimeRange(), trying to keep requested duration unchanged.
//  Another method zoomBy(float factor) can be added on top of this, which deals with keeping the relative
//  playhead positon unchanged if it is visible while zooming, otherwise keeps current view centered.
//  This will be easy to do since it is all in linear time now.

    // flag that our view needs to be rebuilt
    void invalidateRegionSequenceViews() { regionSequenceViewsAreInvalid = true; }

    Component& getPlaybackRegionsView() { return playbackRegionsView; }
    Component& getTrackHeadersView() { return trackHeadersView; }
    Viewport& getTrackHeadersViewPort() { return trackHeadersViewPort; }
    Viewport& getRulersViewPort() { return rulersViewPort; }

    int getPlaybackRegionsViewsXForTime (double time) const;
    double getPlaybackRegionsViewsTimeForX (int x) const;

    double getPlayheadTimePosition() const { return playheadTimePosition; }

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    // juce::Timer overrides
    void timerCallback() override;

    // ARAEditorView::Listener overrides
    void onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences) override;

    // ARADocument::Listener overrides
    void didEndEditing (ARADocument* document) override;
    void didReorderRegionSequencesInDocument (ARADocument* document) override;

private:
    void rebuildRegionSequenceViews();
    void updatePlayheadBounds();

private:
    // simple utility class to show playhead position
    class PlayheadView : public Component
    {
    public:
        PlayheadView (ARASampleProjectAudioProcessorEditor& editorComponent);
        void paint (Graphics&) override;
    private:
        ARASampleProjectAudioProcessorEditor& editorComponent;
        static constexpr int kPlayheadWidth = 3;
    };

    // simple utility class to partially sync scroll postions of our view ports
    class ScrollMasterViewPort : public Viewport
    {
    public:
        ScrollMasterViewPort (ARASampleProjectAudioProcessorEditor& editorComponent) : editorComponent (editorComponent) {};
        void visibleAreaChanged (const Rectangle<int>& newVisibleArea) override;
    private:
        ARASampleProjectAudioProcessorEditor& editorComponent;
    };

    OwnedArray<RegionSequenceView> regionSequenceViews;

    ScrollMasterViewPort playbackRegionsViewPort;
    Component playbackRegionsView;
    PlayheadView playheadView;
    Viewport trackHeadersViewPort;
    Component trackHeadersView;
    Viewport rulersViewPort;
    std::unique_ptr<RulersView> rulersView;

    TextButton zoomInButton, zoomOutButton;
    ToggleButton followPlayheadToggleButton;

    bool regionSequenceViewsAreInvalid = true;
    double startTime = 0.0;
    double endTime = 1.0;
    double pixelsPerSecond = 0.0;
    double playheadTimePosition = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARASampleProjectAudioProcessorEditor)
};
