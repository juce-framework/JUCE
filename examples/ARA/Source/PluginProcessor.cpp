/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ARA/AraTestDocumentController.h"
#include "ARA/AraTestPlaybackRenderer.h"

//==============================================================================
Juce_fakeAraanalysisAudioProcessor::Juce_fakeAraanalysisAudioProcessor()
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

Juce_fakeAraanalysisAudioProcessor::~Juce_fakeAraanalysisAudioProcessor()
{
}

//==============================================================================
const String Juce_fakeAraanalysisAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Juce_fakeAraanalysisAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Juce_fakeAraanalysisAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Juce_fakeAraanalysisAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Juce_fakeAraanalysisAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Juce_fakeAraanalysisAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Juce_fakeAraanalysisAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Juce_fakeAraanalysisAudioProcessor::setCurrentProgram (int index)
{
}

const String Juce_fakeAraanalysisAudioProcessor::getProgramName (int index)
{
    return {};
}

void Juce_fakeAraanalysisAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Juce_fakeAraanalysisAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void Juce_fakeAraanalysisAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Juce_fakeAraanalysisAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void Juce_fakeAraanalysisAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
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

	ARA::PlugIn::AraTestPlaybackRenderer * playbackRenderer = (ARA::PlugIn::AraTestPlaybackRenderer*) getARAPlugInExtension()->getPlaybackRenderer();
	if (playbackRenderer)
	{
		playbackRenderer->renderPlaybackRegions(buffer.getArrayOfWritePointers(), getTotalNumInputChannels(), getSampleRate(), currentPosition.timeInSamples, buffer.getNumSamples(), currentPosition.isPlaying);
	}
}

//==============================================================================
bool Juce_fakeAraanalysisAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Juce_fakeAraanalysisAudioProcessor::createEditor()
{
    return new Juce_fakeAraanalysisAudioProcessorEditor (*this);
}

//==============================================================================
void Juce_fakeAraanalysisAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Juce_fakeAraanalysisAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Juce_fakeAraanalysisAudioProcessor();
}

//==============================================================================
const ARA::PlugIn::PlugInExtension* Juce_fakeAraanalysisAudioProcessor::createARAPlugInExtension(ARA::PlugIn::DocumentController* documentController, bool isPlaybackRenderer, bool isEditorRenderer, bool isEditorView)
{
	// Construct a plugin extension instance with our own playback renderer type
	return new ARA::PlugIn::PlugInExtension (documentController,
											 isPlaybackRenderer ? new ARA::PlugIn::AraTestPlaybackRenderer (documentController) : nullptr,
											 isEditorRenderer ? new ARA::PlugIn::EditorRenderer (documentController) : nullptr,
											 isEditorView ? new ARA::PlugIn::EditorView (documentController) : nullptr);
}