#include "ARAPluginDemoPlaybackRenderer.h"
#include "ARAPluginDemoAudioModification.h"

//==============================================================================
void PluginDemoPlaybackRenderer::prepareToPlay (const ProcessSpec& spec)
{
    processSpec = spec;

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
                const int readAheadSize = juce::jmax (4 * spec.maximumBlockSize, juce::roundToInt (2.0 * processSpec.sampleRate));
                sourceReader = new juce::BufferingAudioReader (sourceReader, *sharedTimesliceThread, readAheadSize);
            }

            audioSourceReaders.emplace (audioSource, sourceReader);
        }
    }

    if (getPlaybackRegions().size() > 1)
        tempBuffer.reset (new juce::AudioBuffer<float> (processSpec.numChannels, processSpec.maximumBlockSize));
    else
        tempBuffer.reset();
}

void PluginDemoPlaybackRenderer::releaseResources()
{
    audioSourceReaders.clear();
    tempBuffer.reset();
}

//==============================================================================
bool PluginDemoPlaybackRenderer::processBlock (juce::AudioBuffer<float>& buffer, const ProcessContext& context) noexcept
{
    const auto numSamples  = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();
    jassert (numSamples <= processSpec.maximumBlockSize);
    jassert (numChannels == processSpec.numChannels);
    jassert (context.isNonRealtime || useBufferedAudioSourceReader);
    const auto timeInSamples = context.positionInfo.timeInSamples;
    const auto isPlaying = context.positionInfo.isPlaying;

    bool success = true;
    bool didRenderFirstRegion = false;
    if (isPlaying)
    {
        const auto sampleStart = timeInSamples;
        const auto sampleEnd = timeInSamples + numSamples;
        for (const auto& playbackRegion : getPlaybackRegions())
        {
            // Get the audio source for the region and find the reader for that source.
            // This simplified example code only produces audio if sample rate and channel count match -
            // proper plug-in would need to do conversion, see ARA SDK documentation.
            const auto audioSource = playbackRegion->getAudioModification()->getAudioSource();
            const auto readerIt = audioSourceReaders.find (audioSource);
            if ((audioSource->getChannelCount() != processSpec.numChannels) ||
                (audioSource->getSampleRate() != processSpec.sampleRate) ||
                (readerIt == audioSourceReaders.end()))
            {
                success = false;
                continue;
            }
            const auto& reader = readerIt->second;

            // Evaluate region borders in song time, calculate sample range to copy in song time.
            // Note that this example does not use head- or tailtime, so the includeHeadAndTail
            // could be set to false in an actual plug-in to optimze.
            const auto regionStartSample = playbackRegion->getStartInPlaybackSamples (processSpec.sampleRate);
            if (sampleEnd <= regionStartSample)
                continue;

            const auto regionEndSample = playbackRegion->getEndInPlaybackSamples (processSpec.sampleRate);
            if (regionEndSample <= sampleStart)
                continue;

            auto startSongSample = juce::jmax (regionStartSample, sampleStart);
            auto endSongSample = juce::jmin (regionEndSample, sampleEnd);

            // Calculate offset between song and audio source samples, clip at region borders in audio source samples
            // (if a plug-in supports time stretching, it will also need to reflect the stretch factor here).
            const auto offsetToPlaybackRegion = playbackRegion->getStartInAudioModificationSamples() - regionStartSample;

            const auto startAvailableSourceSamples = juce::jmax (0LL, playbackRegion->getStartInAudioModificationSamples());
            const auto endAvailableSourceSamples = juce::jmin (audioSource->getSampleCount(), playbackRegion->getEndInAudioModificationSamples());

            startSongSample = juce::jmax (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
            endSongSample = juce::jmin (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);
            const int numSamplesToRead = (int) (endSongSample - startSongSample);
            if (numSamplesToRead <= 0)
                continue;

            // If we're using a buffering reader, set the appropriate timeout.
            if (useBufferedAudioSourceReader)
            {
                jassert (dynamic_cast<juce::BufferingAudioReader*> (reader.get()) != nullptr);
                auto bufferingReader = static_cast<juce::BufferingAudioReader*> (reader.get());
                bufferingReader->setReadTimeout (context.isNonRealtime ? 100 : 0);
            }

            // Calculate buffer offsets and handle reverse playback setting.
            const auto audioModification = playbackRegion->getAudioModification<ARAPluginDemoAudioModification>();
            const bool playReversed = audioModification->getReversePlayback();
            const int startInBuffer = (int) (startSongSample - sampleStart);
            auto startInSource = startSongSample + offsetToPlaybackRegion;
            if (playReversed)
                startInSource = audioSource->getSampleCount() - startInSource - numSamplesToRead;

            // Read samples:
            // first region can write directly into output, later regions need to use local buffer.
            auto& readBuffer = (didRenderFirstRegion) ? *tempBuffer : buffer;
            if (!reader->read (&readBuffer, startInBuffer, numSamplesToRead, startInSource, true, true))
            {
                success = false;
                continue;
            }

            if (playReversed)
                readBuffer.reverse (startInBuffer, numSamplesToRead);

            if (didRenderFirstRegion)
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
                const int remainingSamples = buffer.getNumSamples() - endInBuffer;
                if (remainingSamples != 0)
                    buffer.clear (endInBuffer, remainingSamples);

                didRenderFirstRegion = true;
            }
        }
    }

    // If no playback or no region did intersect, clear buffer now.
    if (! didRenderFirstRegion)
        buffer.clear();

    return success;
}
