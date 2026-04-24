/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct AudioVisualiserComponent::ChannelInfo
{
    void setFifoSize (int numBlocks)
    {
        fifoStorage.clear();
        fifoStorage.resize ((size_t) numBlocks);
        fifo.setTotalSize (numBlocks);
    }

    void setBufferSize (int numBlocks)
    {
        levels.clear();
        levels.resize ((size_t) numBlocks);
        nextSample = 0;
    }

    void clear()
    {
        for (auto& c : levels)
            c = {};

        counter = 0;
        value = {};
    }

    void pushSamples (int blockSize, Span<const float> samples)
    {
        for (const auto& sample : samples)
            pushSample (blockSize, sample);
    }

    void pushSample (int blockSize, float sample)
    {
        if (++counter < blockSize)
        {
            value = value.getUnionWith (sample);
            return;
        }

        fifo.write (1).forEach ([this] (auto index)
        {
            fifoStorage[(size_t) index] = value;
        });

        counter = 0;
        value = Range (sample, sample);
    }

    void popPending()
    {
        fifo.read (fifo.getNumReady()).forEach ([this] (auto index)
        {
            levels[nextSample] = fifoStorage[(size_t) index];
            nextSample = (nextSample + 1) % levels.size();
        });
    }

    Range<float> value;
    int counter = 0;

    std::vector<Range<float>> fifoStorage;
    AbstractFifo fifo { 1 };

    std::vector<Range<float>> levels;
    size_t nextSample = 0;
};

//==============================================================================
AudioVisualiserComponent::AudioVisualiserComponent (int initialNumChannels)
    : numSamples (1024),
      inputSamplesPerBlock (256),
      backgroundColour (Colours::black),
      waveformColour (Colours::white)
{
    setOpaque (true);
    setNumChannels (initialNumChannels);
    setRepaintRate (60);
}

AudioVisualiserComponent::~AudioVisualiserComponent() = default;

void AudioVisualiserComponent::setNumChannels (int numChannels)
{
    channels.clear();

    for (int i = 0; i < numChannels; ++i)
        channels.add (new ChannelInfo);

    for (auto* channel : channels)
        channel->setBufferSize (numSamples);

    updateChannelFifoSizes();
}

void AudioVisualiserComponent::setBufferSize (int newNumSamples)
{
    numSamples = newNumSamples;

    for (auto* c : channels)
        c->setBufferSize (newNumSamples);
}

void AudioVisualiserComponent::clear()
{
    for (auto* c : channels)
        c->clear();
}

void AudioVisualiserComponent::pushBuffer (const float* const* d, int numChannels, int num)
{
    numChannels = jmin (numChannels, channels.size());

    for (auto i = 0; i < numChannels; ++i)
        channels.getUnchecked (i)->pushSamples (inputSamplesPerBlock, { d[i], (size_t) num });
}

void AudioVisualiserComponent::pushBuffer (const AudioBuffer<float>& buffer)
{
    pushBuffer (buffer.getArrayOfReadPointers(),
                buffer.getNumChannels(),
                buffer.getNumSamples());
}

void AudioVisualiserComponent::pushBuffer (const AudioSourceChannelInfo& buffer)
{
    auto numChannels = jmin (buffer.buffer->getNumChannels(), channels.size());

    for (auto i = 0; i < numChannels; ++i)
    {
        channels.getUnchecked (i)->pushSamples (inputSamplesPerBlock,
                                                { buffer.buffer->getReadPointer (i, buffer.startSample), (size_t) buffer.numSamples });
    }
}

void AudioVisualiserComponent::pushSample (const float* d, int numChannels)
{
    numChannels = jmin (numChannels, channels.size());

    for (auto i = 0; i < numChannels; ++i)
        channels.getUnchecked (i)->pushSample (inputSamplesPerBlock, d[i]);
}

void AudioVisualiserComponent::setSamplesPerBlock (int newSamplesPerPixel) noexcept
{
    jassert (newSamplesPerPixel > 0);
    inputSamplesPerBlock = newSamplesPerPixel;
}

void AudioVisualiserComponent::setRepaintRate (int frequencyInHz)
{
    startTimerHz (frequencyInHz);
    updateChannelFifoSizes();
}

void AudioVisualiserComponent::timerCallback()
{
    for (auto* channel : channels)
        channel->popPending();

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

    auto r = getLocalBounds().toFloat();
    auto channelHeight = r.getHeight() / (float) channels.size();

    g.setColour (waveformColour);

    for (auto* c : channels)
    {
        paintChannel (g,
                      r.removeFromTop (channelHeight),
                      c->levels.data(),
                      (int) c->levels.size(),
                      (int) c->nextSample);
    }
}

void AudioVisualiserComponent::getChannelAsPath (Path& path, const Range<float>* levels,
                                                 int numLevels, int nextSample)
{
    path.preallocateSpace (4 * numLevels + 8);

    for (int i = 0; i < numLevels; ++i)
    {
        auto level = -(levels[(nextSample + i) % numLevels].getEnd());

        if (i == 0)
            path.startNewSubPath (0.0f, level);
        else
            path.lineTo ((float) i, level);
    }

    for (int i = numLevels; --i >= 0;)
        path.lineTo ((float) i, -(levels[(nextSample + i) % numLevels].getStart()));

    path.closeSubPath();
}

void AudioVisualiserComponent::paintChannel (Graphics& g, Rectangle<float> area,
                                             const Range<float>* levels, int numLevels, int nextSample)
{
    Path p;
    getChannelAsPath (p, levels, numLevels, nextSample);

    g.fillPath (p, AffineTransform::fromTargetPoints (0.0f, -1.0f,               area.getX(), area.getY(),
                                                      0.0f, 1.0f,                area.getX(), area.getBottom(),
                                                      (float) numLevels, -1.0f,  area.getRight(), area.getY()));
}

void AudioVisualiserComponent::updateChannelFifoSizes()
{
    // This is intended to make sure that the fifo for each channel is large enough to store
    // at least one frame's incoming blocks with some extra padding to avoid dropping too much info
    // if a frame is delayed.

    const auto maxSampleRate = 192'000;
    const auto maxBlocksPerSecond = inputSamplesPerBlock > 0
                                  ? ((maxSampleRate + inputSamplesPerBlock - 1) / inputSamplesPerBlock)
                                  : 1;
    const auto maxBlocksPerRepaint = (maxBlocksPerSecond * getTimerInterval() + 999) / 1000;
    const auto paddedBlocksPerRepaint = 10 + maxBlocksPerRepaint;

    for (auto* channel : channels)
        channel->setFifoSize (paddedBlocksPerRepaint);
}

} // namespace juce
