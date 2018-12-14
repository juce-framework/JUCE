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

    editorComponent->getTracksView().addAndMakeVisible (*trackHeaderView);

    for (auto playbackRegion : regionSequence->getPlaybackRegions())
        addRegionSequenceView (static_cast<ARAPlaybackRegion*> (playbackRegion));
}

RegionSequenceView::~RegionSequenceView()
{
    detachFromRegionSequence();
}

void RegionSequenceView::addRegionSequenceView (ARAPlaybackRegion* playbackRegion)
{
    auto view = new PlaybackRegionView (editorComponent, playbackRegion);
    playbackRegionViews.add (view);
    editorComponent->getRegionSequenceListView().addAndMakeVisible (view);
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

    regionSequence->getTimeRange (startTime, endTime, false);
}

void RegionSequenceView::setRegionsViewBounds (int x, int y, int width, int height)
{
    trackHeaderView->setBounds (0, y, x, height);

    double viewStartTime, viewEndTime;
    editorComponent->getTimeRange (viewStartTime, viewEndTime);
    double viewWidthTime = viewEndTime - viewStartTime;

    for (auto regionView : playbackRegionViews)
    {
        double regionViewStartTime, regionViewEndTime;
        regionView->getTimeRange (regionViewStartTime, regionViewEndTime);

        double normalizedStartTime = (regionViewStartTime - viewStartTime) / viewWidthTime;
        double normalizedWidthTime = (regionViewEndTime - regionViewStartTime) / viewWidthTime;

        regionView->setBounds (roundToInt (width * normalizedStartTime), y, roundToInt (width * normalizedWidthTime), height);
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

    addRegionSequenceView (playbackRegion);

    editorComponent->setDirty();
}

void RegionSequenceView::willDestroyRegionSequence (ARARegionSequence* sequence)
{
    jassert (regionSequence == sequence);

    detachFromRegionSequence();

    editorComponent->setDirty();
}
