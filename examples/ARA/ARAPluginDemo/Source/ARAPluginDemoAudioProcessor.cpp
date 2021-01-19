#include "ARAPluginDemoAudioProcessor.h"
#include "ARAPluginDemoAudioProcessorEditor.h"
#include "ARAPluginDemoAudioModification.h"

//==============================================================================
ARAPluginDemoAudioProcessor::ARAPluginDemoAudioProcessor()
     : ARAPluginDemoAudioProcessor (true)
{
}

ARAPluginDemoAudioProcessor::ARAPluginDemoAudioProcessor (bool useBuffering)
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
     :
#endif
       useBufferedAudioSourceReader (useBuffering)
{
    lastPositionInfo.resetToDefault();
}

//==============================================================================
const juce::String ARAPluginDemoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ARAPluginDemoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ARAPluginDemoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ARAPluginDemoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ARAPluginDemoAudioProcessor::getTailLengthSeconds() const
{
    double tail{};
    if (auto playbackRenderer = getARAPlaybackRenderer())
        for (auto playbackRegion : playbackRenderer->getPlaybackRegions<juce::ARAPlaybackRegion>())
            tail = juce::jmax (tail, playbackRegion->getTailTime());

    return tail;
}

int ARAPluginDemoAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ARAPluginDemoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ARAPluginDemoAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const juce::String ARAPluginDemoAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void ARAPluginDemoAudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================
void ARAPluginDemoAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    if (isARAPlaybackRenderer())
    {
        audioSourceReaders.clear();

        for (auto playbackRegion : getARAPlaybackRenderer()->getPlaybackRegions())
        {
            auto audioSource = playbackRegion->getAudioModification()->getAudioSource<juce::ARAAudioSource>();
            if (audioSourceReaders.count (audioSource) == 0)
            {
                juce::AudioFormatReader* sourceReader = new juce::ARAAudioSourceReader (audioSource);

                if (useBufferedAudioSourceReader)
                {
                    // if we're being used in real-time, wrap our source reader in a buffering
                    // reader to avoid blocking while reading samples in processBlock
                    const int readAheadSize = juce::roundToInt (2.0 * sampleRate);
                    sourceReader = new juce::BufferingAudioReader (sourceReader, *sharedTimesliceThread, readAheadSize);
                }

                audioSourceReaders.emplace (audioSource, sourceReader);
            }
        }

        if (getARAPlaybackRenderer()->getPlaybackRegions().size() > 1)
            tempBuffer.reset (new juce::AudioBuffer<float> (getTotalNumOutputChannels(), getBlockSize()));
        else
            tempBuffer.reset();
    }
}

