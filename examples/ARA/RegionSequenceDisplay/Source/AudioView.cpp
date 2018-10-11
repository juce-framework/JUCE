#include "AudioView.h"
#include "juce_audio_plugin_client/ARA/juce_ARARegionSequence.h"

void AudioView::paint (Graphics& g)
{
    g.fillAll (trackColour);
    g.setColour (_isSelected ? juce::Colours::yellow : juce::Colours::black);
    g.drawRect (getLocalBounds());
    g.setColour (trackColour.contrasting (0.7f));
    if (_audioThumb.getTotalLength())
    {
        _audioThumb.drawChannels (g, getLocalBounds(), _startInSecs, _audioThumb.getTotalLength(), 1.0);
    }
    g.setColour (trackColour.contrasting (1.0f));
    g.setFont (Font (12.0));
    g.drawText ("Track #" + order + ": " + name, 0, 0, getWidth(), getHeight(), juce::Justification::bottomLeft);
}

AudioView::AudioView() :
 _isSelected (false)
,_startInSecs (0.0)
,_audioThumbCache (1)
,_audioThumb (128, _audioFormatManger, _audioThumbCache)
{
    trackColour = Colours::beige;
}

AudioView::AudioView (ARA::PlugIn::RegionSequence& sequence)
: AudioView()
{
    name = String (sequence.getName());
    order = String (sequence.getOrderIndex());
    _audioThumb.setReader ((dynamic_cast<ARARegionSequence&>(sequence)).newReader(), 1);
    _startInSecs = _audioThumb.getTotalLength();
    for (auto region : sequence.getPlaybackRegions())
    {
        _startInSecs = std::min (_startInSecs, region->getStartInPlaybackTime());
    }
    _audioThumb.addChangeListener (this);

    if (const ARA::ARAColor* colour = sequence.getColor())
    {
        trackColour = Colour ((uint8)jmap (colour->r, 0.0f, 255.0f), (uint8)jmap (colour->g, 0.0f, 255.0f), (uint8)jmap (colour->b, 0.0f, 255.0f));
    }
}

AudioView::~AudioView()
{
    _audioThumb.clear();
    _audioThumb.removeChangeListener (this);
}

void AudioView::changeListenerCallback (juce::ChangeBroadcaster* broadcaster)
{
    repaint();
}

double AudioView::getStartInSecs()
{
    return _startInSecs;
}

double AudioView::getLengthInSecs()
{
    return _audioThumb.getTotalLength() - _startInSecs;
}

void AudioView::isSelected(bool value)
{
    _isSelected = value;
}
