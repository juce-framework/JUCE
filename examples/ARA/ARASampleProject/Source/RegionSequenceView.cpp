#include "RegionSequenceView.h"
#include "PlaybackRegionView.h"
#include "ARASampleProjectPlaybackRenderer.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

//==============================================================================
RegionSequenceView::RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence)
: editorComponent (editor),
  regionSequence (sequence)
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
        regionView->getTimeRange(regionViewStartTime, regionViewEndTime);
        startTime = jmin (startTime, regionViewStartTime);
        endTime = jmax (endTime, regionViewEndTime);
    }
}

//==============================================================================
void RegionSequenceView::paint (Graphics& g)
{
    if (regionSequence == nullptr)
        return;

    Colour trackColour;
    if (const ARA::ARAColor* colour = regionSequence->getColor())
        trackColour = Colour::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);

    // draw region sequence header
    Rectangle<int> headerRect (0, 0, ARASampleProjectAudioProcessorEditor::kTrackHeaderWidth, getHeight());
    g.setColour (trackColour);
    g.fillRect (headerRect);

    // draw selection state as a yellow border around the header
    g.setColour (isSelected ? Colours::yellow : Colours::black);
    g.drawRect (headerRect);

    // draw the track name (vertically) in the header by rotating
    // the graphics transform about the center header rect
    // TODO JUCE_ARA this was more of a trial-and-error process - is
    // there a cleaner way to draw vertical text inside headerRect?
    Graphics::ScopedSaveState state (g);
    g.addTransform (AffineTransform::rotation (-MathConstants<float>::halfPi, 
                                               headerRect.getWidth() * 0.5f,
                                               headerRect.getHeight() * 0.5f));
    Rectangle<int> textRect ((int) (-0.3f * headerRect.getHeight()), (int) (1.25f * headerRect.getWidth()), (int) (0.85f * headerRect.getHeight()), headerRect.getWidth());
    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (Font (12.0f));
    g.drawText (String (regionSequence->getName()), textRect, Justification::bottomLeft);
}

void RegionSequenceView::resized()
{
    if (regionSequence == nullptr)
        return;

    double startTime, endTime;
    getTimeRange (startTime, endTime);

    // we should be sized to fit the range of time from the start 
    // of the first region sequence to the end of our last playback region
    double viewStartTime = editorComponent->getMinRegionSequenceStartTime();
    double viewWidthTime = endTime - viewStartTime;

    for (auto regionView : playbackRegionViews)
    {
        double regionViewStartTime, regionViewEndTime;
        regionView->getTimeRange(regionViewStartTime, regionViewEndTime);

        // normalize region boundaries to our visible timeRange
        double normalizedStartPos = (regionViewStartTime - viewStartTime) / viewWidthTime;
        double normalizedLength = (regionViewEndTime - regionViewStartTime) / viewWidthTime;

        // compute region view bounds and place the bounds just after the track header
        auto regionViewBounds = getLocalBounds();
        regionViewBounds.setX ((int) (regionViewBounds.getWidth() * normalizedStartPos));
        regionViewBounds.setWidth ((int) (regionViewBounds.getWidth() * normalizedLength));
        regionViewBounds.translate (ARASampleProjectAudioProcessorEditor::kTrackHeaderWidth, 0);
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
