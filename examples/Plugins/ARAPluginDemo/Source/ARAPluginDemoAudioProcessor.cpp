#include "ARAPluginDemoAudioProcessor.h"
#include "ARAPluginDemoAudioProcessorEditor.h"
#include "ARAPluginDemoPlaybackRenderer.h"

//==============================================================================
ARAPluginDemoAudioProcessor::ARAPluginDemoAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
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
    if (auto playbackRenderer = getPlaybackRenderer())
        for (const auto& playbackRegion : playbackRenderer->getPlaybackRegions())
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
void ARAPluginDemoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // ARARenderer draft note: this could be moved to a centralized AudioProcessorARAExtension::prepareToPlay() method.
    const juce::ARARenderer::ProcessSpec processSpec { sampleRate, samplesPerBlock, getMainBusNumOutputChannels() };
    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->prepareToPlay (processSpec);
    // since we're using the default implementation which does no editor rendering,
    // this will be a no-op and would be omitted in actual plug-ins.
    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->prepareToPlay (processSpec);
}

void ARAPluginDemoAudioProcessor::releaseResources()
{
    // ARARenderer draft note: this could be moved to a centralized AudioProcessorARAExtension::releaseResources() method.
    if (auto playbackRenderer = getPlaybackRenderer())
        playbackRenderer->releaseResources();
    // since we're using the default implementation which does no editor rendering,
    // this will be a no-op and would be omitted in actual plug-ins.
    if (auto editorRenderer = getEditorRenderer())
        editorRenderer->releaseResources();
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

    // update playback position and state
    if (auto playhead = getPlayHead())
    {
        if (!playhead->getCurrentPosition (lastPositionInfo))
            lastPositionInfo.resetToDefault();
    }

    if (isBoundToARA())
    {
        // ARARenderer draft note: this could be moved to a centralized AudioProcessorARAExtension::processBlockARA() method.
        const juce::ARARenderer::ProcessContext processContext { isNonRealtime(), lastPositionInfo };
        
        // render our ARA playback regions for this buffer
        if (auto playbackRenderer = getPlaybackRenderer())
            playbackRenderer->processBlock (buffer, processContext);

        // render our ARA editor regions and sequences for this buffer
        // since we're using the default implementation which does no editor rendering,
        // this will be a no-op and would be omitted in actual plug-ins.
        if (auto editorRenderer = getEditorRenderer())
            editorRenderer->processBlock (buffer, processContext);
    }
    else
    {
        // this sample plug-in requires to be used with ARA - we just pass through otherwise.
        // in an actual plug-in, proper non-ARA rendering would be invoked here
        processBlockBypassed (buffer, midiMessages);
    }
}

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
