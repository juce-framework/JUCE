#pragma once

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <map>

#if ! JucePlugin_Enable_ARA
    #error "bad project configuration, JucePlugin_Enable_ARA is required for compiling this class"
#endif

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
    // Default construction as done by the plug-in client wrappers via createPluginFilterOfType()
    // Since these may be used in realtime contexts, they must use internal buffering when reading
    // audio source samples, thus this c'tor sets useBufferedAudioSourceReader to true.
    ARAPluginDemoAudioProcessor();

    // Explicit construction when used internally, see PlaybackRegionView.cpp for an example
    // In typical UI use cases, these internal processors are used inside an ARAPlaybackRegionReader
    // on a separate background thread which already implements buffering - to prevent unnecessary
    // double buffering, set useBufferedAudioSourceReader to false in such cases.
    explicit ARAPluginDemoAudioProcessor (bool useBufferedAudioSourceReader);

    // Getter of current playback state for the UI
    const juce::AudioPlayHead::CurrentPositionInfo& getLastKnownPositionInfo() { return lastPositionInfo; }

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool didProcessBlockSucceed() override;

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

    // We're subclassing here only to provide a proper default c'tor for our shared ressource
    class SharedTimeSliceThread   : public juce::TimeSliceThread
    {
        public:
            SharedTimeSliceThread()
                : TimeSliceThread (juce::String (JucePlugin_Name) + " ARA Sample Reading Thread")
            {
                startThread (7);   // above "default" priority so playback is fluent, but below realtime
            }
    };
    juce::SharedResourcePointer<SharedTimeSliceThread> sharedTimesliceThread;

    // map of audio sources to buffering audio source readers
    // we'll use them to pull ARA samples from the host as we render
    std::map<juce::ARAAudioSource*, std::unique_ptr<juce::AudioFormatReader>> audioSourceReaders;

    // temp buffers to use for summing signals if rendering multiple regions
    std::unique_ptr<juce::AudioBuffer<float>> tempBuffer;

    const bool useBufferedAudioSourceReader;

    bool lastProcessBlockSucceeded { true };

    juce::AudioPlayHead::CurrentPositionInfo lastPositionInfo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ARAPluginDemoAudioProcessor)
};
