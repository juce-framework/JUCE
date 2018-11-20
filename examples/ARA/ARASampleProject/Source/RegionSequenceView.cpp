#include "RegionSequenceView.h"
#include "ARASampleProjectPlaybackRenderer.h"
#include "ARASampleProjectDocumentController.h"
#include "ARASampleProjectAudioProcessorEditor.h"

void RegionSequenceView::paint (Graphics& g)
{
    Colour trackColour;
    if (const ARA::ARAColor* colour = regionSequence->getColor ())
        trackColour = Colour ((uint8) jmap (colour->r, 0.0f, 255.0f), (uint8) jmap (colour->g, 0.0f, 255.0f), (uint8) jmap (colour->b, 0.0f, 255.0f));

    g.fillAll (trackColour);
    g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
    g.drawRect (getLocalBounds());
    g.setColour (trackColour.contrasting (0.7f));
    if (audioThumb.getTotalLength() != 0.0)
        audioThumb.drawChannels (g, getLocalBounds(), startInSecs, audioThumb.getTotalLength(), 1.0);
    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (Font (12.0));
    g.drawText ("Track #" + String (regionSequence->getOrderIndex()) + ": " + regionSequence->getName(), 0, 0, getWidth(), getHeight(), juce::Justification::bottomLeft);
}

RegionSequenceView::RegionSequenceView (ARASampleProjectAudioProcessorEditor* editor, ARARegionSequence* sequence)
: isSelected (false)
, startInSecs (0.0)
, editorComponent (editor)
, regionSequence (sequence)
, audioThumbCache (1)
, audioThumb (128, audioFormatManger, audioThumbCache)
{
    recreateRegionSequneceReader();

    audioThumb.addChangeListener (this);

    regionSequence->addListener (this);
    static_cast<ARADocument*>(regionSequence->getDocument ())->addListener (this);
}

RegionSequenceView::~RegionSequenceView()
{
    if (regionSequence)
    {
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

void RegionSequenceView::recreateRegionSequneceReader()
{
    auto documentController = static_cast<ARASampleProjectDocumentController*> (regionSequence->getDocument()->getDocumentController());
    regionSequenceReader = documentController->createRegionSequenceReader (regionSequence);
    audioThumb.setReader (regionSequenceReader, kAudioThumbHashCode);
    audioThumbCache.loadThumb(audioThumb, kAudioThumbHashCode);

    startInSecs = audioThumb.getTotalLength();

// TODO JUCE_ARA do we need this?
//  audioThumbCache.loadThumb(audioThumb, kAudioThumbHashCode);
//  editorComponent->setDirty ();
}

void RegionSequenceView::doEndEditing (ARADocument* document)
{
    if (!regionSequenceReader || ! regionSequenceReader->isValid())
    {
        recreateRegionSequneceReader();

        audioThumbCache.clear ();
   }
}

void RegionSequenceView::didUpdateRegionSequenceProperties (ARARegionSequence* sequence)
{
    jassert (sequence == regionSequence);

    repaint();
}
