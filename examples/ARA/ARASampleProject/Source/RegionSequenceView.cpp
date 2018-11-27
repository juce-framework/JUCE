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

    // draw region sequence header
    Rectangle<int> headerRect (0, 0, ARASampleProjectAudioProcessorEditor::kTrackHeaderWidth, getHeight ());
    g.setColour (trackColour);
    g.fillRect (headerRect);

    // draw selection state as a yellow border around the header
    g.setColour (isSelected ? Colours::yellow : Colours::black);
    g.drawRect (headerRect);

    // draw the track name (vertically) in the header by rotating
    // the graphics transform about the center header rect
    Graphics::ScopedSaveState state (g);
    g.addTransform (AffineTransform::rotation (-MathConstants<float>::halfPi, 
                                               headerRect.getWidth () * 0.5f,
                                               headerRect.getHeight () * 0.5f));
    Rectangle<int> textRect (-headerRect.getHeight (), (int)(1.25 * headerRect.getWidth ()), (int)(1.5 * headerRect.getHeight ()), headerRect.getWidth ());
    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (Font (12.0));
    g.drawText ("Track #" + String (regionSequence->getOrderIndex()) + ": " + regionSequence->getName(), textRect, Justification::bottomLeft);
}

void RegionSequenceView::resized()
{
    double startInSeconds (0), endInSeconds (0);
    getTimeRange (startInSeconds, endInSeconds);

    // use this to set size of playback region views
    for (auto v : playbackRegionViews)
    {
        // normalize region boundaries to our entire view length in seconds
        double normalizedStartPos = (v->getPlaybackRegion()->getStartInPlaybackTime()) / endInSeconds;
        double normalizedLength = (v->getPlaybackRegion ()->getDurationInPlaybackTime()) / endInSeconds;

        // compute region view bounds and place the bounds just after the track header
        auto regionBounds = getLocalBounds();
        regionBounds.setX ((int) (regionBounds.getWidth() * normalizedStartPos));
        regionBounds.setWidth ((int) (regionBounds.getWidth() * normalizedLength));
        regionBounds.translate (ARASampleProjectAudioProcessorEditor::kTrackHeaderWidth, 0);
        v->setBounds (regionBounds);
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
        startTimeInSeconds = jmin (startTimeInSeconds, v->getPlaybackRegion()->getStartInPlaybackTime());
        endTimeInSeconds = jmax (endTimeInSeconds, v->getPlaybackRegion()->getEndInPlaybackTime());
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
