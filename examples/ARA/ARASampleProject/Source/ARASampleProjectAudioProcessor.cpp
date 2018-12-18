#include "ARASampleProjectAudioProcessor.h"
#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARASampleProjectPlaybackRenderer.h"

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

ARASampleProjectAudioProcessor::~ARASampleProjectAudioProcessor()
{
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
    double tail = 0.0;
    if (auto playbackRenderer = getARAPlaybackRenderer())
    {
        for (auto playbackRegion : playbackRenderer->getPlaybackRegions<ARAPlaybackRegion>())
            tail = jmax (tail, playbackRegion->getTailTime());
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
        getARAPlaybackRenderer()->prepareToPlay (newSampleRate, getTotalNumOutputChannels(), samplesPerBlock, true);
}

void ARASampleProjectAudioProcessor::releaseResources()
{
    if (isARAPlaybackRenderer())
        getARAPlaybackRenderer()->releaseResources();
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
        // render our ARA playback regions for this buffer
        if (isARAPlaybackRenderer())
            getARAPlaybackRenderer()->processBlock (buffer, timeInSamples, isPlaying, isNonRealtime());

        // render our ARA editing preview
        if (isARAEditorRenderer())
            getARAEditorRenderer()->processBlock (buffer, timeInSamples, isPlaying, isNonRealtime());
    }
    else
    {
        // this sample plug-in requires to be used with ARA.
        // otherwise, proper non-ARA rendering would be invoked here
        buffer.clear();
    }
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
