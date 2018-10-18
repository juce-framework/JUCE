#include "AudioView.h"
#include "juce_audio_plugin_client/ARA/juce_ARARegionSequence.h"

void AudioView::paint (Graphics& g)
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
    g.drawText ("Track #" + order + ": " + name, 0, 0, getWidth(), getHeight(), juce::Justification::bottomLeft);
}

AudioView::AudioView() :
 isSelected (false)
,startInSecs (0.0)
,audioThumbCache (1)
,audioThumb (128, audioFormatManger, audioThumbCache)
{
    trackColour = Colours::beige;
}

AudioView::AudioView (ARA::PlugIn::RegionSequence& sequence)
: AudioView()
{
    name = String (sequence.getName());
    order = String (sequence.getOrderIndex());
    audioThumb.setReader ((dynamic_cast<ARARegionSequence&>(sequence)).newReader(), 1);
    startInSecs = audioThumb.getTotalLength();
    for (auto region : sequence.getPlaybackRegions())
    {
        startInSecs = std::min (startInSecs, region->getStartInPlaybackTime());
    }
    audioThumb.addChangeListener (this);

    if (const ARA::ARAColor* colour = sequence.getColor())
    {
        trackColour = Colour ((uint8)jmap (colour->r, 0.0f, 255.0f), (uint8)jmap (colour->g, 0.0f, 255.0f), (uint8)jmap (colour->b, 0.0f, 255.0f));
    }
}

AudioView::~AudioView()
{
    audioThumb.clear();
    audioThumb.removeChangeListener (this);
}

void AudioView::changeListenerCallback (juce::ChangeBroadcaster* /*broadcaster*/)
{
    repaint();
}

double AudioView::getStartInSecs()
{
    return startInSecs;
}

double AudioView::getLengthInSecs()
{
    return audioThumb.getTotalLength() - startInSecs;
}

void AudioView::setIsSelected(bool value)
{
    isSelected = value;
}
