/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

struct AudioVisualiserComponent::ChannelInfo
{
    ChannelInfo (AudioVisualiserComponent& o, int bufferSize)
       : owner (o), nextSample (0), subSample (0)
    {
        setBufferSize (bufferSize);
        clear();
    }

    void clear() noexcept
    {
        for (int i = 0; i < levels.size(); ++i)
            levels.getReference(i) = Range<float>();

        value = Range<float>();
        subSample = 0;
    }

    void pushSamples (const float* inputSamples, const int num) noexcept
    {
        for (int i = 0; i < num; ++i)
            pushSample (inputSamples[i]);
    }

    void pushSample (const float newSample) noexcept
    {
        if (--subSample <= 0)
        {
            nextSample %= levels.size();
            levels.getReference (nextSample++) = value;
            subSample = owner.getSamplesPerBlock();
            value = Range<float> (newSample, newSample);
        }
        else
        {
            value = value.getUnionWith (newSample);
        }
    }

    void setBufferSize (int newSize)
    {
        levels.removeRange (newSize, levels.size());
        levels.insertMultiple (-1, Range<float>(), newSize - levels.size());

        if (nextSample >= newSize)
            nextSample = 0;
    }

    AudioVisualiserComponent& owner;
    Array<Range<float> > levels;
    Range<float> value;
    int nextSample, subSample;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelInfo);
};

//==============================================================================
AudioVisualiserComponent::AudioVisualiserComponent (const int initialNumChannels)
  : numSamples (1024),
    inputSamplesPerBlock (256),
    backgroundColour (Colours::black),
    waveformColour (Colours::white)
{
    setOpaque (true);
    setNumChannels (initialNumChannels);
    setRepaintRate (60);
}

AudioVisualiserComponent::~AudioVisualiserComponent()
{
}

void AudioVisualiserComponent::setNumChannels (const int numChannels)
{
    channels.clear();

    for (int i = 0; i < numChannels; ++i)
        channels.add (new ChannelInfo (*this, numSamples));
}

void AudioVisualiserComponent::setBufferSize (int newNumSamples)
{
    numSamples = newNumSamples;

    for (int i = 0; i < channels.size(); ++i)
        channels.getUnchecked(i)->setBufferSize (newNumSamples);
}

void AudioVisualiserComponent::clear()
{
    for (int i = 0; i < channels.size(); ++i)
        channels.getUnchecked(i)->clear();
}

void AudioVisualiserComponent::pushBuffer (const float** d, int numChannels, int num)
{
    numChannels = jmin (numChannels, channels.size());

    for (int i = 0; i < numChannels; ++i)
        channels.getUnchecked(i)->pushSamples (d[i], num);
}

void AudioVisualiserComponent::pushBuffer (const AudioSampleBuffer& buffer)
{
    pushBuffer (buffer.getArrayOfReadPointers(),
                buffer.getNumChannels(),
                buffer.getNumSamples());
}

void AudioVisualiserComponent::pushBuffer (const AudioSourceChannelInfo& buffer)
{
    const int numChannels = jmin (buffer.buffer->getNumChannels(), channels.size());

    for (int i = 0; i < numChannels; ++i)
        channels.getUnchecked(i)->pushSamples (buffer.buffer->getReadPointer (i, buffer.startSample),
                                               buffer.numSamples);
}

void AudioVisualiserComponent::pushSample (const float* d, int numChannels)
{
    numChannels = jmin (numChannels, channels.size());

    for (int i = 0; i < numChannels; ++i)
        channels.getUnchecked(i)->pushSample (d[i]);
}

void AudioVisualiserComponent::setSamplesPerBlock (int newSamplesPerPixel) noexcept
{
    inputSamplesPerBlock = newSamplesPerPixel;
}

void AudioVisualiserComponent::setRepaintRate (int frequencyInHz)
{
    startTimerHz (frequencyInHz);
}

void AudioVisualiserComponent::timerCallback()
{
    repaint();
}

void AudioVisualiserComponent::setColours (Colour bk, Colour fg) noexcept
{
    backgroundColour = bk;
    waveformColour = fg;
    repaint();
}

void AudioVisualiserComponent::paint (Graphics& g)
{
    g.fillAll (backgroundColour);

    Rectangle<float> r (getLocalBounds().toFloat());
    const float channelHeight = r.getHeight() / channels.size();

    g.setColour (waveformColour);

    for (int i = 0; i < channels.size(); ++i)
    {
        const ChannelInfo& c = *channels.getUnchecked(i);

        paintChannel (g, r.removeFromTop (channelHeight),
                      c.levels.begin(), c.levels.size(), c.nextSample);
    }
}

void AudioVisualiserComponent::getChannelAsPath (Path& path, const Range<float>* levels, int numLevels, int nextSample)
{
    path.preallocateSpace (4 * numLevels + 8);

    for (int i = 0; i < numLevels; ++i)
    {
        const float level = -(levels[(nextSample + i) % numLevels].getEnd());

        if (i == 0)
            path.startNewSubPath (0.0f, level);
        else
            path.lineTo ((float) i, level);
    }

    for (int i = numLevels; --i >= 0;)
        path.lineTo ((float) i, -(levels[(nextSample + i) % numLevels].getStart()));

    path.closeSubPath();
}

void AudioVisualiserComponent::paintChannel (Graphics& g, Rectangle<float> bounds,
                                             const Range<float>* levels, int numLevels, int nextSample)
{
    Path p;
    getChannelAsPath (p, levels, numLevels, nextSample);

    g.fillPath (p, AffineTransform::fromTargetPoints (0.0f, -1.0f,      bounds.getX(), bounds.getY(),
                                                      0.0f, 1.0f,       bounds.getX(), bounds.getBottom(),
                                                      numLevels, -1.0f, bounds.getRight(), bounds.getY()));
}
