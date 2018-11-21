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

    // TODO JUCE_ARA
    // Getting the document / document controller as an ARADocument should be easier
    static_cast<ARADocument*>(playbackRegion->getRegionSequence ()->getDocument ())->addListener (this);

    recreatePlaybackRegionReader ();
}

PlaybackRegionView::~PlaybackRegionView ()
{
    static_cast<ARADocument*>(playbackRegion->getRegionSequence ()->getDocument ())->removeListener (this);

    audioThumb.clear ();
    audioThumb.removeChangeListener (this);
}

void PlaybackRegionView::paint (Graphics& g)
{
    Colour regionColour;
    if (const ARA::ARAColor* colour = playbackRegion->getColor ())
        regionColour = Colour ((uint8) jmap (colour->r, 0.0f, 255.0f), (uint8) jmap (colour->g, 0.0f, 255.0f), (uint8) jmap (colour->b, 0.0f, 255.0f));

    // TODO JUCE_ARA
    // Studio One uses black as the default color, which looks bad
    //g.fillAll (regionColour);
    g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
    g.drawRect (getLocalBounds ());
    if (getLengthInSeconds () != 0.0)
    {
        g.setColour (regionColour.contrasting (0.7f));
        audioThumb.drawChannels (g, getLocalBounds (), getStartInSeconds (), getEndInSeconds(), 1.0);
    }
}

void PlaybackRegionView::changeListenerCallback (juce::ChangeBroadcaster* /*broadcaster*/)
{
    repaint ();
}

double PlaybackRegionView::getStartInSeconds ()
{
    return playbackRegion->getStartInPlaybackTime ();
}

double PlaybackRegionView::getEndInSeconds ()
{
    return playbackRegion->getEndInPlaybackTime ();
}

double PlaybackRegionView::getLengthInSeconds ()
{
    return getEndInSeconds () - getStartInSeconds ();
}

void PlaybackRegionView::setIsSelected (bool value)
{
    bool needsRepaint = (value != isSelected);
    isSelected = value;
    if (needsRepaint)
        repaint ();
}

bool PlaybackRegionView::getIsSelected () const
{
    return isSelected;
}

void PlaybackRegionView::recreatePlaybackRegionReader ()
{
    audioThumbCache.clear ();

    auto documentController = static_cast<ARASampleProjectDocumentController*> (playbackRegion->getRegionSequence()->getDocument ()->getDocumentController ());
    playbackRegionReader = documentController->createPlaybackRegionReader ({ playbackRegion });
    audioThumb.setReader (playbackRegionReader, kAudioThumbHashCode);
}

void PlaybackRegionView::doEndEditing (ARADocument* /*document*/)
{
    if (!playbackRegionReader || !playbackRegionReader->isValid ())
    {
        recreatePlaybackRegionReader ();
        editorComponent->setDirty ();
    }
}
