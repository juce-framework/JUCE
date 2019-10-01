#include "ARASampleProjectAudioProcessor.h"
#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARASampleProjectDocumentController.h"

//==============================================================================
ARASampleProjectAudioProcessor::ARASampleProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    lastPositionInfo.resetToDefault();
}

//==============================================================================
const String ARASampleProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ARASampleProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ARASampleProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ARASampleProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ARASampleProjectAudioProcessor::getTailLengthSeconds() const
{
    double tail{};
    if (auto playbackRenderer = getARAPlaybackRenderer())
    {
        auto documentController = playbackRenderer->getDocumentController();
        for (auto playbackRegion : playbackRenderer->getPlaybackRegions<ARAPlaybackRegion>())
        {
            ARA::ARATimeDuration regionHeadTime{}, regionTailTime{};
            documentController->getPlaybackRegionHeadAndTailTime (ARA::PlugIn::toRef (playbackRegion), &regionHeadTime, &regionTailTime);
            tail = jmax (tail, regionTailTime);
        }
    }

    return tail;
}

int ARASampleProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ARASampleProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ARASampleProjectAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const String ARASampleProjectAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void ARASampleProjectAudioProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

//==============================================================================
void ARASampleProjectAudioProcessor::prepareToPlay (double newSampleRate, int samplesPerBlock)
{
    if (isARAPlaybackRenderer())
    {
        audioSourceReaders.clear();

        const auto documentController = getARAPlaybackRenderer()->getDocumentController<ARASampleProjectDocumentController>();

        for (auto playbackRegion : getARAPlaybackRenderer()->getPlaybackRegions())
        {
            auto audioSource = playbackRegion->getAudioModification()->getAudioSource<ARAAudioSource>();
            if (audioSourceReaders.count (audioSource) == 0)
            {
                AudioFormatReader* sourceReader = new ARAAudioSourceReader (audioSource);

                if (! isAlwaysNonRealtime())
                {
                    // if we're being used in real-time, wrap our source reader in buffering
                    // reader to avoid blocking while reading samples in processBlock
                    const int readAheadSizeBySampleRate = roundToInt (2.0 * newSampleRate);
                    const int readAheadSizeByBlockSize = 8 * samplesPerBlock;
                    const int readAheadSize = jmax (readAheadSizeBySampleRate, readAheadSizeByBlockSize);

                    sourceReader = new BufferingAudioReader (sourceReader, documentController->getAudioSourceReadingThread(), readAheadSize);
                }

                audioSourceReaders.emplace (audioSource, sourceReader);
            }
        }

        if (getARAPlaybackRenderer()->getPlaybackRegions().size() > 1)
            tempBuffer.reset (new AudioBuffer<float> (getTotalNumOutputChannels(), getBlockSize()));
        else
            tempBuffer.reset();
    }
}

