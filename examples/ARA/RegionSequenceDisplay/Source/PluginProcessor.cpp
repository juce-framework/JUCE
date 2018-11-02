/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginARAPlaybackRenderer.h"

//==============================================================================
ArasampleProjectAudioProcessor::ArasampleProjectAudioProcessor()
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

ArasampleProjectAudioProcessor::~ArasampleProjectAudioProcessor()
{
}

//==============================================================================
const String ArasampleProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ArasampleProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ArasampleProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ArasampleProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ArasampleProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ArasampleProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ArasampleProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ArasampleProjectAudioProcessor::setCurrentProgram (int /*index*/)
{
}

const String ArasampleProjectAudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void ArasampleProjectAudioProcessor::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

//==============================================================================
void ArasampleProjectAudioProcessor::prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ArasampleProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ArasampleProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ArasampleProjectAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& /*midiMessages*/)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    AudioPlayHead::CurrentPositionInfo ci{ 0 };
    if (getPlayHead() == nullptr || !(getPlayHead()->getCurrentPosition(ci)))
        return;

    ARASampleProjectPlaybackRenderer* playbackRenderer = static_cast<ARASampleProjectPlaybackRenderer*>(getARAPlaybackRenderer());
    playbackRenderer->renderPlaybackRegions(buffer, getSampleRate(), ci.timeInSamples, ci.isPlaying);
}

//==============================================================================
bool ArasampleProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ArasampleProjectAudioProcessor::createEditor()
{
    return new ArasampleProjectAudioProcessorEditor (*this);
}

//==============================================================================
void ArasampleProjectAudioProcessor::getStateInformation (MemoryBlock& /*destData*/)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ArasampleProjectAudioProcessor::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ArasampleProjectAudioProcessor();
}
