/*
  ==============================================================================

    NoiseGate.cpp
    Created: 23 Nov 2015 3:08:33pm
    Author:  Fabian Renn

  ==============================================================================
*/

#include "NoiseGate.h"
#include "NoiseGateEditor.h"


//==============================================================================
NoiseGate::NoiseGate()
{
    addParameter (threshold = new AudioParameterFloat ("threshold", "Threshold", 0.0f, 1.0f, 0.5f));
    addParameter (alpha  = new AudioParameterFloat ("alpha",  "Alpha",   0.0f, 1.0f, 0.8f));

    busArrangement.inputBuses.clear();
  //  busArrangement.outputBuses.clear();

   // busArrangement.inputBuses. add (AudioProcessorBus ("Sidechain",  AudioChannelSet::stereo()));
}

NoiseGate::~NoiseGate()
{
}

//==============================================================================
const String NoiseGate::getName() const
{
    return JucePlugin_Name;
}

bool NoiseGate::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoiseGate::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoiseGate::silenceInProducesSilenceOut() const
{
    return false;
}

double NoiseGate::getTailLengthSeconds() const
{
    return 0.0;
}

int NoiseGate::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NoiseGate::getCurrentProgram()
{
    return 0;
}

void NoiseGate::setCurrentProgram (int /*index*/)
{
}

const String NoiseGate::getProgramName (int /*index*/)
{
    return String();
}

void NoiseGate::changeProgramName (int /*index*/, const String& /*newName*/)
{
}

//==============================================================================
bool NoiseGate::setPreferredBusArrangement (bool isInputBus, int busIndex, const AudioChannelSet& preferred)
{
    //const bool isMainBus = (busIndex ==  0);

    // always force input and output bus to match their formats
    //if (isMainBus && (! AudioProcessor::setPreferredBusArrangement (! isInputBus, 0, preferred)))
      //  return false;

    return AudioProcessor::setPreferredBusArrangement (isInputBus, busIndex, preferred);
}

//==============================================================================
void NoiseGate::prepareToPlay (double /*sampleRate*/, int /*samplesPerBlock*/)
{
    lowPassCoeff = 0.0f;
    sampleCountDown = 0;
}

void NoiseGate::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void NoiseGate::processBlock (AudioSampleBuffer& buffer, MidiBuffer&)
{
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // I've added this to avoid people getting screaming feedback
    // when they first compile the plugin, but obviously you don't need to
    // this code if your algorithm already fills all the output channels.
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

   // AudioSampleBuffer mainInputOutput = busArrangement.getBusBuffer (buffer, true, 0);
     AudioSampleBuffer mainInputOutput = busArrangement.getBusBuffer (buffer, false, 0);
   // AudioSampleBuffer sideChainInput  = busArrangement.getBusBuffer (buffer, true, 1);

    float alphaCopy = *alpha;
    float thresholdCopy = *threshold;

    for (int j = 0; j < buffer.getNumSamples(); ++j)
    {
        float mixedSamples = 0.0f;
      //  for (int i = 0; i < sideChainInput.getNumChannels(); ++i)
      //      mixedSamples += sideChainInput.getReadPointer (i) [j];

      //  mixedSamples /= static_cast<float> (sideChainInput.getNumChannels());
        lowPassCoeff = (alphaCopy * lowPassCoeff) + ((1.0f - alphaCopy) * mixedSamples);

        if (lowPassCoeff >= thresholdCopy)
            sampleCountDown = (int) getSampleRate();

        // very in-effective way of doing this
        for (int i = 0; i < mainInputOutput.getNumChannels(); ++i)
            *mainInputOutput.getWritePointer (i, j) = sampleCountDown > 0 ? *mainInputOutput.getReadPointer (i, j) : 0.0f;

        if (sampleCountDown > 0)
            --sampleCountDown;
    }
}

//==============================================================================
bool NoiseGate::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* NoiseGate::createEditor()
{
    return new NoiseGateEditor (*this);
}

//==============================================================================
void NoiseGate::getStateInformation (MemoryBlock&)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NoiseGate::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoiseGate();
}
