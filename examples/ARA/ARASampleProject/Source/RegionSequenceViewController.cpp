#include "RegionSequenceViewController.h"
#include "DocumentView.h"
#include "PlaybackRegionView.h"

//==============================================================================
RegionSequenceViewController::RegionSequenceViewController (DocumentView& docView, ARARegionSequence* sequence)
    : documentView (docView),
      regionSequence (sequence),
      trackHeaderView (docView.getARAEditorView(), regionSequence)
{
    regionSequence->addListener (this);

    documentView.getTrackHeadersView().addAndMakeVisible (trackHeaderView);

    for (auto playbackRegion : regionSequence->getPlaybackRegions<ARAPlaybackRegion>())
        addRegionSequenceViewAndMakeVisible (playbackRegion);
}

RegionSequenceViewController::~RegionSequenceViewController()
{
    detachFromRegionSequence();
}

void RegionSequenceViewController::addRegionSequenceViewAndMakeVisible (ARAPlaybackRegion* playbackRegion)
{
    auto view = new PlaybackRegionView (documentView, playbackRegion);
    playbackRegionViews.add (view);
    documentView.getPlaybackRegionsView().addAndMakeVisible (view);
}

void RegionSequenceViewController::detachFromRegionSequence()
{
    if (regionSequence == nullptr)
        return;

    regionSequence->removeListener (this);

    regionSequence = nullptr;
}

//==============================================================================
void RegionSequenceViewController::setRegionsViewBoundsByYRange (int y, int height)
{
    trackHeaderView.setBounds (0, y, trackHeaderView.getParentWidth(), height);

    for (auto regionView : playbackRegionViews)
    {
        const auto regionTimeRange = regionView->getTimeRange();
        const int startX = documentView.getPlaybackRegionsViewsXForTime (regionTimeRange.getStart());
        const int endX = documentView.getPlaybackRegionsViewsXForTime (regionTimeRange.getEnd());
        const int width = jmax (1, endX - startX);
        regionView->setBounds (startX, y, width, height);
    }
}

//==============================================================================
void RegionSequenceViewController::willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* /*regionSequence*/, ARAPlaybackRegion* playbackRegion)
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

void RegionSequenceViewController::didAddPlaybackRegionToRegionSequence (ARARegionSequence* /*regionSequence*/, ARAPlaybackRegion* playbackRegion)
{
    addRegionSequenceViewAndMakeVisible (playbackRegion);

    documentView.invalidateRegionSequenceViews();
}

void RegionSequenceViewController::willDestroyRegionSequence (ARARegionSequence* /*regionSequence*/)
{
    detachFromRegionSequence();

    documentView.invalidateRegionSequenceViews();
}

void RegionSequenceViewController::willUpdateRegionSequenceProperties (ARARegionSequence* /*regionSequence*/, ARARegionSequence::PropertiesPtr newProperties)
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

