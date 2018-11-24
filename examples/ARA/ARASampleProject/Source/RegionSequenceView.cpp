#include "RegionSequenceView.h"
#include "PlaybackRegionView.h"
#include "ARASampleProjectPlaybackRenderer.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

RegionSequenceView::RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence)
: isSelected (false),
  editorComponent (editor),
  regionSequence (sequence)
{
    regionSequence->addListener (this);

    for (auto* playbackRegion : regionSequence->getPlaybackRegions())
    {
        playbackRegionViews.add (new PlaybackRegionView (editorComponent, static_cast<ARAPlaybackRegion*> (playbackRegion)));
        addAndMakeVisible (playbackRegionViews.getLast());
    }
}

RegionSequenceView::~RegionSequenceView()
{
    regionSequence->removeListener(this);
}

void RegionSequenceView::paint (Graphics& g)
{
    Colour trackColour;
    if (const ARA::ARAColor* colour = regionSequence->getColor())
        trackColour = Colour::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);

    g.fillAll (trackColour);
    g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
    g.drawRect (getLocalBounds());
    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (Font (12.0));
    g.drawText ("Track #" + String (regionSequence->getOrderIndex()) + ": " + regionSequence->getName(), 0, 0, getWidth(), getHeight(), juce::Justification::bottomLeft);
}

void RegionSequenceView::resized()
{
    double startInSeconds (0), endInSeconds (0);
    getTimeRange (startInSeconds, endInSeconds);
    double lengthInSeconds = endInSeconds - startInSeconds;

    // use this to set size of playback region views
    for (auto v : playbackRegionViews)
    {
        double normalizedStartPos = (v->getStartInSeconds() - startInSeconds) / lengthInSeconds;
        double normalizedLength = (v->getLengthInSeconds()) / lengthInSeconds;
        auto ourBounds = getLocalBounds();
        ourBounds.setX ((int) (ourBounds.getWidth() * normalizedStartPos));
        ourBounds.setWidth ((int) (ourBounds.getWidth() * normalizedLength));
        v->setBounds (ourBounds);
    }
}

void RegionSequenceView::setIsSelected (bool value)
{
    bool needsRepaint = (value != isSelected);
    isSelected = value;
    if (needsRepaint)
        repaint();
}

void RegionSequenceView::getTimeRange (double& startTimeInSeconds, double& endTimeInSeconds) const
{
    if (playbackRegionViews.isEmpty())
        return;

    startTimeInSeconds = std::numeric_limits<double>::max();
    endTimeInSeconds = 0;
    for (auto v : playbackRegionViews)
    {
        startTimeInSeconds = jmin (startTimeInSeconds, v->getStartInSeconds());
        endTimeInSeconds = jmax (endTimeInSeconds, v->getEndInSeconds());
    }
}

void RegionSequenceView::didUpdateRegionSequenceProperties (ARARegionSequence* sequence)
{
    jassert (regionSequence == sequence);

    repaint();
}

void RegionSequenceView::willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion)
{
    jassert (regionSequence == sequence);

    for (int i = 0; i < playbackRegionViews.size(); i++)
    {
        if (playbackRegionViews[i]->getPlaybackRegion() == playbackRegion)
        {
            playbackRegionViews.remove (i);
            break;
        }
    }

    editorComponent->setDirty();
}

void RegionSequenceView::didAddPlaybackRegionToRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion)
{
    jassert (regionSequence == sequence);

    playbackRegionViews.add (new PlaybackRegionView (editorComponent, playbackRegion));
    addAndMakeVisible (playbackRegionViews.getLast());

    editorComponent->setDirty();
}
