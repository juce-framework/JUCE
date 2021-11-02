#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

//==============================================================================
/**
    Processor class for ARAPluginDemo
    In this simple demo we're using a buffered ARA sample reader to pull audio samples
    from the host and per default render them without any modifications, effectively
    making this an ARA enabled pass-through renderer.
    The only edit supported by the plug-in is reversing the audio as an example how to
    use ARA audio modification state, e.g. how it can be shared across multiple
    ARA playback regions if desired.
*/
class ARAPluginDemoAudioProcessor     : public juce::AudioProcessor,
                                        public juce::AudioProcessorARAExtension
{
public:
    //==============================================================================
    ARAPluginDemoAudioProcessor();

    // Getter of current playback state for the UI
    const juce::AudioPlayHead::CurrentPositionInfo& getLastKnownPositionInfo() { return lastPositionInfo; }

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    juce::AudioPlayHead::CurrentPositionInfo lastPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginDemoAudioProcessor)
};
