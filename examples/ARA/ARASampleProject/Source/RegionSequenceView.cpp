#include "RegionSequenceView.h"
#include "TrackHeaderView.h"
#include "PlaybackRegionView.h"
#include "ARASampleProjectPlaybackRenderer.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

//==============================================================================
RegionSequenceView::RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence)
    : editorComponent (editor),
      regionSequence (sequence),
      trackHeaderView (new TrackHeaderView (editor->getARAEditorView(), static_cast<ARARegionSequence*> (regionSequence)))
{
    regionSequence->addListener (this);

    editorComponent->getTrackHeadersView().addAndMakeVisible (*trackHeaderView);

    for (auto playbackRegion : regionSequence->getPlaybackRegions())
        addRegionSequenceViewAndMakeVisible (static_cast<ARAPlaybackRegion*> (playbackRegion));
}

RegionSequenceView::~RegionSequenceView()
{
    detachFromRegionSequence();
}

void RegionSequenceView::addRegionSequenceViewAndMakeVisible (ARAPlaybackRegion* playbackRegion)
{
    auto view = new PlaybackRegionView (editorComponent, playbackRegion);
    playbackRegionViews.add (view);
    editorComponent->getPlaybackRegionsView().addAndMakeVisible (view);
}

void RegionSequenceView::detachFromRegionSequence()
{
    if (regionSequence == nullptr)
        return;

    regionSequence->removeListener (this);

    regionSequence = nullptr;
}

//==============================================================================
void RegionSequenceView::getTimeRange (double& startTime, double& endTime) const
{
    if (regionSequence == nullptr)
    {
        startTime = 0.0;
        endTime = 0.0;
        return;
    }

    regionSequence->getTimeRange (startTime, endTime, true);
}

void RegionSequenceView::setRegionsViewBoundsByYRange (int y, int height)
{
    trackHeaderView->setBounds (0, y, trackHeaderView->getParentWidth(), height);

    for (auto regionView : playbackRegionViews)
    {
        double regionViewStartTime, regionViewEndTime;
        regionView->getTimeRange (regionViewStartTime, regionViewEndTime);
        int startX = editorComponent->getPlaybackRegionsViewsXForTime (regionViewStartTime);
        int endX = editorComponent->getPlaybackRegionsViewsXForTime (regionViewEndTime);
        regionView->setBounds (startX, y, endX - startX, height);
    }
}

//==============================================================================
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

    addRegionSequenceViewAndMakeVisible (playbackRegion);

    editorComponent->setDirty();
}

void RegionSequenceView::willDestroyRegionSequence (ARARegionSequence* sequence)
{
    jassert (regionSequence == sequence);

    detachFromRegionSequence();

    editorComponent->setDirty();
}
