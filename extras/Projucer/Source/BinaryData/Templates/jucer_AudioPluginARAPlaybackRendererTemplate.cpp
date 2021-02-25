/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for an ARA playback renderer implementation.

  ==============================================================================
*/

%%araplaybackrenderer_headers%%

//==============================================================================
void %%araplaybackrenderer_class_name%%::prepareToPlay (double rate, int maxSamplesPerBlock, int numChans)
{
    sampleRate = rate;
    maximumSamplesPerBlock = maxSamplesPerBlock;
    numChannels = numChans;
}

void %%araplaybackrenderer_class_name%%::releaseResources()
{
}

//==============================================================================
bool %%araplaybackrenderer_class_name%%::processBlock (juce::AudioBuffer<float>& buffer, bool isNonRealtime, const juce::AudioPlayHead::CurrentPositionInfo& positionInfo) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();
    jassert (numSamples <= maximumSamplesPerBlock);
    jassert (numChannels == numChannels);
    const auto timeInSamples = positionInfo.timeInSamples;
    const auto isPlaying = positionInfo.isPlaying;

    bool success = true;
    bool didRenderAnyRegion = false;
    if (isPlaying)
    {
        const auto blockRange = juce::Range<juce::int64>::withStartAndLength (timeInSamples, numSamples);
        for (const auto& playbackRegion : getPlaybackRegions())
        {
            // Evaluate region borders in song time, calculate sample range to render in song time.
            // Note that this example does not use head- or tailtime, so the includeHeadAndTail
            // parameter is set to false here - this might need to be adjusted in actual plug-ins.
            const auto playbackSampleRange = playbackRegion->getSampleRange (sampleRate, false);
            auto renderRange = blockRange.getIntersectionWith (playbackSampleRange);
            if (renderRange.isEmpty())
                continue;

            // Now calculate samples in renderRange for this playback region based on the ARA model graph.
            // If didRenderAnyRegion is true, add the region's output samples in renderRange to the buffer.
            // If it is false, the buffer must be initialized so you replace
            const int numSamples = (int) renderRange.getLength();
            const int startInBuffer = (int) (renderRange.getStart() - blockRange.getStart());
            for (int c = 0; c < numChannels; ++c)
            {
                auto* channelData = buffer.getWritePointer (c);
                for (int i = 0; i < numSamples; ++i)
                {
                    // ... calculate region output sample at index renderRange.getStart() + i
                    float sample = 0.0f;

                    if (didRenderAnyRegion)
                        channelData[startInBuffer + i] += sample;
                    else
                        channelData[startInBuffer + i] = sample;
                }
            }

            // If rendering first region, clear any excess at start or end of the region.
            if (! didRenderAnyRegion)
            {
                if (startInBuffer != 0)
                    buffer.clear (0, startInBuffer);

                const int endInBuffer = startInBuffer + numSamples;
                const int remainingSamples = numSamples - endInBuffer;
                if (remainingSamples != 0)
                    buffer.clear (endInBuffer, remainingSamples);

                didRenderAnyRegion = true;
            }
        }
    }

    // If no playback or no region did intersect, clear buffer now.
    if (! didRenderAnyRegion)
        buffer.clear();

    return success;
}
