/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "ARATestPluginProcessor.h"
#include "ARATestPluginEditor.h"
#include "ARA/ARATestDocumentController.h"
#include "ARA/ARATestPlaybackRenderer.h"

//==============================================================================
ARATestPluginProcessor::ARATestPluginProcessor()
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

ARATestPluginProcessor::~ARATestPluginProcessor()
{
}

//==============================================================================
const String ARATestPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ARATestPluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ARATestPluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ARATestPluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ARATestPluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ARATestPluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ARATestPluginProcessor::getCurrentProgram()
{
    return 0;
}

void ARATestPluginProcessor::setCurrentProgram (int index)
{
}

const String ARATestPluginProcessor::getProgramName (int index)
{
    return {};
}

void ARATestPluginProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ARATestPluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ARATestPluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ARATestPluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ARATestPluginProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
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

    // Attempt to delegate processing to our playback renderer implementation
    if (getARAPlugInExtension() == nullptr)
        return;

    AudioPlayHead::CurrentPositionInfo currentPosition;
    if (!getPlayHead()->getCurrentPosition(currentPosition))
        return;

    ARA::PlugIn::ARATestPlaybackRenderer * playbackRenderer = (ARA::PlugIn::ARATestPlaybackRenderer*) getARAPlugInExtension()->getPlaybackRenderer();
    if (playbackRenderer)
    {
        playbackRenderer->renderPlaybackRegions(buffer.getArrayOfWritePointers(), getTotalNumInputChannels(), getSampleRate(), currentPosition.timeInSamples, buffer.getNumSamples(), currentPosition.isPlaying);
    }
}

//==============================================================================
bool ARATestPluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ARATestPluginProcessor::createEditor()
{
    return new ARATestPluginEditor (*this);
}

//==============================================================================
void ARATestPluginProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ARATestPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ARATestPluginProcessor();
}

//==============================================================================
const ARA::PlugIn::PlugInExtension* ARATestPluginProcessor::createARAPlugInExtension(ARA::PlugIn::DocumentController* documentController, ARA::ARAPlugInInstanceRoleFlags knownRoles, ARA::ARAPlugInInstanceRoleFlags assignedRoles)
{
    // Construct a plugin extension instance with our own playback renderer type
    return ARA::PlugIn::PlugInExtension::createWithRoles<ARA::PlugIn::ARATestPlaybackRenderer> (documentController, knownRoles, assignedRoles);
}