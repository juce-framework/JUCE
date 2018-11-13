#include "RegionSequenceView.h"
#include "PluginARARegionSequence.h"

void RegionSequenceView::paint (Graphics& g)
{
    g.fillAll (trackColour);
    g.setColour (isSelected ? juce::Colours::yellow : juce::Colours::black);
    g.drawRect (getLocalBounds());
    g.setColour (trackColour.contrasting (0.7f));
    if (audioThumb.getTotalLength() != 0.0)
    {
        audioThumb.drawChannels (g, getLocalBounds(), startInSecs, audioThumb.getTotalLength(), 1.0);
    }
    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (Font (12.0));
    g.drawText ("Track #" + orderIndex + ": " + name, 0, 0, getWidth(), getHeight(), juce::Justification::bottomLeft);
}


RegionSequenceView::RegionSequenceView (ARA::PlugIn::RegionSequence& sequence)
: isSelected (false)
, startInSecs (0.0)
, regionSequence (nullptr)
, audioThumbCache (1)
, audioThumb (128, audioFormatManger, audioThumbCache)
{
    name = String (sequence.getName());
    orderIndex = String (sequence.getOrderIndex());
    audioThumb.addChangeListener (this);
    audioThumb.setReader ((dynamic_cast<ARASampleProjectRegionSequence&> (sequence)).newReader(), 1);
    startInSecs = audioThumb.getTotalLength();
    
    regionSequence = &sequence;
    for (auto region : sequence.getPlaybackRegions())
    {
        startInSecs = std::min (startInSecs, region->getStartInPlaybackTime());
    }

    if (const ARA::ARAColor* colour = sequence.getColor())
    {
        trackColour = Colour ((uint8)jmap (colour->r, 0.0f, 255.0f), (uint8)jmap (colour->g, 0.0f, 255.0f), (uint8)jmap (colour->b, 0.0f, 255.0f));
    }
}

RegionSequenceView::~RegionSequenceView()
{
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
        repaint ();
}

bool RegionSequenceView::getIsSelected () const
{
    return isSelected;
}