void ARAPluginDemoAudioProcessor::releaseResources()
{
    if (isARAPlaybackRenderer())
    {
        audioSourceReaders.clear();
        tempBuffer = nullptr;
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ARAPluginDemoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ARAPluginDemoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    bool success = true;

    // get playback position and state
    auto timeInSamples = 0LL;
    bool isPlaying = false;
    if (getPlayHead() != nullptr)
    {
        if (getPlayHead()->getCurrentPosition (lastPositionInfo))
        {
            timeInSamples = lastPositionInfo.timeInSamples;
            isPlaying = lastPositionInfo.isPlaying;
        }
    }

    if (isBoundToARA())
    {
        // render our ARA playback regions for this buffer, realtime or offline
        if (isARAPlaybackRenderer())
        {
            jassert (buffer.getNumSamples() <= getBlockSize());
            jassert (isNonRealtime() || useBufferedAudioSourceReader);

            bool didRenderFirstRegion = false;

            if (isPlaying)
            {
                const auto sampleStart = timeInSamples;
                const auto sampleEnd = timeInSamples + buffer.getNumSamples();
                for (auto playbackRegion : getARAPlaybackRenderer()->getPlaybackRegions())
                {
                    // get the audio source for this region and make sure we have an audio source reader for it
                    const auto audioSource = playbackRegion->getAudioModification()->getAudioSource<juce::ARAAudioSource>();
                    const auto readerIt = audioSourceReaders.find (audioSource);
                    if (readerIt == audioSourceReaders.end())
                    {
                        success = false;
                        continue;
                    }
                    const auto& reader = readerIt->second;

                    // this simplified test code "rendering" only produces audio if sample rate and channel count match
                    if ((audioSource->getChannelCount() != getTotalNumOutputChannels()) || (audioSource->getSampleRate() != getSampleRate()))
                        continue;

                    // evaluate region borders in song time, calculate sample range to copy in song time
                    const auto regionStartSample = playbackRegion->getStartInPlaybackSamples (getSampleRate());
                    if (sampleEnd <= regionStartSample)
                        continue;

                    const auto regionEndSample = playbackRegion->getEndInPlaybackSamples (getSampleRate());
                    if (regionEndSample <= sampleStart)
                        continue;

                    auto startSongSample = juce::jmax (regionStartSample, sampleStart);
                    auto endSongSample = juce::jmin (regionEndSample, sampleEnd);

                    // calculate offset between song and audio source samples, clip at region borders in audio source samples
                    // (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
                    const auto offsetToPlaybackRegion = playbackRegion->getStartInAudioModificationSamples() - regionStartSample;

                    const auto startAvailableSourceSamples = juce::jmax (0LL, playbackRegion->getStartInAudioModificationSamples());
                    const auto endAvailableSourceSamples = juce::jmin (audioSource->getSampleCount(), playbackRegion->getEndInAudioModificationSamples());

                    startSongSample = juce::jmax (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
                    endSongSample = juce::jmin (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);
                    if (endSongSample <= startSongSample)
                        continue;

                    // calculate buffer offsets
                    const int startInDestBuffer = (int) (startSongSample - sampleStart);
                    const auto startInSource = startSongSample + offsetToPlaybackRegion;
                    const int numSamplesToRead = (int) (endSongSample - startSongSample);

                    // if we're using a buffering reader then set the appropriate timeout
                    if (useBufferedAudioSourceReader)
                    {
                        jassert (dynamic_cast<juce::BufferingAudioReader*> (reader.get()) != nullptr);
                        auto bufferingReader = static_cast<juce::BufferingAudioReader*> (reader.get());
                        bufferingReader->setReadTimeout (isNonRealtime() ? 100 : 0);
                    }

                    // read samples
                    const auto audioModification = playbackRegion->getAudioModification<ARAPluginDemoAudioModification>();
                    bool bufferSuccess;
                    if (didRenderFirstRegion)
                    {
                        if (audioModification->getReversePlayback())
                        {
                            const auto reversedStartInSource = audioSource->getSampleCount() - startInSource - numSamplesToRead;
                            bufferSuccess = reader->read (tempBuffer.get(), 0, numSamplesToRead, reversedStartInSource, true, true);
                            tempBuffer->reverse (0, numSamplesToRead);
                        }
                        else
                        {
                            // target buffer is initialized, so read region samples into local buffer
                            bufferSuccess = reader->read (tempBuffer.get(), 0, numSamplesToRead, startInSource, true, true);
                        }
                        
                        // if successful, mix local buffer into the output buffer
                        if (bufferSuccess)
                            for (int c = 0; c < getTotalNumOutputChannels(); ++c)
                                buffer.addFrom (c, startInDestBuffer, *tempBuffer, c, 0, numSamplesToRead);
                    }
                    else
                    {
                        if (audioModification->getReversePlayback())
                        {
                            const auto reversedStartInSource = audioSource->getSampleCount() - startInSource - numSamplesToRead;
                            bufferSuccess = reader->read (&buffer, 0, numSamplesToRead, reversedStartInSource, true, true);
                            buffer.reverse (0, numSamplesToRead);
                        }
                        else
                        {
                            // this is the first region to hit the buffer, so initialize its samples
                            bufferSuccess = reader->read (&buffer, startInDestBuffer, numSamplesToRead, startInSource, true, true);
                        }

                        // if successful, clear any excess at start or end of the region and mark successful buffer initialization
                        if (bufferSuccess)
                        {
                            if (startInDestBuffer != 0)
                                buffer.clear (0, startInDestBuffer);

                            int samplesWritten = startInDestBuffer + numSamplesToRead;
                            int remainingSamples = buffer.getNumSamples() - samplesWritten;
                            if (remainingSamples != 0)
                                buffer.clear (samplesWritten, remainingSamples);

                            didRenderFirstRegion = true;
                        }
                    }
                    success &= bufferSuccess;
                }
            }

            // if no playback or no region did intersect, clear buffer now
            if (! didRenderFirstRegion)
                buffer.clear();
        }

        // render our ARA editing preview only if in real time
        if (isARAEditorRenderer() && ! isNonRealtime())
        {
            // no rendering to do here since this sample plug-in does not provide editor rendering
            // otherwise, we'd add our signal to the buffer here.
        }
    }
    else
    {
        // this sample plug-in requires to be used with ARA - we just pass through otherwise.
        // in an actual plug-in, proper non-ARA rendering would be invoked here
        processBlockBypassed (buffer, midiMessages);
    }

    lastProcessBlockSucceeded = success;
}

#if JucePlugin_Enable_ARA
bool ARAPluginDemoAudioProcessor::didProcessBlockSucceed()
{
    // You can use this function to inform the calling code that the
    // most recent processBlock call didn't output samples as expected.
    return lastProcessBlockSucceeded;
}
#endif

//==============================================================================
bool ARAPluginDemoAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ARAPluginDemoAudioProcessor::createEditor()
{
    return new ARAPluginDemoAudioProcessorEditor (*this);
}

//==============================================================================
// when using ARA, all model state is stored in the ARA archives,
// and the state here in the plug-in instance is limited to view configuration data
// or other editor settings, of which this sample plug-in has none.

void ARAPluginDemoAudioProcessor::getStateInformation (juce::MemoryBlock& /*destData*/)
{
}

void ARAPluginDemoAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ARAPluginDemoAudioProcessor();
}
