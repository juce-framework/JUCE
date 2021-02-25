#include "RegionSequenceViewContainer.h"
#include "DocumentView.h"
#include "PlaybackRegionView.h"

//==============================================================================
RegionSequenceViewContainer::RegionSequenceViewContainer (DocumentView& docView, juce::ARARegionSequence* sequence)
    : documentView (docView),
      regionSequence (sequence),
      regionSequenceHeaderView (docView.getARAEditorView(), regionSequence)
{
    regionSequence->addListener (this);

    documentView.getRegionSequenceHeadersView().addAndMakeVisible (regionSequenceHeaderView);

    for (auto playbackRegion : regionSequence->getPlaybackRegions())
        addRegionSequenceViewAndMakeVisible (playbackRegion);
}

RegionSequenceViewContainer::~RegionSequenceViewContainer()
{
    detachFromRegionSequence();
}

void RegionSequenceViewContainer::addRegionSequenceViewAndMakeVisible (juce::ARAPlaybackRegion* playbackRegion)
{
    auto view = new PlaybackRegionView (*this, playbackRegion);
    playbackRegionViews.add (view);
    documentView.getPlaybackRegionsView().addAndMakeVisible (view);
}

void RegionSequenceViewContainer::detachFromRegionSequence()
{
    if (regionSequence == nullptr)
        return;

    regionSequence->removeListener (this);

    regionSequence = nullptr;
}

//==============================================================================
void RegionSequenceViewContainer::setRegionsViewBoundsByYRange (int y, int height)
{
    regionSequenceHeaderView.setBounds (0, y, regionSequenceHeaderView.getParentWidth(), height);

    for (auto regionView : playbackRegionViews)
        regionView->updateBounds();
}

//==============================================================================
void RegionSequenceViewContainer::willRemovePlaybackRegionFromRegionSequence (juce::ARARegionSequence* /*regionSequence*/, juce::ARAPlaybackRegion* playbackRegion)
{
    for (int i = 0; i < playbackRegionViews.size(); ++i)
    {
        if (playbackRegionViews[i]->getPlaybackRegion() == playbackRegion)
        {
            playbackRegionViews.remove (i);
            break;
        }
    }

    documentView.invalidateTimeRange();
}

void RegionSequenceViewContainer::didAddPlaybackRegionToRegionSequence (juce::ARARegionSequence* /*regionSequence*/, juce::ARAPlaybackRegion* playbackRegion)
{
    addRegionSequenceViewAndMakeVisible (playbackRegion);

    documentView.invalidateTimeRange();
}

void RegionSequenceViewContainer::willDestroyRegionSequence (juce::ARARegionSequence* /*regionSequence*/)
{
    detachFromRegionSequence();
}

void RegionSequenceViewContainer::willUpdateRegionSequenceProperties (juce::ARARegionSequence* /*regionSequence*/, juce::ARARegionSequence::PropertiesPtr newProperties)
{
    if (newProperties->color != regionSequence->getColor())
    {
        // repaint any PlaybackRegion that should follow RegionSequence color
        for (auto regionView : playbackRegionViews)
            if (regionView->getPlaybackRegion()->getColor() == nullptr)
                regionView->repaint();
    }
}

