#include "PlaybackRegionView.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

//==============================================================================
PlaybackRegionView::PlaybackRegionView (ARASampleProjectAudioProcessorEditor* editor, ARAPlaybackRegion* region)
: editorComponent (editor),
  playbackRegion (region),
  audioThumbCache (1),
  audioThumb (128, audioFormatManger, audioThumbCache)
{
    audioThumb.addChangeListener (this);

    editorComponent->getARAEditorView()->addListener (this);
    onNewSelection (editorComponent->getARAEditorView()->getViewSelection());

    static_cast<ARADocument*> (playbackRegion->getRegionSequence()->getDocument())->addListener (this);
    static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource())->addListener (this);
    playbackRegion->addListener (this);
    recreatePlaybackRegionReader();
}

PlaybackRegionView::~PlaybackRegionView()
{
    editorComponent->getARAEditorView()->removeListener (this);

    playbackRegion->removeListener (this);
    static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource())->removeListener (this);
    static_cast<ARADocument*> (playbackRegion->getRegionSequence()->getDocument())->removeListener (this);

    audioThumb.removeChangeListener (this);
    audioThumb.clear();
}

//==============================================================================
void PlaybackRegionView::getTimeRange (double& startTime, double& endTime) const
{
    startTime = playbackRegion->getStartInPlaybackTime() - playbackRegion->getHeadTime();
    endTime = playbackRegion->getEndInPlaybackTime() + playbackRegion->getTailTime();
}

void PlaybackRegionView::paint (Graphics& g)
{
    const int lineThickness = 1;

    Colour regionColour;
    const ARA::ARAColor* colour = playbackRegion->getColor();
    if (colour == nullptr)
        colour = playbackRegion->getRegionSequence()->getColor();
    if (colour != nullptr)
        regionColour = Colour::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);

    auto rect = getLocalBounds();
    double headTime = playbackRegion->getHeadTime();
    double tailTime = playbackRegion->getTailTime();
    double totalTime = playbackRegion->getDurationInPlaybackTime() + headTime + tailTime;
    int totalWidth = rect.getWidth();
    rect.removeFromLeft ((int) (totalWidth * headTime / totalTime + 0.5));
    rect.removeFromRight ((int) (totalWidth * tailTime / totalTime + 0.5));
    g.setColour (isSelected ? Colours::yellow : Colours::black);
    g.drawRect (rect, lineThickness);

    rect.reduce (lineThickness, lineThickness);
    if (colour)
    {
        g.setColour (regionColour);
        g.fillRect (rect);
    }

    if (playbackRegion->getAudioModification()->getAudioSource()->isSampleAccessEnabled())
    {
        if (totalTime != 0.0)
        {
            g.setColour (regionColour.contrasting (0.7f));
            audioThumb.drawChannels (g, getLocalBounds(), 0.0, totalTime, 1.0f);
        }
    }
    else
    {
        g.setColour (regionColour.contrasting (1.0f));
        g.setFont (Font (12.0f));
        g.drawText ("Access Disabled", getBounds(), Justification::centred);
    }

    ARA::ARAUtf8String name = playbackRegion->getName();
    if (!name)
        name = playbackRegion->getAudioModification()->getName();
    if (!name)
        name = playbackRegion->getAudioModification()->getAudioSource()->getName();
    if (name)
    {
        g.setColour (regionColour.contrasting (1.0f));
        g.setFont (Font (12.0f));
        g.drawText (String (name), rect, Justification::topLeft);
    }
}

//==============================================================================
void PlaybackRegionView::changeListenerCallback (ChangeBroadcaster* /*broadcaster*/)
{
    repaint();
}

void PlaybackRegionView::onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection)
{
    bool isOurPlaybackRegionSelected = ARA::contains (currentSelection.getPlaybackRegions(), playbackRegion);
    if (isOurPlaybackRegionSelected != isSelected)
    {
        isSelected = isOurPlaybackRegionSelected;
        repaint();
    }
}

void PlaybackRegionView::doEndEditing (ARADocument* document)
{
    jassert (document == playbackRegion->getRegionSequence()->getDocument());

    if ((playbackRegionReader ==  nullptr) || ! playbackRegionReader->isValid())
    {
        recreatePlaybackRegionReader();
        editorComponent->setDirty();
    }
}

void PlaybackRegionView::didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool /*enable*/)
{
    jassert (audioSource == playbackRegion->getAudioModification()->getAudioSource());

    repaint();
}

void PlaybackRegionView::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* region, ARAPlaybackRegion::PropertiesPtr newProperties)
{
    jassert (playbackRegion == region);

    if ((playbackRegion->getStartInPlaybackTime() != newProperties->startInPlaybackTime) ||
        (playbackRegion->getDurationInPlaybackTime() != newProperties->durationInPlaybackTime))
    {
        editorComponent->setDirty();
    }
}

void PlaybackRegionView::recreatePlaybackRegionReader()
{
    audioThumbCache.clear();

    // create a non-realtime playback region reader for our audio thumb
    auto documentController = static_cast<ARASampleProjectDocumentController*> (editorComponent->getARADocumentController());
    playbackRegionReader = documentController->createPlaybackRegionReader ({ playbackRegion }, true);
    
    // see juce_AudioThumbnail.cpp line 122 - AudioThumbnail does not deal with zero length sources.
    if (playbackRegionReader->lengthInSamples <= 0)
    {
        delete playbackRegionReader;
        playbackRegionReader = nullptr;
        audioThumb.clear();
    }
    else
    {
        audioThumb.setReader (playbackRegionReader, reinterpret_cast<intptr_t> (playbackRegion));   // TODO JUCE_ARA better hash?
    }
}
