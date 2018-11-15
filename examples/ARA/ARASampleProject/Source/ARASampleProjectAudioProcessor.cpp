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
    return 0.0;
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
void ARASampleProjectAudioProcessor::prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ARASampleProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // clear unused input channeds
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // make sure we can get the play head
    AudioPlayHead::CurrentPositionInfo ci{ 0 };
    if (getPlayHead() == nullptr || !(getPlayHead()->getCurrentPosition (ci)))
        return;

    // render our ARA playback regions for this time duration using the ARA playback renderer instance
    ARASampleProjectPlaybackRenderer* playbackRenderer = static_cast<ARASampleProjectPlaybackRenderer*> (getARAPlaybackRenderer());
    playbackRenderer->renderSamples (buffer, getSampleRate(), ci.timeInSamples, ci.isPlaying);
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
void ARASampleProjectAudioProcessor::getStateInformation (MemoryBlock& /*destData*/)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ARASampleProjectAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ARASampleProjectAudioProcessor();
}
