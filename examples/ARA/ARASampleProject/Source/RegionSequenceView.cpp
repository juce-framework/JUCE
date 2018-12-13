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

void RegionSequenceView::detachFromRegionSequence()
{
    if (regionSequence == nullptr)
        return;

    regionSequence->removeListener(this);

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

    regionSequence->getTimeRange(startTime, endTime, false);
}

//==============================================================================
Component& RegionSequenceView::getTrackHeaderView()
{
    return *trackHeaderView;
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
