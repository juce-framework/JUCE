#include "PlaybackRegionView.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

PlaybackRegionView::PlaybackRegionView (ARASampleProjectAudioProcessorEditor* editor, ARAPlaybackRegion* region)
: editorComponent (editor),
  playbackRegion (region),
  audioThumbCache (1),
  audioThumb (128, audioFormatManger, audioThumbCache)
{
    audioThumb.addChangeListener (this);

    static_cast<ARADocument*> (playbackRegion->getRegionSequence()->getDocument())->addListener (this);
    static_cast<ARAAudioSource*> (playbackRegion->getAudioModification()->getAudioSource())->addListener (this);
    playbackRegion->addListener (this);

    recreatePlaybackRegionReader();
}

PlaybackRegionView::~PlaybackRegionView()
{
    playbackRegion->removeListener (this);
    static_cast<ARAAudioSource*>(playbackRegion->getAudioModification()->getAudioSource())->removeListener (this);
    static_cast<ARADocument*> (playbackRegion->getRegionSequence()->getDocument())->removeListener (this);

    audioThumb.clear();
    audioThumb.removeChangeListener (this);
}

void PlaybackRegionView::paint (Graphics& g)
{
    Colour regionColour;
    const ARA::ARAColor* colour = playbackRegion->getColor();
    if (colour == nullptr)
        colour = playbackRegion->getRegionSequence()->getColor();
    if (colour != nullptr)
    {
        regionColour = Colour::fromFloatRGBA (colour->r, colour->g, colour->b, 1.0f);
        g.fillAll (regionColour);
    }

    g.setColour (isSelected ? Colours::yellow : Colours::black);
    g.drawRect (getLocalBounds());

    if (playbackRegion->getAudioModification()->getAudioSource()->isSampleAccessEnabled())
    {
        double duration = playbackRegion->getDurationInPlaybackTime();
        if (duration != 0.0)
        {
            g.setColour (regionColour.contrasting (0.7f));
            audioThumb.drawChannels (g, getLocalBounds(), 0.0, duration, 1.0);
        }
    }
    else
    {
        g.setColour (regionColour.contrasting (1.0f));
        g.setFont (Font (12.0));
        g.drawText ("Access Disabled", getBounds(), Justification::centred);
    }
}

void PlaybackRegionView::changeListenerCallback (ChangeBroadcaster* /*broadcaster*/)
{
    repaint();
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

void PlaybackRegionView::didEnableAudioSourceSamplesAccess (ARAAudioSource* audioSource, bool enable)
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

void PlaybackRegionView::setIsSelected (bool value)
{
    if (isSelected != value)
    {
        isSelected = value;
        repaint();
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
        audioThumb.clear ();
    }
    else
    {
        audioThumb.setReader (playbackRegionReader, reinterpret_cast<intptr_t> (playbackRegion));   // TODO JUCE_ARA better hash?
    }
}
