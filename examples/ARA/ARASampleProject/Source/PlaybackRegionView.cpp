#include "PlaybackRegionView.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

PlaybackRegionView::PlaybackRegionView (ARASampleProjectAudioProcessorEditor* editor, ARAPlaybackRegion* region)
: editorComponent (editor),
  playbackRegion (region),
  isSelected (false),
  audioThumbCache (1),
  audioThumb (128, audioFormatManger, audioThumbCache)
{
    audioThumb.addChangeListener (this);

    static_cast<ARADocument*> (editorComponent->getARADocumentController()->getDocument())->addListener (this);

    recreatePlaybackRegionReader();
}

PlaybackRegionView::~PlaybackRegionView()
{
    static_cast<ARADocument*>(playbackRegion->getRegionSequence()->getDocument())->removeListener (this);

    audioThumb.clear();
    audioThumb.removeChangeListener (this);
}

void PlaybackRegionView::paint (Graphics& g)
{
    Colour regionColour;
    // TODO JUCE_ARA Studio One uses black as the default color, which looks bad...
    if (const ARA::ARAColor* colour = playbackRegion->getColor())
        regionColour = Colour::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);

    g.fillAll (regionColour);
    g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
    g.drawRect (getLocalBounds());
    if (getLengthInSeconds() != 0.0)
    {
        g.setColour (regionColour.contrasting (0.7f));
        audioThumb.drawChannels (g, getLocalBounds(), 0.0, getLengthInSeconds(), 1.0);
    }
}

void PlaybackRegionView::changeListenerCallback (juce::ChangeBroadcaster* /*broadcaster*/)
{
    repaint();
}

double PlaybackRegionView::getStartInSeconds() const
{
    return playbackRegion->getStartInPlaybackTime();
}

double PlaybackRegionView::getLengthInSeconds() const
{
    return playbackRegion->getDurationInPlaybackTime();
}

double PlaybackRegionView::getEndInSeconds() const
{
    return playbackRegion->getEndInPlaybackTime();
}

void PlaybackRegionView::setIsSelected (bool value)
{
    bool needsRepaint = (value != isSelected);
    isSelected = value;
    if (needsRepaint)
        repaint();
}

void PlaybackRegionView::recreatePlaybackRegionReader()
{
    audioThumbCache.clear();

    // create a non-realtime playback region reader for our audio thumb
    auto documentController = static_cast<ARASampleProjectDocumentController*> (editorComponent->getARADocumentController());
    playbackRegionReader = documentController->createPlaybackRegionReader ({ playbackRegion }, true);
    audioThumb.setReader (playbackRegionReader, kAudioThumbHashCode);
}

void PlaybackRegionView::doEndEditing (ARADocument* /*document*/)
{
    if ((playbackRegionReader ==  nullptr) || ! playbackRegionReader->isValid())
    {
        recreatePlaybackRegionReader();
        editorComponent->setDirty();
    }
}
