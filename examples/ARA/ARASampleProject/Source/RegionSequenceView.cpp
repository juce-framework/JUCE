#include "RegionSequenceView.h"
#include "PlaybackRegionView.h"
#include "ARASampleProjectPlaybackRenderer.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

//==============================================================================
RegionSequenceView::RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence)
    : editorComponent (editor),
      regionSequence (sequence),
      trackHeaderView (*this)
{
    editorComponent->getARAEditorView()->addListener (this);
    onNewSelection (editorComponent->getARAEditorView()->getViewSelection());

    regionSequence->addListener (this);

    for (auto playbackRegion : regionSequence->getPlaybackRegions())
    {
        playbackRegionViews.add (new PlaybackRegionView (editorComponent, static_cast<ARAPlaybackRegion*> (playbackRegion)));
        addAndMakeVisible (playbackRegionViews.getLast());
    }
}

RegionSequenceView::~RegionSequenceView()
{
    detachFromRegionSequence();
}

//==============================================================================
void RegionSequenceView::detachFromRegionSequence()
{
    if (regionSequence == nullptr)
        return;

    regionSequence->removeListener(this);

    editorComponent->getARAEditorView()->removeListener (this);

    regionSequence = nullptr;
}

void RegionSequenceView::getTimeRange (double& startTime, double& endTime) const
{
    if (playbackRegionViews.isEmpty())
    {
        startTime = 0.0;
        endTime = 0.0;
        return;
    }

    startTime = std::numeric_limits<double>::max();
    endTime = std::numeric_limits<double>::lowest();
    for (auto regionView : playbackRegionViews)
    {
        double regionViewStartTime, regionViewEndTime;
        regionView->getTimeRange (regionViewStartTime, regionViewEndTime);
        startTime = jmin (startTime, regionViewStartTime);
        endTime = jmax (endTime, regionViewEndTime);
    }
}

//==============================================================================
Component& RegionSequenceView::getTrackHeaderView()
{
    return trackHeaderView;
}

void RegionSequenceView::resized()
{
    if (regionSequence == nullptr)
        return;

    double viewStartTime, viewEndTime;
    editorComponent->getTimeRange (viewStartTime, viewEndTime);
    double viewWidthTime = viewEndTime - viewStartTime;

    for (auto regionView : playbackRegionViews)
    {
        double regionViewStartTime, regionViewEndTime;
        regionView->getTimeRange (regionViewStartTime, regionViewEndTime);

        double normalizedStartTime = (regionViewStartTime - viewStartTime) / viewWidthTime;
        double normalizedWidthTime = (regionViewEndTime - regionViewStartTime) / viewWidthTime;

        auto regionViewBounds = getLocalBounds();
        regionViewBounds.setX ((int) (regionViewBounds.getWidth() * normalizedStartTime + 0.5));
        regionViewBounds.setWidth ((int) (regionViewBounds.getWidth() * normalizedWidthTime + 0.5));
        regionView->setBounds (regionViewBounds);
    }
}

//==============================================================================
void RegionSequenceView::onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection)
{
    jassert (regionSequence != nullptr);

    bool isOurRegionSequenceSelected = ARA::contains (currentSelection.getRegionSequences(), regionSequence);
    if (isOurRegionSequenceSelected != isSelected)
    {
        isSelected = isOurRegionSequenceSelected;
        repaint();
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

    for (int i = 0; i < playbackRegionViews.size(); ++i)
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

void RegionSequenceView::willDestroyRegionSequence (ARARegionSequence* sequence)
{
    jassert (regionSequence == sequence);

    detachFromRegionSequence();

    editorComponent->setDirty();
}

//==============================================================================
RegionSequenceView::TrackHeaderView::TrackHeaderView(RegionSequenceView& owner)
: owner(owner)
{
}

void RegionSequenceView::TrackHeaderView::paint(juce::Graphics& g)
{
    if (owner.regionSequence == nullptr)
        return;

    Colour trackColour;
    if (auto& colour = owner.regionSequence->getColor())
        trackColour = Colour::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);

    // draw region sequence header
    Rectangle<int> headerRect (0, 0, kTrackHeaderWidth, getHeight());
    g.setColour (trackColour);
    g.fillRect (headerRect);

    // draw selection state as a yellow border around the header
    g.setColour (owner.isSelected ? Colours::yellow : Colours::black);
    g.drawRect (headerRect);

    if (auto& name = owner.regionSequence->getName())
    {
        g.setColour (trackColour.contrasting (1.0f));
        g.setFont (Font (12.0f));
        g.drawText (String (name), headerRect, Justification::centredLeft);
    }
}
