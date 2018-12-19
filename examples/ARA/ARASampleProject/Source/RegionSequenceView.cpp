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
      trackHeaderView (new TrackHeaderView (editor->getARAEditorView(), regionSequence))
{
    regionSequence->addListener (this);

    editorComponent->getTrackHeadersView().addAndMakeVisible (*trackHeaderView);

    for (auto playbackRegion : regionSequence->getPlaybackRegions<ARAPlaybackRegion>())
        addRegionSequenceViewAndMakeVisible (playbackRegion);
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
Range<double> RegionSequenceView::getTimeRange() const
{
    if (regionSequence == nullptr)
        return {};

    return regionSequence->getTimeRange (true);
}

void RegionSequenceView::setRegionsViewBoundsByYRange (int y, int height)
{
    trackHeaderView->setBounds (0, y, trackHeaderView->getParentWidth(), height);

    for (auto regionView : playbackRegionViews)
    {
        Range<double> regionTimeRange = regionView->getTimeRange();
        int startX = editorComponent->getPlaybackRegionsViewsXForTime (regionTimeRange.getStart());
        int endX = editorComponent->getPlaybackRegionsViewsXForTime (regionTimeRange.getEnd());
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

    editorComponent->invalidateRegionSequenceViews();
}

void RegionSequenceView::didAddPlaybackRegionToRegionSequence (ARARegionSequence* sequence, ARAPlaybackRegion* playbackRegion)
{
    jassert (regionSequence == sequence);

    addRegionSequenceViewAndMakeVisible (playbackRegion);

    editorComponent->invalidateRegionSequenceViews();
}

void RegionSequenceView::willDestroyRegionSequence (ARARegionSequence* sequence)
{
    jassert (regionSequence == sequence);

    detachFromRegionSequence();

    editorComponent->invalidateRegionSequenceViews();
}
