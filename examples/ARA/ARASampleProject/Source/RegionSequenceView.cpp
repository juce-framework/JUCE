#include "RegionSequenceView.h"
#include "ARASampleProjectPlaybackRenderer.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

void RegionSequenceView::paint (Graphics& g)
{
    g.fillAll (trackColour);
    g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
    g.drawRect (getLocalBounds());
    g.setColour (trackColour.contrasting (0.7f));
    if (audioThumb.getTotalLength() != 0.0)
        audioThumb.drawChannels (g, getLocalBounds(), startInSecs, audioThumb.getTotalLength(), 1.0);
    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (Font (12.0));
    g.drawText ("Track #" + orderIndex + ": " + name, 0, 0, getWidth(), getHeight(), juce::Justification::bottomLeft);
}

RegionSequenceView::RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence)
: isSelected (false)
, startInSecs (0.0)
, editorComponent (editor)
, regionSequence (sequence)
, audioThumbCache (1)
, audioThumb (128, audioFormatManger, audioThumbCache)
{
    auto documentController = static_cast<ARASampleProjectDocumentController*> (regionSequence->getDocument()->getDocumentController());
    name = String (regionSequence->getName());
    orderIndex = String (regionSequence->getOrderIndex());
    audioThumb.addChangeListener (this);
    audioThumb.setReader (documentController->createRegionSequenceReader (regionSequence), kAudioThumbHashCode);
    startInSecs = audioThumb.getTotalLength();

    regionSequence->addListener (this);
    static_cast<ARADocument*>(regionSequence->getDocument ())->addListener (this);

    for (auto region : regionSequence->getPlaybackRegions ())
    {
        startInSecs = jmin (startInSecs, region->getStartInPlaybackTime ());
        static_cast<ARAPlaybackRegion*> (region)->addListener (this);
    }

    if (const ARA::ARAColor* colour = regionSequence->getColor())
        trackColour = Colour ((uint8)jmap (colour->r, 0.0f, 255.0f), (uint8)jmap (colour->g, 0.0f, 255.0f), (uint8)jmap (colour->b, 0.0f, 255.0f));
}

RegionSequenceView::~RegionSequenceView()
{
    if (regionSequence)
    {
        for (auto region : regionSequence->getPlaybackRegions ())
            static_cast<ARAPlaybackRegion*> (region)->removeListener (this);
        
        regionSequence->removeListener(this);
        static_cast<ARADocument*>(regionSequence->getDocument ())->removeListener (this);
    }

    audioThumb.clear();
    audioThumb.removeChangeListener (this);
}

void RegionSequenceView::changeListenerCallback (juce::ChangeBroadcaster* /*broadcaster*/)
{
    repaint();
}

double RegionSequenceView::getStartInSecs()
{
    return startInSecs;
}

double RegionSequenceView::getLengthInSecs()
{
    return audioThumb.getTotalLength() - startInSecs;
}

void RegionSequenceView::setIsSelected (bool value)
{
    bool needsRepaint = (value != isSelected);
    isSelected = value;
    if (needsRepaint)
        repaint();
    
}

bool RegionSequenceView::getIsSelected() const
{
    return isSelected;
}

void RegionSequenceView::doEndEditing (ARADocument* document)
{
    if (audioThumbCache.loadThumb(audioThumb, kAudioThumbHashCode) == false)
    {
        auto documentController = static_cast<ARASampleProjectDocumentController*> (document->getDocumentController ());
        audioThumb.setReader (documentController->createRegionSequenceReader (regionSequence), kAudioThumbHashCode);
    }
}

void RegionSequenceView::willUpdatePlaybackRegionProperties (ARAPlaybackRegion* playbackRegion, ARAPlaybackRegion::PropertiesPtr newProperties)
{
    if ((playbackRegion->getStartInPlaybackTime () != newProperties->startInPlaybackTime) ||
        (playbackRegion->getDurationInPlaybackTime () != newProperties->durationInPlaybackTime))
    {
        editorComponent->setDirty ();
        audioThumbCache.clear ();

        // TODO JUCE_ARA
        // put this in a function to avoid repetition
        startInSecs = std::numeric_limits<double>::max ();
        for (auto region : regionSequence->getPlaybackRegions ())
            startInSecs = jmin (startInSecs, newProperties->startInPlaybackTime);
    }
}

void RegionSequenceView::willDestroyPlaybackRegion (ARAPlaybackRegion* playbackRegion)
{
    editorComponent->setDirty ();
    audioThumbCache.clear ();
    playbackRegion->removeListener (this);
}

void RegionSequenceView::didUpdateRegionSequenceProperties (ARARegionSequence* /*sequence*/)
{
    if (const ARA::ARAColor* colour = regionSequence->getColor ())
        trackColour = Colour ((uint8) jmap (colour->r, 0.0f, 255.0f), (uint8) jmap (colour->g, 0.0f, 255.0f), (uint8) jmap (colour->b, 0.0f, 255.0f));
    repaint();
}

void RegionSequenceView::willRemovePlaybackRegionFromRegionSequence (ARARegionSequence* /*sequence*/, ARAPlaybackRegion* playbackRegion)
{
    playbackRegion->removeListener (this);
}

void RegionSequenceView::didAddPlaybackRegionToRegionSequence (ARARegionSequence* /*sequence*/, ARAPlaybackRegion* playbackRegion)
{
    playbackRegion->addListener (this);
}