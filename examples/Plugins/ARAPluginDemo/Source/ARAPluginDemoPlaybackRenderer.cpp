#include "ARAPluginDemoPlaybackRenderer.h"
#include "ARAPluginDemoAudioModification.h"

//==============================================================================
void PluginDemoPlaybackRenderer::prepareToPlay (double rate, int maxSamplesPerBlock, int numChans, bool alwaysNonRealtime)
{
    sampleRate = rate;
    maximumSamplesPerBlock = maxSamplesPerBlock;
    numChannels = numChans;
    useBufferedAudioSourceReader = ! alwaysNonRealtime;

    audioSourceReaders.clear();

    for (const auto& playbackRegion : getPlaybackRegions())
    {
        auto audioSource = playbackRegion->getAudioModification()->getAudioSource();
        if (audioSourceReaders.count (audioSource) == 0)
        {
            juce::AudioFormatReader* sourceReader = new juce::ARAAudioSourceReader (audioSource);

            if (useBufferedAudioSourceReader)
            {
                // If we're being used in real-time, wrap our source reader in a buffering
                // reader to avoid blocking while reading samples in processBlock.
                const int readAheadSize = juce::jmax (4 * maximumSamplesPerBlock, juce::roundToInt (2.0 * sampleRate));
                sourceReader = new juce::BufferingAudioReader (sourceReader, *sharedTimesliceThread, readAheadSize);
            }

            audioSourceReaders.emplace (audioSource, sourceReader);
        }
    }

    if (getPlaybackRegions().size() > 1)
        tempBuffer.reset (new juce::AudioBuffer<float> (numChannels, maximumSamplesPerBlock));
    else
        tempBuffer.reset();
}

void PluginDemoPlaybackRenderer::releaseResources()
{
    audioSourceReaders.clear();
    tempBuffer.reset();
}

//==============================================================================
bool PluginDemoPlaybackRenderer::processBlock (juce::AudioBuffer<float>& buffer, bool isNonRealtime, const juce::AudioPlayHead::CurrentPositionInfo& positionInfo) noexcept
{
    const auto numSamples = buffer.getNumSamples();
    jassert (numSamples <= maximumSamplesPerBlock);
    jassert (numChannels == buffer.getNumChannels());
    jassert (isNonRealtime || useBufferedAudioSourceReader);
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

            // Evaluate region borders in modification/source time and calculate offset between
            // song and source samples, then clip song samples accordingly
            // (if an actual plug-in supports time stretching, this must be taken into account here).
            juce::Range<juce::int64> modificationSampleRange { playbackRegion->getStartInAudioModificationSamples(), playbackRegion->getEndInAudioModificationSamples() };
            const auto modificationSampleOffset = modificationSampleRange.getStart() - playbackSampleRange.getStart();

            renderRange = renderRange.getIntersectionWith (modificationSampleRange.movedToStartAt (playbackSampleRange.getStart()));
            if (renderRange.isEmpty())
                continue;

            // Get the audio source for the region and find the reader for that source.
            // This simplified example code only produces audio if sample rate and channel count match -
            // proper plug-in would need to do conversion, see ARA SDK documentation.
            const auto audioSource = playbackRegion->getAudioModification()->getAudioSource();
            const auto readerIt = audioSourceReaders.find (audioSource);
            if ((audioSource->getChannelCount() != numChannels) ||
                (audioSource->getSampleRate() != sampleRate) ||
                (readerIt == audioSourceReaders.end()))
            {
                success = false;
                continue;
            }
            const auto& reader = readerIt->second;

            // If we're using a buffering reader, set the appropriate timeout.
            if (useBufferedAudioSourceReader)
            {
                jassert (dynamic_cast<juce::BufferingAudioReader*> (reader.get()) != nullptr);
                auto bufferingReader = static_cast<juce::BufferingAudioReader*> (reader.get());
                bufferingReader->setReadTimeout (isNonRealtime ? 100 : 0);
            }

            // Calculate buffer offsets.
            const int numSamplesToRead = (int) renderRange.getLength();
            const int startInBuffer = (int) (renderRange.getStart() - blockRange.getStart());
            auto startInSource = renderRange.getStart() + modificationSampleOffset;

            // Read samples:
            // first region can write directly into output, later regions need to use local buffer.
            auto& readBuffer = (didRenderAnyRegion) ? *tempBuffer : buffer;
            if (! reader->read (&readBuffer, startInBuffer, numSamplesToRead, startInSource, true, true))
            {
                success = false;
                continue;
            }

            // Apply dim if enabled
            if (playbackRegion->getAudioModification<ARAPluginDemoAudioModification>()->isDimmed())
                readBuffer.applyGain (startInBuffer, numSamplesToRead, 0.25f);  // dim by about 12 dB

            if (didRenderAnyRegion)
            {
                // Mix local buffer into the output buffer.
                for (int c = 0; c < numChannels; ++c)
                    buffer.addFrom (c, startInBuffer, *tempBuffer, c, startInBuffer, numSamplesToRead);
            }
            else
            {
                // Clear any excess at start or end of the region.
                if (startInBuffer != 0)
                    buffer.clear (0, startInBuffer);

                const int endInBuffer = startInBuffer + numSamplesToRead;
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