void ARASampleProjectAudioProcessor::releaseResources()
{
    if (isARAPlaybackRenderer())
    {
        audioSourceReaders.clear();
        tempBuffer = nullptr;
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ARASampleProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void ARASampleProjectAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& /*midiMessages*/)
{
    ScopedNoDenormals noDenormals;

    bool success = true;

    // get playback position and state
    int64 timeInSamples = 0;
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
            jassert (isNonRealtime() || ! isAlwaysNonRealtime());

            bool didRenderFirstRegion = false;

            if (isPlaying)
            {
                int64 sampleStart = timeInSamples;
                int64 sampleEnd = timeInSamples + buffer.getNumSamples();
                for (auto playbackRegion : getARAPlaybackRenderer()->getPlaybackRegions())
                {
                    // get the audio source for this region and make sure we have an audio source reader for it
                    auto audioSource = playbackRegion->getAudioModification()->getAudioSource<ARAAudioSource>();
                    auto readerIt = audioSourceReaders.find(audioSource);
                    if (readerIt == audioSourceReaders.end())
                    {
                        success = false;
                        continue;
                    }
                    auto& reader = readerIt->second;

                    // render silence if access is currently disabled
                    // (the audio reader deals with this internally too, checking it here is merely an optimization)
                    if (! audioSource->isSampleAccessEnabled())
                    {
                        success = false;
                        continue;
                    }

                    // this simplified test code "rendering" only produces audio if sample rate and channel count match
                    if ((audioSource->getChannelCount() != getTotalNumOutputChannels()) || (audioSource->getSampleRate() != getSampleRate()))
                        continue;

                    // evaluate region borders in song time, calculate sample range to copy in song time
                    int64 regionStartSample = playbackRegion->getStartInPlaybackSamples (getSampleRate());
                    if (sampleEnd <= regionStartSample)
                        continue;

                    int64 regionEndSample = playbackRegion->getEndInPlaybackSamples (getSampleRate());
                    if (regionEndSample <= sampleStart)
                        continue;

                    int64 startSongSample = jmax (regionStartSample, sampleStart);
                    int64 endSongSample = jmin (regionEndSample, sampleEnd);

                    // calculate offset between song and audio source samples, clip at region borders in audio source samples
                    // (if a plug-in supports time stretching, it will also need to reflect the stretch factor here)
                    int64 offsetToPlaybackRegion = playbackRegion->getStartInAudioModificationSamples() - regionStartSample;

                    int64 startAvailableSourceSamples = jmax (0LL, playbackRegion->getStartInAudioModificationSamples());
                    int64 endAvailableSourceSamples = jmin (audioSource->getSampleCount(), playbackRegion->getEndInAudioModificationSamples());

                    startSongSample = jmax (startSongSample, startAvailableSourceSamples - offsetToPlaybackRegion);
                    endSongSample = jmin (endSongSample, endAvailableSourceSamples - offsetToPlaybackRegion);
                    if (endSongSample <= startSongSample)
                        continue;

                    // calculate buffer offsets
                    int startInDestBuffer = (int) (startSongSample - sampleStart);
                    int64 startInSource = startSongSample + offsetToPlaybackRegion;
                    int numSamplesToRead = (int) (endSongSample - startSongSample);

                    // if we're using a buffering reader then set the appropriate timeout
                    if (! isAlwaysNonRealtime())
                    {
                        jassert (dynamic_cast<BufferingAudioReader*> (reader.get()) != nullptr);
                        auto bufferingReader = static_cast<BufferingAudioReader*> (reader.get());
                        bufferingReader->setReadTimeout (isNonRealtime() ? 100 : 0);
                    }

                    // read samples
                    bool bufferSuccess;
                    if (didRenderFirstRegion)
                    {
                        // target buffer is initialized, so read region samples into local buffer
                        bufferSuccess = reader->read (tempBuffer.get(), 0, numSamplesToRead, startInSource, true, true);

                        // if successful, mix local buffer into the output buffer
                        if (bufferSuccess)
                        {
                            for (int c = 0; c < getTotalNumOutputChannels(); ++c)
                                buffer.addFrom(c, startInDestBuffer, *tempBuffer, c, 0, numSamplesToRead);
                        }
                    }
                    else
                    {
                        // this is the first region to hit the buffer, so initialize its samples
                        bufferSuccess = reader->read (&buffer, startInDestBuffer, numSamplesToRead, startInSource, true, true);

                        // if successful, clear any excess at start or end of the region and mark successful buffer initialization
                        if (bufferSuccess)
                        {
                            if (startInDestBuffer != 0)
                                buffer.clear(0, startInDestBuffer);

                            int samplesWritten = startInDestBuffer + numSamplesToRead;
                            int remainingSamples = buffer.getNumSamples() - samplesWritten;
                            if (remainingSamples != 0)
                                buffer.clear(samplesWritten, remainingSamples);

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
        // this sample plug-in requires to be used with ARA.
        // otherwise, proper non-ARA rendering would be invoked here
        buffer.clear();
    }

    lastProcessBlockSucceeded = success;
}

//==============================================================================
bool ARASampleProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ARASampleProjectAudioProcessor::createEditor()
{
    return new ARASampleProjectAudioProcessorEditor (*this);
}

//==============================================================================
// when using ARA, all model state is stored in the ARA archives,
// and the state here in the plug-in instance is limited to view configuration data
// or other editor settings, of which this sample plug-in has none.

void ARASampleProjectAudioProcessor::getStateInformation (MemoryBlock& /*destData*/)
{
}

void ARASampleProjectAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ARASampleProjectAudioProcessor();
}
