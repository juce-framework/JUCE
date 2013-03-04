/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

FILTERHEADERS


//==============================================================================
FILTERCLASSNAME::FILTERCLASSNAME()
{
}

FILTERCLASSNAME::~FILTERCLASSNAME()
{
}

//==============================================================================
const String FILTERCLASSNAME::getName() const
{
    return JucePlugin_Name;
}

int FILTERCLASSNAME::getNumParameters()
{
    return 0;
}

float FILTERCLASSNAME::getParameter (int index)
{
    return 0.0f;
}

void FILTERCLASSNAME::setParameter (int index, float newValue)
{
}

const String FILTERCLASSNAME::getParameterName (int index)
{
    return String::empty;
}

const String FILTERCLASSNAME::getParameterText (int index)
{
    return String::empty;
}

const String FILTERCLASSNAME::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String FILTERCLASSNAME::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool FILTERCLASSNAME::isInputChannelStereoPair (int index) const
{
    return true;
}

bool FILTERCLASSNAME::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool FILTERCLASSNAME::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool FILTERCLASSNAME::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool FILTERCLASSNAME::silenceInProducesSilenceOut() const
{
    return false;
}

double FILTERCLASSNAME::getTailLengthSeconds() const
{
    return 0.0;
}

int FILTERCLASSNAME::getNumPrograms()
{
    return 0;
}

int FILTERCLASSNAME::getCurrentProgram()
{
    return 0;
}

void FILTERCLASSNAME::setCurrentProgram (int index)
{
}

const String FILTERCLASSNAME::getProgramName (int index)
{
    return String::empty;
}

void FILTERCLASSNAME::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void FILTERCLASSNAME::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void FILTERCLASSNAME::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void FILTERCLASSNAME::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getSampleData (channel);

        // ..do something to the data...
    }

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool FILTERCLASSNAME::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* FILTERCLASSNAME::createEditor()
{
    return new EDITORCLASSNAME (this);
}

//==============================================================================
void FILTERCLASSNAME::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FILTERCLASSNAME::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FILTERCLASSNAME();
}
