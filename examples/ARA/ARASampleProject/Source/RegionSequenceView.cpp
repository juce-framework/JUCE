#include "RegionSequenceView.h"
#include "DocumentView.h"
#include "TrackHeaderView.h"
#include "PlaybackRegionView.h"

//==============================================================================
RegionSequenceView::RegionSequenceView (DocumentView& docView, ARARegionSequence* sequence)
    : documentView (docView),
      regionSequence (sequence),
      trackHeaderView (docView.getARAEditorView(), regionSequence)
{
    regionSequence->addListener (this);

    documentView.getTrackHeadersView().addAndMakeVisible (trackHeaderView);

    for (auto playbackRegion : regionSequence->getPlaybackRegions<ARAPlaybackRegion>())
        addRegionSequenceViewAndMakeVisible (playbackRegion);
}

RegionSequenceView::~RegionSequenceView()
{
    detachFromRegionSequence();
}

void RegionSequenceView::addRegionSequenceViewAndMakeVisible (ARAPlaybackRegion* playbackRegion)
{
    auto view = new PlaybackRegionView (documentView, playbackRegion);
    playbackRegionViews.add (view);
    documentView.getPlaybackRegionsView().addAndMakeVisible (view);
}

void RegionSequenceView::detachFromRegionSequence()
{
    if (regionSequence == nullptr)
        return;

    regionSequence->removeListener (this);

    regionSequence = nullptr;
}

//==============================================================================
void RegionSequenceView::setRegionsViewBoundsByYRange (int y, int height)
{
    trackHeaderView.setBounds (0, y, trackHeaderView.getParentWidth(), height);

    for (auto regionView : playbackRegionViews)
    {
        const auto regionTimeRange = regionView->getTimeRange();
        const int startX = documentView.getPlaybackRegionsViewsXForTime (regionTimeRange.getStart());
        const int endX = documentView.getPlaybackRegionsViewsXForTime (regionTimeRange.getEnd());
        regionView->setBounds (startX, y, endX - startX, height);
    }
}

//==============================================================================
void RegionSequenceView::willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* /*regionSequence*/, ARAPlaybackRegion* playbackRegion)
{
    for (int i = 0; i < playbackRegionViews.size(); ++i)
    {
        if (playbackRegionViews[i]->getPlaybackRegion() == playbackRegion)
        {
            playbackRegionViews.remove (i);
            break;
        }
    }

    documentView.invalidateRegionSequenceViews();
}

void RegionSequenceView::didAddPlaybackRegionToRegionSequence (ARARegionSequence* /*regionSequence*/, ARAPlaybackRegion* playbackRegion)
{
    addRegionSequenceViewAndMakeVisible (playbackRegion);

    documentView.invalidateRegionSequenceViews();
}

void RegionSequenceView::willDestroyRegionSequence (ARARegionSequence* /*regionSequence*/)
{
    detachFromRegionSequence();

    documentView.invalidateRegionSequenceViews();
}

void RegionSequenceView::willUpdateRegionSequenceProperties (ARARegionSequence* /*regionSequence*/, ARARegionSequence::PropertiesPtr newProperties)
{
    if (newProperties->color != regionSequence->getColor())
    {
        // repaints any PlaybackRegion that should follow RegionSequence color
        for (auto region : playbackRegionViews)
        {
            if (region->getPlaybackRegion()->getColor() == nullptr)
                region->repaint();
        }
    }
}

